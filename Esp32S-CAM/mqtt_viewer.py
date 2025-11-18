#!/usr/bin/env python3
"""
Cliente MQTT para visualizar frames da ESP32-CAM via HiveMQ Cloud.

Uso:
    python mqtt_viewer.py

Configuração:
    Preencha as credenciais do HiveMQ Cloud no início do arquivo.
"""

import json
import base64
import argparse
from io import BytesIO
from typing import Optional

try:
    import paho.mqtt.client as mqtt
    import cv2
    import numpy as np
except ImportError:
    print("Erro: Instale as dependências:")
    print("  pip install paho-mqtt opencv-python numpy")
    exit(1)

# =================== Configurações HiveMQ Cloud ===================
# PREENCHA COM SUAS CREDENCIAIS:
MQTT_BROKER = "7bf2160d4f4849f0885901cae207a260.s1.eu.hivemq.cloud"  # Ex: abc123.s1.eu.hivemq.cloud
MQTT_PORT = 8883
MQTT_USER = "esp32cam_001"
MQTT_PASS = "Esp32cam_001"
MQTT_CLIENT_ID = "mqtt_viewer_python"

# Tópicos
MQTT_TOPIC_FRAMES = "esp32cam/frames"
MQTT_TOPIC_STATUS = "esp32cam/status"
MQTT_TOPIC_COMMANDS = "esp32cam/commands"

# =================== Variáveis Globais ===================
current_frame = None
frame_count = 0
window_name = "ESP32-CAM via MQTT"


def on_connect(client, userdata, flags, rc):
    """Callback quando conecta ao broker."""
    if rc == 0:
        print(f"[MQTT] Conectado ao broker: {MQTT_BROKER}")
        print(f"[MQTT] Inscrito em: {MQTT_TOPIC_FRAMES}")
        client.subscribe(MQTT_TOPIC_FRAMES)
        client.subscribe(MQTT_TOPIC_STATUS)
    else:
        print(f"[MQTT] Falha na conexão. Código: {rc}")


def on_message(client, userdata, msg):
    """Callback quando recebe mensagem."""
    global current_frame, frame_count
    
    topic = msg.topic
    
    if topic == MQTT_TOPIC_FRAMES:
        try:
            # Parsear JSON
            data = json.loads(msg.payload.decode('utf-8'))
            
            # Decodificar frame base64
            frame_data = base64.b64decode(data['data'])
            
            # Converter para imagem OpenCV
            nparr = np.frombuffer(frame_data, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
            if frame is not None:
                current_frame = frame
                frame_count += 1
                
                # Adicionar informações no frame
                cv2.putText(frame, f"Frame: {frame_count} | ID: {data.get('frame_id', 'N/A')}", 
                           (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                cv2.putText(frame, f"Size: {data.get('size', 0)} bytes", 
                           (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                
                print(f"[Frame {frame_count}] Recebido: {data.get('width')}x{data.get('height')} "
                      f"({data.get('size', 0)} bytes)")
            else:
                print("[ERRO] Falha ao decodificar frame")
                
        except Exception as e:
            print(f"[ERRO] Erro ao processar frame: {e}")
    
    elif topic == MQTT_TOPIC_STATUS:
        try:
            status = json.loads(msg.payload.decode('utf-8'))
            print(f"[Status] {status.get('status', 'N/A')} | IP: {status.get('ip', 'N/A')} | "
                  f"Uptime: {status.get('uptime', 0)}s")
        except Exception as e:
            print(f"[ERRO] Erro ao processar status: {e}")


def send_command(client, action: str, **kwargs):
    """Envia comando para ESP32-CAM."""
    command = {"action": action, **kwargs}
    payload = json.dumps(command)
    client.publish(MQTT_TOPIC_COMMANDS, payload)
    print(f"[Comando] Enviado: {action}")


def main():
    """Função principal."""
    parser = argparse.ArgumentParser(description="Visualizador MQTT para ESP32-CAM")
    parser.add_argument("--broker", default=MQTT_BROKER, help="Broker MQTT")
    parser.add_argument("--port", type=int, default=MQTT_PORT, help="Porta MQTT")
    parser.add_argument("--user", default=MQTT_USER, help="Username MQTT")
    parser.add_argument("--pass", dest="password", default=MQTT_PASS, help="Password MQTT")
    parser.add_argument("--no-display", action="store_true", help="Não exibir janela (apenas logs)")
    
    args = parser.parse_args()
    
    # Verificar se credenciais foram preenchidas
    if "SEU-CLUSTER" in args.broker or "SEU-USUARIO" in args.user:
        print("\n" + "="*60)
        print("ERRO: Configure as credenciais do HiveMQ Cloud!")
        print("="*60)
        print("Edite o arquivo mqtt_viewer.py e preencha:")
        print("  - MQTT_BROKER")
        print("  - MQTT_USER")
        print("  - MQTT_PASS")
        print("="*60 + "\n")
        return
    
    # Criar cliente MQTT
    client = mqtt.Client(client_id=MQTT_CLIENT_ID)
    client.username_pw_set(args.user, args.password)
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Conectar com TLS
    print(f"[MQTT] Conectando a {args.broker}:{args.port}...")
    try:
        client.tls_set()  # Habilita TLS
        client.connect(args.broker, args.port, 60)
    except Exception as e:
        print(f"[ERRO] Falha ao conectar: {e}")
        return
    
    # Iniciar loop em thread separada
    client.loop_start()
    
    print("\n" + "="*60)
    print("ESP32-CAM MQTT Viewer")
    print("="*60)
    print("Aguardando frames...")
    print("\nComandos disponíveis:")
    print("  'q' - Sair")
    print("  'y' - Toggle YOLO")
    print("  'm' - Toggle MQTT")
    print("  'r' - Reiniciar ESP32")
    print("="*60 + "\n")
    
    try:
        while True:
            # Exibir frame se disponível
            if current_frame is not None and not args.no_display:
                cv2.imshow(window_name, current_frame)
            
            # Processar teclas
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('y'):
                send_command(client, "toggle_yolo")
            elif key == ord('m'):
                send_command(client, "toggle_mqtt")
            elif key == ord('r'):
                send_command(client, "restart")
            
    except KeyboardInterrupt:
        print("\n[Saindo...]")
    finally:
        client.loop_stop()
        client.disconnect()
        if not args.no_display:
            cv2.destroyAllWindows()
        print(f"[MQTT] Desconectado. Total de frames recebidos: {frame_count}")


if __name__ == "__main__":
    main()

