#!/usr/bin/env python3
"""
Servidor CCO (Centro de Controle Operacional)
Sistema de monitoramento ferroviário com análise de IA

Funcionalidades:
- Recebe frames via MQTT da ESP32-CAM
- Processa imagens com YOLO/IA para detectar humanos e animais
- Filtra falsos positivos (trens) baseado em tempo de detecção
- Publica resultados de detecção via MQTT
- Interface web para visualização
"""

import json
import base64
import time
import threading
from datetime import datetime
from collections import defaultdict
from typing import Dict, List, Optional

import paho.mqtt.client as mqtt
import cv2
import numpy as np
from flask import Flask, render_template_string, jsonify

# =================== Configurações ===================

# MQTT Configuration
MQTT_BROKER = "7bf2160d4f4849f0885901cae207a260.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_USER = "esp32cam_001"
MQTT_PASS = "Esp32cam_001"

# Tópicos MQTT
MQTT_TOPIC_FRAMES = "esp32cam/frames"
MQTT_TOPIC_MOTION = "esp32cam/motion"
MQTT_TOPIC_DETECTIONS = "esp32cam/detections"
MQTT_TOPIC_MODE = "esp32cam/mode"

# Configurações de IA
USE_YOLO_LOCAL = True  # Se False, usa API externa (Gemini/Hugging Face)
YOLO_MODEL_PATH = "yolov8n.pt"  # Modelo YOLO (será baixado automaticamente se não existir)

# Filtro de falso positivo
MIN_DETECTION_TIME_SECONDS = 3.0  # Tempo mínimo para considerar detecção válida (humanos/animais ficam mais tempo)
DETECTION_HISTORY_WINDOW = 5.0  # Janela de tempo para rastrear detecções (segundos)

# Classes de interesse (humanos e animais)
INTEREST_CLASSES = ['person', 'dog', 'cat', 'horse', 'cow', 'sheep', 'bird']

# =================== Estado Global ===================

detection_history: Dict[str, List[Dict]] = defaultdict(list)  # device_id -> lista de detecções
current_detections: Dict[str, Dict] = {}  # device_id -> última detecção
frame_buffer: Dict[str, np.ndarray] = {}  # device_id -> último frame
lock = threading.Lock()

# =================== Inicialização YOLO ===================

yolo_model = None

def init_yolo():
    """Inicializa modelo YOLO"""
    global yolo_model
    
    if not USE_YOLO_LOCAL:
        print("[YOLO] Modo API externa - YOLO local desabilitado")
        return None
    
    try:
        from ultralytics import YOLO
        print("[YOLO] Carregando modelo YOLO...")
        yolo_model = YOLO(YOLO_MODEL_PATH)
        print("[YOLO] Modelo carregado com sucesso")
        return yolo_model
    except ImportError:
        print("[ERRO] ultralytics não instalado. Execute: pip install ultralytics")
        print("[YOLO] Usando modo placeholder (sem detecção real)")
        return None
    except Exception as e:
        print(f"[ERRO] Falha ao carregar YOLO: {e}")
        print("[YOLO] Usando modo placeholder (sem detecção real)")
        return None

# =================== Processamento de IA ===================

def process_frame_with_yolo(frame: np.ndarray) -> List[Dict]:
    """
    Processa frame com YOLO e retorna detecções
    
    Returns:
        Lista de detecções: [{"class": str, "confidence": float, "bbox": [x, y, w, h]}]
    """
    if yolo_model is None:
        # Modo placeholder - retorna detecção fake para teste
        return [{"class": "person", "confidence": 0.85, "bbox": [100, 100, 200, 300]}]
    
    try:
        results = yolo_model(frame, verbose=False)
        detections = []
        
        for result in results:
            boxes = result.boxes
            for box in boxes:
                cls = int(box.cls[0])
                conf = float(box.conf[0])
                class_name = yolo_model.names[cls]
                
                # Filtrar apenas classes de interesse
                if class_name in INTEREST_CLASSES and conf > 0.5:
                    x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                    detections.append({
                        "class": class_name,
                        "confidence": conf,
                        "bbox": [int(x1), int(y1), int(x2 - x1), int(y2 - y1)]
                    })
        
        return detections
    except Exception as e:
        print(f"[ERRO] Falha ao processar frame com YOLO: {e}")
        return []

