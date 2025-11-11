"""Visualizador para o stream da ESP32-CAM com opção de detecção de pessoas via YOLOv6.

Uso básico:
    python camera_viewer.py --url http://<IP_DA_ESP32-CAM>/stream

Para habilitar a detecção de pessoas (necessário baixar o modelo YOLOv6 ONNX):
    python camera_viewer.py --url http://<IP>/stream --model yolov6s.onnx

Dependências:
    pip install opencv-python numpy onnxruntime
"""

from __future__ import annotations

import argparse
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Tuple

import cv2
import numpy as np

try:
    import onnxruntime as ort
except ImportError:  # pragma: no cover - import optional
    ort = None  # type: ignore


@dataclass
class Detection:
    bbox: Tuple[int, int, int, int]
    score: float
    label: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Visualiza o stream MJPEG da ESP32-CAM em uma janela pop-up e, opcionalmente, executa detecção de pessoas com YOLOv6"
    )
    parser.add_argument(
        "--url",
        required=True,
        help="URL completa do stream, por exemplo: http://192.168.0.123/stream",
    )
    parser.add_argument(
        "--retry",
        type=int,
        default=5,
        help="Número de tentativas de conexão antes de abortar (padrão: 5)",
    )
    parser.add_argument(
        "--model",
        type=Path,
        help="Caminho para o arquivo YOLOv6 ONNX (ex.: yolov6s.onnx). Se omitido, não realiza detecção.",
    )
    parser.add_argument(
        "--conf-thres",
        type=float,
        default=0.35,
        help="Confiança mínima para exibir deteções (padrão: 0.35)",
    )
    parser.add_argument(
        "--iou-thres",
        type=float,
        default=0.45,
        help="IoU mínima para o NMS (padrão: 0.45)",
    )
    parser.add_argument(
        "--det-interval",
        type=int,
        default=3,
        help="Executa detecção a cada N frames (padrão: 3)",
    )
    parser.add_argument(
        "--max-delay-ms",
        type=int,
        default=200,
        help="Descarta frames atrasados além desse limite (padrão: 200 ms)",
    )
    return parser.parse_args()


def letterbox(image: np.ndarray, new_shape: Tuple[int, int] = (640, 640), color: Tuple[int, int, int] = (114, 114, 114)) -> Tuple[np.ndarray, float, Tuple[float, float]]:
    shape = image.shape[:2]  # (alto, largo)
    if not shape[0] or not shape[1]:
        raise ValueError("Frame vazio recebido da câmera")

    r = min(new_shape[0] / shape[0], new_shape[1] / shape[1])
    new_unpad = (int(round(shape[1] * r)), int(round(shape[0] * r)))
    dw = (new_shape[1] - new_unpad[0]) / 2
    dh = (new_shape[0] - new_unpad[1]) / 2

    resized = cv2.resize(image, new_unpad, interpolation=cv2.INTER_LINEAR)
    top, bottom = int(round(dh - 0.1)), int(round(dh + 0.1))
    left, right = int(round(dw - 0.1)), int(round(dw + 0.1))
    new_img = cv2.copyMakeBorder(resized, top, bottom, left, right, cv2.BORDER_CONSTANT, value=color)
    return new_img, r, (dw, dh)


def nms(detections: List[Detection], iou_thres: float) -> List[Detection]:
    if not detections:
        return []

    boxes = np.array([det.bbox for det in detections])
    scores = np.array([det.score for det in detections])
    idxs = scores.argsort()[::-1]

    selected = []
    while idxs.size > 0:
        current = idxs[0]
        selected.append(detections[current])
        if idxs.size == 1:
            break
        rest = idxs[1:]

        xx1 = np.maximum(boxes[current, 0], boxes[rest, 0])
        yy1 = np.maximum(boxes[current, 1], boxes[rest, 1])
        xx2 = np.minimum(boxes[current, 2], boxes[rest, 2])
        yy2 = np.minimum(boxes[current, 3], boxes[rest, 3])

        w = np.maximum(0, xx2 - xx1)
        h = np.maximum(0, yy2 - yy1)
        inter = w * h
        area_current = (boxes[current, 2] - boxes[current, 0]) * (boxes[current, 3] - boxes[current, 1])
        area_rest = (boxes[rest, 2] - boxes[rest, 0]) * (boxes[rest, 3] - boxes[rest, 1])
        iou = inter / (area_current + area_rest - inter + 1e-6)

        idxs = rest[iou <= iou_thres]

    return selected


