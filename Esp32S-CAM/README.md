## ESP32S-CAM

Este diretório foi simplificado para uso rápido da ESP32-CAM:

- `Esp32S-CAM.ino`: sketch Arduino a ser carregado na placa.
- `camera_viewer.py`: visualizador do stream da câmera, com opção de detecção de pessoas (YOLOv6 ONNX).
- `Testes/`: materiais e rascunhos de testes do projeto.


### 1) Como gravar o firmware

Opção A — Arduino IDE:
- Abra o `esp32s-cam.ino` na Arduino IDE.
- Placa: “AI Thinker ESP32-CAM” ou equivalente (`esp32:esp32:esp32cam`).
- Partições/flash padrão (flash 4MB, DIO, 80 MHz). Ative PSRAM se disponível.
- Faça o upload com o jumper IO0-GND (modo boot), depois remova e reinicie.

Opção B — CLI (PowerShell, Windows):
- Pré-requisitos: `arduino-cli`, `esptool.py`.
- Use o script `flash_esp32cam.ps1`:
  ```powershell
  .\flash_esp32cam.ps1 -Port COM3
  ```

Após o upload, abra o Serial Monitor a 115200 baud para ver o IP obtido via DHCP (ou configure IP fixo no sketch).

### 2) Visualizar o vídeo

Instale dependências do visualizador (Python 3.8+):
```bash
pip install opencv-python numpy onnxruntime
```

Rode o visualizador apontando para o endpoint `/stream` da sua ESP32-CAM:
```bash
python camera_viewer.py --url http://<IP_DA_ESP32-CAM>/stream
```

Detecção de pessoas (opcional, precisa do modelo YOLOv6 em ONNX):
```bash
python camera_viewer.py --url http://<IP>/stream --model yolov6s.onnx
```
Parâmetros úteis:
- `--retry`: tentativas de conexão (padrão: 5)
- `--conf-thres`: confiança mínima (padrão: 0.35)
- `--iou-thres`: IoU do NMS (padrão: 0.45)
- `--det-interval`: executar detecção a cada N frames (padrão: 3)

### 3) Sobre o sketch `esp32s-cam.ino`

Este arquivo é o ponto único do firmware. Coloque aqui seu código para:
- Configurar Wi‑Fi (SSID/Senha).
- Inicializar a câmera (pinos do módulo e PSRAM).
- Iniciar o servidor web/stream em `/stream` (compatível com `camera_viewer.py`).

Se já possui um sketch funcional em outro local (ex.: `ProjetoM4.ino`), migre o conteúdo para `esp32s-cam.ino`.

### 4) Testes

Use a pasta `Testes/` para guardar scripts, sketches experimentais e evidências. Mantida para histórico e iterações rápidas.