def is_false_positive(device_id: str, detections: List[Dict]) -> bool:
    """
    Verifica se as detecções são falsos positivos (provavelmente trem)
    Baseado em tempo de detecção: trens passam rápido, humanos/animais ficam mais tempo
    """
    if not detections:
        return False
    
    current_time = time.time()
    
    with lock:
        # Adicionar detecções ao histórico
        detection_history[device_id].append({
            "timestamp": current_time,
            "detections": detections
        })
        
        # Limpar histórico antigo
        detection_history[device_id] = [
            d for d in detection_history[device_id]
            if current_time - d["timestamp"] < DETECTION_HISTORY_WINDOW
        ]
        
        # Verificar se há detecções consistentes por tempo suficiente
        if len(detection_history[device_id]) < 2:
            # Muito recente, pode ser trem passando rápido
            return True
        
        # Calcular tempo desde primeira detecção
        first_detection_time = detection_history[device_id][0]["timestamp"]
        detection_duration = current_time - first_detection_time
        
        if detection_duration < MIN_DETECTION_TIME_SECONDS:
            # Detecção muito rápida = provável trem
            print(f"[Filtro] Detecção muito rápida ({detection_duration:.2f}s) - provável trem")
            return True
        
        # Detecção por tempo suficiente = provável humano/animal
        print(f"[Filtro] Detecção válida ({detection_duration:.2f}s) - provável humano/animal")
        return False

# =================== MQTT Callbacks ===================

def on_connect(client, userdata, flags, rc):
    """Callback quando conecta ao MQTT"""
    if rc == 0:
        print("[MQTT] Conectado ao broker")
        
        # Subscrever aos tópicos
        client.subscribe(MQTT_TOPIC_FRAMES)
        client.subscribe(MQTT_TOPIC_MOTION)
        client.subscribe(MQTT_TOPIC_MODE)
        print(f"[MQTT] Inscrito em: {MQTT_TOPIC_FRAMES}, {MQTT_TOPIC_MOTION}, {MQTT_TOPIC_MODE}")
    else:
        print(f"[MQTT] Falha na conexão. Código: {rc}")

def on_message(client, userdata, msg):
    """Callback quando recebe mensagem MQTT"""
    topic = msg.topic
    
    try:
        if topic == MQTT_TOPIC_FRAMES:
            handle_frame_message(msg)
        elif topic == MQTT_TOPIC_MOTION:
            handle_motion_message(msg)
        elif topic == MQTT_TOPIC_MODE:
            handle_mode_message(msg)
    except Exception as e:
        print(f"[ERRO] Erro ao processar mensagem: {e}")

def handle_frame_message(msg):
    """Processa frame recebido via MQTT"""
    try:
        data = json.loads(msg.payload.decode('utf-8'))
        device_id = data.get('device_id', 'unknown')
        frame_id = data.get('frame_id', 0)
        timestamp = data.get('timestamp', 0)
        mode = data.get('mode', 'unknown')
        motion_triggered = data.get('motion_triggered', False)
        
        # Decodificar frame base64
        frame_data = base64.b64decode(data['data'])
        nparr = np.frombuffer(frame_data, np.uint8)
        frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        
        if frame is None:
            print(f"[ERRO] Falha ao decodificar frame do dispositivo {device_id}")
            return
        
        print(f"[Frame] Recebido do {device_id}: Frame #{frame_id}, Mode={mode}, Motion={motion_triggered}")
        
        # Armazenar frame
        with lock:
            frame_buffer[device_id] = frame
        
        # Processar com YOLO
        detections = process_frame_with_yolo(frame)
        
        if detections:
            print(f"[Detecção] {len(detections)} objeto(s) detectado(s): {[d['class'] for d in detections]}")
            
            # Verificar se é falso positivo
            is_fp = is_false_positive(device_id, detections)
            
            # Preparar resultado
            result = {
                "device_id": device_id,
                "frame_id": frame_id,
                "timestamp": timestamp,
                "detections": detections,
                "is_false_positive": is_fp,
                "alert_level": "high" if not is_fp else "low"
            }
            
            # Publicar resultado
            client.publish(MQTT_TOPIC_DETECTIONS, json.dumps(result))
            print(f"[Resultado] Publicado: {len(detections)} detecções, FP={is_fp}")
            
            # Armazenar última detecção
            with lock:
                current_detections[device_id] = result
        else:
            print("[Detecção] Nenhum objeto de interesse detectado")
            
    except Exception as e:
        print(f"[ERRO] Erro ao processar frame: {e}")

