#!/usr/bin/env python3
"""
Cliente MQTT com interface web Flask para visualizar frames da ESP32-CAM.

Uso:
    python mqtt_web_viewer.py

Acesse: http://localhost:5000
"""

import json
import base64
import threading
import time
from io import BytesIO
from flask import Flask, render_template_string, Response, jsonify, request
from flask_cors import CORS

try:
    import paho.mqtt.client as mqtt
    import cv2
    import numpy as np
except ImportError:
    print("Erro: Instale as dependências:")
    print("  pip install paho-mqtt opencv-python numpy flask flask-cors")
    exit(1)

# =================== Configurações HiveMQ Cloud ===================
MQTT_BROKER = "7bf2160d4f4849f0885901cae207a260.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_USER = "esp32cam_001"
MQTT_PASS = "Esp32cam_001"
MQTT_CLIENT_ID = f"esp32cam_viewer_{int(time.time())}"  # ID único para evitar conflitos

MQTT_TOPIC_FRAMES = "esp32cam/frames"
MQTT_TOPIC_STATUS = "esp32cam/status"
MQTT_TOPIC_COMMANDS = "esp32cam/commands"

# =================== Estado Global ===================
app = Flask(__name__)
CORS(app)

current_frame_jpeg = None
frame_count = 0
status_info = {}
mqtt_client = None
lock = threading.Lock()

# =================== Template HTML ===================
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM Remote Viewer</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #fff;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .video-container {
            background: rgba(255,255,255,0.1);
            border-radius: 20px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        .video-box {
            background: #000;
            border-radius: 15px;
            overflow: hidden;
            position: relative;
            min-height: 400px;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .video-box img {
            max-width: 100%;
            height: auto;
        }
        .status {
            color: #ccc;
            text-align: center;
            padding: 10px;
        }
        .controls {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
            margin-top: 20px;
        }
        button {
            padding: 12px 24px;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            background: rgba(255,255,255,0.2);
            color: #fff;
            backdrop-filter: blur(10px);
        }
        button:hover {
            background: rgba(255,255,255,0.3);
            transform: translateY(-2px);
        }
        .info-panel {
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 20px;
            margin-top: 20px;
            backdrop-filter: blur(10px);
        }
        .info-item {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        .info-item:last-child {
            border-bottom: none;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📹 ESP32-CAM Remote Viewer</h1>
            <p>Visualização remota via MQTT + HiveMQ Cloud</p>
        </div>
        
        <div class="video-container">
            <div class="video-box">
                <img id="stream" src="/video_feed" alt="Live Stream" onerror="console.error('Erro ao carregar stream')" onload="console.log('Frame carregado')" />
            </div>
            <div class="status" id="status">Aguardando frames...</div>
        </div>
        
        <div class="controls">
            <button onclick="sendCommand('toggle_yolo')">Toggle YOLO</button>
            <button onclick="sendCommand('toggle_mqtt')">Toggle MQTT</button>
            <button onclick="sendCommand('restart')">Reiniciar ESP32</button>
        </div>
        
        <div class="info-panel">
            <h3>Informações do Sistema</h3>
            <div class="info-item">
                <span>Frames Recebidos:</span>
                <span id="frameCount">0</span>
            </div>
            <div class="info-item">
                <span>Status ESP32:</span>
                <span id="espStatus">Desconhecido</span>
            </div>
            <div class="info-item">
                <span>IP ESP32:</span>
                <span id="espIP">--</span>
            </div>
            <div class="info-item">
                <span>Uptime:</span>
                <span id="uptime">--</span>
            </div>
        </div>
    </div>
    
    <script>
        let frameCount = 0;
        
        // Atualizar contador de frames
        setInterval(() => {
            fetch('/api/stats')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('frameCount').textContent = data.frames;
                    document.getElementById('espStatus').textContent = data.status || 'Desconhecido';
                    document.getElementById('espIP').textContent = data.ip || '--';
                    document.getElementById('uptime').textContent = data.uptime ? data.uptime + 's' : '--';
                });
        }, 2000);
        
        function sendCommand(action) {
            fetch('/api/command', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({action: action})
            })
            .then(r => r.json())
            .then(data => {
                alert('Comando enviado: ' + action);
            });
        }
    </script>
