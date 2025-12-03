# Sistema de Monitoramento de Via Férrea

Sistema completo de monitoramento ferroviário com ESP32-CAM, sensor PIR, múltiplos modos de operação e análise de IA no servidor.

## Arquitetura

```
Sensor PIR (GPIO 13) → ESP32-CAM → MQTT → Servidor CCO → YOLO/IA → Resultados MQTT
```

## Componentes

### Firmware ESP32-CAM

- **Sensor PIR HC-SR501**: Detecta movimento na via férrea (GPIO 13)
- **Câmera**: Ativada sob demanda para economia de energia
- **MQTT**: Comunicação com servidor CCO
- **Múltiplos Modos**: Hibernação, Auto, Manual, Vigilância

### Servidor CCO (Python)

- **Processamento de IA**: YOLO para detectar humanos e animais
- **Filtro de Falso Positivo**: Distingue trens de humanos/animais baseado em tempo
- **Interface Web**: Dashboard para monitoramento
- **MQTT**: Recebe frames e publica resultados

## Modos de Operação

### 1. HIBERNATION (Hibernação)
- Câmera desligada (economia máxima de energia)
- Apenas monitora sensor PIR
- Notifica CCO quando detecta movimento (mas não captura imagem)

### 2. AUTO (Automático)
- Câmera desligada inicialmente
- Quando PIR detecta movimento:
  - Liga câmera
  - Aguarda estabilização (500ms)
  - Captura 1 frame
  - Envia via MQTT com flag `motion_triggered: true`
  - Desliga câmera
  - Aguarda cooldown (2s) antes de nova detecção

### 3. MANUAL
- Câmera desligada
- Aguarda comando MQTT `{"action": "capture"}`
- Quando recebe comando:
  - Liga câmera
  - Captura frame
  - Envia via MQTT
  - Desliga câmera

### 4. VIGILANCE (Vigilância)
- Câmera ativa continuamente
- Captura frames em intervalos (5 segundos)
- Envia todos os frames via MQTT

## Controle Remoto via MQTT

### Mudar Modo
```json
{
  "action": "set_mode",
  "mode": "hibernation|auto|manual|vigilance"
}
```

### Captura Manual
```json
{
  "action": "capture"
}
```

### Reiniciar
```json
{
  "action": "restart"
}
```

## Tópicos MQTT

- `esp32cam/frames` - Frames da câmera (publicação)
- `esp32cam/motion` - Notificação de movimento (publicação)
- `esp32cam/mode` - Modo atual (publicação)
- `esp32cam/commands` - Comandos remotos (subscrição)
- `esp32cam/detections` - Resultados de detecção do servidor (publicação)
- `esp32cam/status` - Status do sistema (publicação)

## Instalação

### Firmware

1. Abra `firmware/firmware.ino` no Arduino IDE
2. Configure credenciais WiFi e MQTT em `config.h`
3. Faça upload para ESP32-CAM

### Servidor CCO

1. Instale dependências:
```bash
pip install -r requirements.txt
```

2. Configure credenciais MQTT em `cco_server.py`

3. Execute o servidor:
```bash
python cco_server.py
```

4. Acesse interface web: http://localhost:5000

## Filtro de Falso Positivo

O servidor CCO implementa filtro baseado em tempo:

- **Trens**: Passam rápido (< 3 segundos) → Marcado como falso positivo
- **Humanos/Animais**: Ficam mais tempo (≥ 3 segundos) → Detecção válida

Configurável em `cco_server.py`:
```python
MIN_DETECTION_TIME_SECONDS = 3.0  # Tempo mínimo para considerar válido
```

## Processamento de IA

### YOLO Local (Padrão)
- Usa modelo YOLOv8n (baixado automaticamente)
- Processa frames localmente
- Detecta: person, dog, cat, horse, cow, sheep, bird

### API Externa (Opcional)
- Pode ser configurado para usar Gemini Vision API ou Hugging Face
- Modificar função `process_frame_with_yolo()` em `cco_server.py`

## Economia de Energia

- Câmera desligada via PWDN quando não em uso
- Modo hibernação mantém apenas PIR e comunicação básica
- WiFi e MQTT sempre ativos (necessário para comunicação)

## Estrutura de Mensagens

### Frame para Análise
```json
{
  "device_id": "esp32cam_device_001",
  "frame_id": 123,
  "timestamp": 1234567890,
  "mode": "auto",
  "motion_triggered": true,
  "format": "jpeg",
  "width": 640,
  "height": 480,
  "size": 15000,
  "quality": 12,
  "data": "base64_encoded_jpeg..."
}
```

### Resultado de Detecção
```json
{
  "device_id": "esp32cam_device_001",
  "frame_id": 123,
  "timestamp": 1234567890,
  "detections": [
    {
      "class": "person",
      "confidence": 0.85,
      "bbox": [100, 100, 200, 300]
    }
  ],
  "is_false_positive": false,
  "alert_level": "high"
}
```

## Configurações

### Firmware (`config.h`)
- `PIR_GPIO_PIN = 13` - Pino do sensor PIR
- `VIGILANCE_INTERVAL_MS = 5000` - Intervalo no modo vigília
- `MOTION_DEBOUNCE_MS = 200` - Debounce do PIR
- `CAMERA_STABILIZATION_MS = 500` - Tempo de estabilização
- `AUTO_MODE_COOLDOWN_MS = 2000` - Cooldown entre detecções

### Servidor (`cco_server.py`)
- `MIN_DETECTION_TIME_SECONDS = 3.0` - Tempo mínimo para detecção válida
- `USE_YOLO_LOCAL = True` - Usar YOLO local ou API externa
- `INTEREST_CLASSES` - Classes de interesse para detecção

## Troubleshooting

### Câmera não inicializa
- Verifique conexões dos pinos
- Verifique se PSRAM está habilitado
- Tente reiniciar o ESP32

### MQTT não conecta
- Verifique credenciais do HiveMQ Cloud
- Verifique conexão WiFi
- Verifique firewall/porta 8883

### Detecções incorretas
- Ajuste `MIN_DETECTION_TIME_SECONDS`
- Verifique qualidade da imagem
- Ajuste threshold de confiança do YOLO

## Licença

Este projeto é parte do sistema M4-IOT.