def handle_motion_message(msg):
    """Processa notificação de movimento"""
    try:
        data = json.loads(msg.payload.decode('utf-8'))
        device_id = data.get('device_id', 'unknown')
        timestamp = data.get('timestamp', 0)
        
        print(f"[Movimento] Detectado no dispositivo {device_id} em {datetime.fromtimestamp(timestamp/1000)}")
        
    except Exception as e:
        print(f"[ERRO] Erro ao processar notificação de movimento: {e}")

def handle_mode_message(msg):
    """Processa mudança de modo"""
    try:
        data = json.loads(msg.payload.decode('utf-8'))
        device_id = data.get('device_id', 'unknown')
        mode = data.get('mode', 'unknown')
        
        print(f"[Modo] Dispositivo {device_id} mudou para modo: {mode}")
        
    except Exception as e:
        print(f"[ERRO] Erro ao processar mudança de modo: {e}")

# =================== Interface Web ===================

app = Flask(__name__)

HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CCO - Monitoramento Ferroviário</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background: #1a1a1a;
            color: #fff;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        h1 {
            color: #4CAF50;
        }
        .device-card {
            background: #2a2a2a;
            border-radius: 8px;
            padding: 20px;
            margin: 20px 0;
        }
        .detection {
            background: #3a3a3a;
            padding: 10px;
            margin: 10px 0;
            border-radius: 4px;
            border-left: 4px solid #4CAF50;
        }
        .false-positive {
            border-left-color: #ff9800;
        }
        .alert-high {
            border-left-color: #f44336;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Centro de Controle Operacional (CCO)</h1>
        <p>Sistema de Monitoramento Ferroviário com IA</p>
        
        <div id="devices">
            <p>Carregando dispositivos...</p>
        </div>
    </div>
    
    <script>
        async function updateDevices() {
            const res = await fetch('/api/devices');
            const data = await res.json();
            
            const devicesDiv = document.getElementById('devices');
            if (data.devices.length === 0) {
                devicesDiv.innerHTML = '<p>Nenhum dispositivo conectado</p>';
                return;
            }
            
            devicesDiv.innerHTML = data.devices.map(device => `
                <div class="device-card">
                    <h2>Dispositivo: ${device.device_id}</h2>
                    <p>Última atualização: ${new Date(device.last_update * 1000).toLocaleString()}</p>
                    ${device.last_detection ? `
                        <div class="detection ${device.last_detection.is_false_positive ? 'false-positive' : 'alert-high'}">
                            <h3>Última Detecção</h3>
                            <p>Objetos: ${device.last_detection.detections.map(d => d.class).join(', ')}</p>
                            <p>Falso Positivo: ${device.last_detection.is_false_positive ? 'Sim (provável trem)' : 'Não'}</p>
                            <p>Nível de Alerta: ${device.last_detection.alert_level}</p>
                        </div>
                    ` : '<p>Nenhuma detecção ainda</p>'}
                </div>
            `).join('');
        }
        
        setInterval(updateDevices, 5000);
        updateDevices();
    </script>
</body>
</html>
"""

@app.route('/')
def index():
    return render_template_string(HTML_TEMPLATE)

@app.route('/api/devices')
def api_devices():
    """API para obter status dos dispositivos"""
    with lock:
        devices = []
        for device_id, detection in current_detections.items():
            devices.append({
                "device_id": device_id,
                "last_detection": detection,
                "last_update": time.time()
            })
    
    return jsonify({"devices": devices})

# =================== Main ===================

def main():
    """Função principal"""
    global client
    
    print("=" * 50)
    print("Servidor CCO - Monitoramento Ferroviário")
    print("=" * 50)
    
    # Inicializar YOLO
    init_yolo()
    
    # Configurar MQTT
    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)
    client.tls_set()  # TLS para HiveMQ Cloud
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Conectar ao broker
    print(f"[MQTT] Conectando ao broker {MQTT_BROKER}...")
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
    except Exception as e:
        print(f"[ERRO] Falha ao conectar ao broker: {e}")
        return
    
    # Iniciar loop MQTT em thread separada
    client.loop_start()
    
    # Iniciar servidor web
    print("[Web] Iniciando servidor web na porta 5000...")
    app.run(host='0.0.0.0', port=5000, debug=False)

if __name__ == "__main__":
    main()