</body>
</html>
"""


def on_connect(client, userdata, flags, rc):
    """Callback quando conecta ao broker."""
    if rc == 0:
        print(f"[MQTT] ✓ Conectado ao broker: {MQTT_BROKER}")
        print(f"[MQTT] Flags: {flags}")
        result1 = client.subscribe(MQTT_TOPIC_FRAMES, qos=0)
        result2 = client.subscribe(MQTT_TOPIC_STATUS, qos=0)
        print(f"[MQTT] Inscrito em '{MQTT_TOPIC_FRAMES}': {result1}")
        print(f"[MQTT] Inscrito em '{MQTT_TOPIC_STATUS}': {result2}")
        if result1[0] == 0 and result2[0] == 0:
            print(f"[MQTT] ✓ Inscrições bem-sucedidas. Aguardando mensagens...")
        else:
            print(f"[MQTT] ⚠ Erro nas inscrições!")
    else:
        print(f"[MQTT] ✗ Falha na conexão. Código: {rc}")
        print(f"[MQTT] Códigos de erro: 0=OK, 1=Protocol, 2=Client, 3=Network, 4=Auth, 5=Unknown")


def on_disconnect(client, userdata, rc):
    """Callback quando desconecta do broker."""
    if rc != 0:
        print(f"[MQTT] Desconexão inesperada. Código: {rc}")
    else:
        print(f"[MQTT] Desconectado normalmente")


def on_subscribe(client, userdata, mid, granted_qos):
    """Callback quando se inscreve em um tópico."""
    print(f"[MQTT] Inscrição confirmada. MID: {mid}, QoS: {granted_qos}")


def on_message(client, userdata, msg):
    """Callback quando recebe mensagem."""
    global current_frame_jpeg, frame_count, status_info
    
    topic = msg.topic
    payload_size = len(msg.payload)
    
    print(f"[MQTT] ✓✓✓ Mensagem recebida no tópico '{topic}' ({payload_size} bytes) ✓✓✓")
    
    if topic == MQTT_TOPIC_FRAMES:
        try:
            print(f"[DEBUG] Parseando JSON...")
            data = json.loads(msg.payload.decode('utf-8'))
            print(f"[DEBUG] JSON parseado. Keys: {list(data.keys())}")
            
            if 'data' not in data:
                print("[ERRO] Campo 'data' não encontrado no JSON!")
                return
            
            print(f"[DEBUG] Decodificando base64 (tamanho: {len(data['data'])} chars)...")
            frame_data = base64.b64decode(data['data'])
            print(f"[DEBUG] Base64 decodificado: {len(frame_data)} bytes")
            
            # Converter para JPEG
            nparr = np.frombuffer(frame_data, np.uint8)
            print(f"[DEBUG] Decodificando JPEG com OpenCV...")
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
            if frame is not None:
                print(f"[DEBUG] Frame decodificado: {frame.shape}")
                # Codificar como JPEG para streaming
                success, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 85])
                
                if success:
                    jpeg_bytes = buffer.tobytes()
                    with lock:
                        current_frame_jpeg = jpeg_bytes
                        frame_count += 1
                        new_count = frame_count
                    
                    print(f"[✓ Frame {new_count}] Recebido e armazenado: {data.get('width')}x{data.get('height')} ({len(frame_data)} bytes JPEG -> {len(jpeg_bytes)} bytes para stream)")
                else:
                    print("[ERRO] cv2.imencode falhou - não foi possível codificar o frame")
            else:
                print("[ERRO] cv2.imdecode retornou None - frame inválido ou corrompido")
                print(f"[DEBUG] Primeiros 100 bytes do frame_data: {frame_data[:100]}")
        except json.JSONDecodeError as e:
            print(f"[ERRO] Erro ao parsear JSON: {e}")
            print(f"[DEBUG] Payload (primeiros 200 chars): {msg.payload.decode('utf-8')[:200]}")
        except Exception as e:
            print(f"[ERRO] Erro ao processar frame: {e}")
            import traceback
            traceback.print_exc()
    
    elif topic == MQTT_TOPIC_STATUS:
        try:
            with lock:
                status_info = json.loads(msg.payload.decode('utf-8'))
        except Exception as e:
            print(f"[ERRO] Erro ao processar status: {e}")


def generate_frames():
    """Generator para streaming de frames."""
    last_frame_count = 0
    no_frame_warnings = 0
    print("[Stream] Generator iniciado")
    
    while True:
        with lock:
            frame = current_frame_jpeg
            current_count = frame_count
        
        if frame:
            # Verificar se é um novo frame
            if current_count != last_frame_count:
                print(f"[Stream] ✓ Enviando frame #{current_count} ({len(frame)} bytes)")
                last_frame_count = current_count
                no_frame_warnings = 0
            
            try:
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
            except Exception as e:
                print(f"[Stream] ERRO ao enviar frame: {e}")
        else:
            # Frame placeholder quando não há dados
            if current_count == 0:
                no_frame_warnings += 1
                if no_frame_warnings % 50 == 0:  # Avisar a cada 5 segundos (50 * 0.1s)
                    print(f"[Stream] ⚠ Aguardando frames... (count={current_count})")
                # Enviar frame placeholder (imagem preta 1x1)
                placeholder = b'\xff\xd8\xff\xe0\x00\x10JFIF\x00\x01\x01\x01\x00H\x00H\x00\x00\xff\xdb\x00C\x00\x08\x06\x06\x07\x06\x05\x08\x07\x07\x07\t\t\x08\n\x0c\x14\r\x0c\x0b\x0b\x0c\x19\x12\x13\x0f\x14\x1d\x1a\x1f\x1e\x1d\x1a\x1c\x1c $.\' ",#\x1c\x1c(7),01444\x1f\'9=82<.342\xff\xc0\x00\x11\x08\x00\x01\x00\x01\x01\x01\x11\x00\x02\x11\x01\x03\x11\x01\xff\xc4\x00\x14\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\xff\xc4\x00\x14\x10\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xda\x00\x08\x01\x01\x00\x00?\x00\xd2\xcf \xff\xd9'
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + placeholder + b'\r\n')
            time.sleep(0.1)


@app.route('/')
def index():
    """Página principal."""
    return render_template_string(HTML_TEMPLATE)


@app.route('/video_feed')
def video_feed():
    """Endpoint para streaming MJPEG."""
    print("[Stream] Cliente conectado ao /video_feed")
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/api/stats')
def api_stats():
    """API para estatísticas."""
    with lock:
        has_frame = current_frame_jpeg is not None
        frame_size = len(current_frame_jpeg) if has_frame else 0
        return jsonify({
            'frames': frame_count,
            'has_frame': has_frame,
            'frame_size': frame_size,
            'status': status_info.get('status', 'unknown'),
            'ip': status_info.get('ip', '--'),
            'uptime': status_info.get('uptime', 0)
        })


@app.route('/api/command', methods=['POST'])
def api_command():
    """API para enviar comandos."""
    data = request.json
    action = data.get('action', '')
    
    if mqtt_client and mqtt_client.is_connected():
        command = {"action": action}
        mqtt_client.publish(MQTT_TOPIC_COMMANDS, json.dumps(command))
        return jsonify({'success': True, 'message': f'Comando {action} enviado'})
    else:
        return jsonify({'success': False, 'message': 'MQTT não conectado'}), 500


def start_mqtt_client():
    """Inicia cliente MQTT em thread separada."""
    global mqtt_client
    
    if "SEU-CLUSTER" in MQTT_BROKER or "SEU-USUARIO" in MQTT_USER:
        print("\n" + "="*60)
        print("ERRO: Configure as credenciais do HiveMQ Cloud!")
        print("Edite mqtt_web_viewer.py e preencha MQTT_BROKER, MQTT_USER, MQTT_PASS")
        print("="*60 + "\n")
        return
    
    # Verificar se já existe um cliente
    if mqtt_client is not None:
        print("[MQTT] ⚠ Cliente MQTT já existe. Reutilizando...")
        if mqtt_client.is_connected():
            print("[MQTT] Cliente já está conectado.")
            return
        else:
            print("[MQTT] Cliente desconectado. Criando novo...")
    
    # Criar cliente com callback_api_version para compatibilidade
    # Usar clean_session=True para evitar problemas com sessões antigas
    try:
        # Tentar criar com VERSION1 (mais recente)
        mqtt_client = mqtt.Client(
            client_id=MQTT_CLIENT_ID, 
            clean_session=True,
            callback_api_version=mqtt.CallbackAPIVersion.VERSION1
        )
        print(f"[MQTT] Cliente criado com CallbackAPIVersion.VERSION1 (ID: {MQTT_CLIENT_ID})")
    except (AttributeError, TypeError):
        # Fallback para versões antigas do paho-mqtt
        try:
            mqtt_client = mqtt.Client(client_id=MQTT_CLIENT_ID, clean_session=True)
            print(f"[MQTT] Cliente criado (versão antiga do paho-mqtt) (ID: {MQTT_CLIENT_ID})")
        except:
            mqtt_client = mqtt.Client(clean_session=True)
            print(f"[MQTT] Cliente criado (versão muito antiga do paho-mqtt)")
    
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASS)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_subscribe = on_subscribe
    
    print(f"[MQTT] Callbacks configurados: on_connect={mqtt_client.on_connect}, on_message={mqtt_client.on_message}")
    
    try:
        mqtt_client.tls_set()
        # Habilitar reconexão automática
        mqtt_client.reconnect_delay_set(min_delay=1, max_delay=120)
        
        print(f"[MQTT] Conectando a {MQTT_BROKER}:{MQTT_PORT}...")
        result = mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        if result == 0:
            print(f"[MQTT] Conexão estabelecida. Iniciando loop...")
            # Usar loop_start() para rodar em thread separada
            mqtt_client.loop_start()
            print(f"[MQTT] Loop iniciado. Aguardando mensagens...")
            
            # Aguardar um pouco para a conexão se estabelecer completamente
            time.sleep(2)
            if mqtt_client.is_connected():
                print(f"[MQTT] ✓ Cliente conectado e pronto para receber mensagens")
            else:
                print(f"[MQTT] ⚠ Cliente não está conectado após loop_start()")
        else:
            print(f"[MQTT] Falha na conexão inicial. Código: {result}")
    except Exception as e:
        print(f"[ERRO] Falha ao conectar MQTT: {e}")
        import traceback
        traceback.print_exc()
        return
    
    # Manter a thread viva - o loop_start() já cuida do processamento em background
    # Não fazer reconexão manual, deixar o paho-mqtt fazer isso automaticamente
    try:
        while True:
            time.sleep(10)
            # Apenas logar status periodicamente
            if mqtt_client:
                if mqtt_client.is_connected():
                    # Tudo OK - não fazer nada
                    pass
                else:
                    print(f"[MQTT] Status: Desconectado (loop_start() vai reconectar automaticamente)")
    except KeyboardInterrupt:
        print("[MQTT] Encerrando cliente...")
        if mqtt_client:
            mqtt_client.loop_stop()
            mqtt_client.disconnect()


if __name__ == '__main__':
    print("="*60)
    print("ESP32-CAM Web Viewer via MQTT")
    print("="*60)
    
    # Verificar se já existe um cliente (evitar múltiplas instâncias)
    if mqtt_client is None:
        # Iniciar MQTT em thread separada
        mqtt_thread = threading.Thread(target=start_mqtt_client, daemon=True)
        mqtt_thread.start()
        
        # Aguardar um pouco para MQTT conectar
        time.sleep(3)
    else:
        print("[MQTT] Cliente já existe, reutilizando...")
    
    print("\nServidor web iniciando em http://localhost:5000")
    print("Pressione Ctrl+C para sair\n")
    
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)