class YOLOv6Detector:
    def __init__(self, model_path: Path, conf_thres: float, iou_thres: float) -> None:
        if ort is None:
            raise RuntimeError(
                "onnxruntime não está instalado. Execute 'pip install onnxruntime' para usar a detecção."
            )
        if not model_path.is_file():
            raise FileNotFoundError(f"Modelo não encontrado: {model_path}")

        self.session = ort.InferenceSession(str(model_path), providers=["CPUExecutionProvider"])
        self.conf_thres = conf_thres
        self.iou_thres = iou_thres
        self.input_shape = self.session.get_inputs()[0].shape[2:]
        if isinstance(self.input_shape[0], str) or isinstance(self.input_shape[1], str):
            # Modelos dinâmicos ainda assim costumam ser quadrados
            self.input_shape = (640, 640)

        print(f"YOLOv6 carregado ({model_path.name}) | entrada: {self.input_shape}")

    def preprocess(self, frame: np.ndarray) -> Tuple[np.ndarray, float, Tuple[float, float]]:
        img, ratio, dwdh = letterbox(frame, self.input_shape)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        img = img.astype(np.float32) / 255.0
        img = np.transpose(img, (2, 0, 1))  # CHW
        img = np.expand_dims(img, 0)  # NCHW
        return img, ratio, dwdh

    def postprocess(
        self, outputs: List[np.ndarray], ratio: float, dwdh: Tuple[float, float]
    ) -> List[Detection]:
        # Saída típica: [1, 8400, 85]
        predictions = outputs[0]
        if predictions.ndim == 3:
            predictions = predictions[0]

        detections: List[Detection] = []
        dw, dh = dwdh

        for pred in predictions:
            obj_conf = pred[4]
            class_scores = pred[5:]
            class_id = int(np.argmax(class_scores))
            if class_id != 0:  # classe 0 = pessoa
                continue
            score = obj_conf * class_scores[class_id]
            if score < self.conf_thres:
                continue

            cx, cy, w, h = pred[:4]
            x1 = (cx - w / 2 - dw) / ratio
            y1 = (cy - h / 2 - dh) / ratio
            x2 = (cx + w / 2 - dw) / ratio
            y2 = (cy + h / 2 - dh) / ratio

            box = (
                max(int(x1), 0),
                max(int(y1), 0),
                max(int(x2), 0),
                max(int(y2), 0),
            )
            detections.append(Detection(bbox=box, score=float(score), label="person"))

        return nms(detections, self.iou_thres)

    def __call__(self, frame: np.ndarray) -> List[Detection]:
        blob, ratio, dwdh = self.preprocess(frame)
        outputs = self.session.run(None, {self.session.get_inputs()[0].name: blob})
        return self.postprocess(outputs, ratio, dwdh)


def draw_detections(frame: np.ndarray, detections: List[Detection]) -> np.ndarray:
    for det in detections:
        x1, y1, x2, y2 = det.bbox
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        label = f"{det.label}: {det.score:.2f}"
        (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
        cv2.rectangle(frame, (x1, y1 - th - 6), (x1 + tw + 4, y1), (0, 255, 0), cv2.FILLED)
        cv2.putText(frame, label, (x1 + 2, y1 - 4), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)

    cv2.putText(
        frame,
        f"Pessoas: {len(detections)}",
        (10, 30),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.8,
        (0, 255, 255),
        2,
    )
    return frame


def main() -> int:
    args = parse_args()
    retries = args.retry

    detector: Optional[YOLOv6Detector] = None
    if args.model:
        try:
            detector = YOLOv6Detector(args.model, args.conf_thres, args.iou_thres)
        except Exception as exc:  # pragma: no cover - feedback ao usuário
            print(f"Falha ao carregar modelo YOLOv6: {exc}")
            return 1

    cap = None
    for attempt in range(1, retries + 1):
        cap = cv2.VideoCapture(args.url)
        if cap.isOpened():
            # tentar reduzir buffer interno
            cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            cap.set(cv2.CAP_PROP_FPS, 0)  # nem todos os backends respeitam
            break
        print(f"Tentativa {attempt}/{retries} falhou. Repetindo em 2 segundos...")
        cap.release()
        cap = None
        time.sleep(2)

    if cap is None or not cap.isOpened():
        print("Não foi possível abrir o stream. Verifique o IP, a URL e se a ESP32-CAM está ligada.")
        return 1

    window_name = "ESP32-CAM"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    print("Pressione ESC ou feche a janela para encerrar.")

    frame_count = 0
    last_detections: List[Detection] = []
    last_det_time = 0.0
    max_delay_sec = args.max_delay_ms / 1000.0

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Frame inválido recebido. Encerrando...")
            break

        # descartamos frames muito atrasados quando possível
        timestamp = cap.get(cv2.CAP_PROP_POS_MSEC)
        if timestamp > 0 and time.time() - last_det_time > max_delay_sec:
            # se estamos atrasados, lê e descarta até aproximar
            while True:
                ret2, frame2 = cap.read()
                if not ret2:
                    break
                timestamp2 = cap.get(cv2.CAP_PROP_POS_MSEC)
                if timestamp2 <= 0 or time.time() - last_det_time <= max_delay_sec:
                    frame = frame2
                    break

        frame_count += 1

        if detector is not None and (frame_count % max(args.det_interval, 1) == 0):
            detections = detector(frame)
            last_detections = detections
            last_det_time = time.time()

        if last_detections:
            frame_to_show = draw_detections(frame.copy(), last_detections)
        else:
            frame_to_show = frame

        cv2.imshow(window_name, frame_to_show)

        key = cv2.waitKey(1) & 0xFF
        if key == 27:  # ESC
            break

    cap.release()
    cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    sys.exit(main())

