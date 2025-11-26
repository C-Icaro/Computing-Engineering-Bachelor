#ifndef CONFIG_H
#define CONFIG_H

// =================== Configuração da câmera ===================
// Este código assume o módulo ESP32-CAM AI-Thinker
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#else
#error "Defina os pinos para o seu modelo de câmera"
#endif

// =================== Configurações Wi-Fi ===================
// Substitua pelos dados da sua rede 2.4 GHz
const char *WIFI_SSID = "Server";
const char *WIFI_PASS = "server123";

// =================== Configuração de YOLO ===================
// Caso possua um endpoint HTTP para inferência YOLO (por exemplo, um servidor Python),
// informe abaixo. Caso contrário, deixe vazio para utilizar apenas o placeholder local.
const char *YOLO_INFERENCE_ENDPOINT = "";

// =================== Configuração MQTT (HiveMQ Cloud) ===================
// PREENCHA COM SUAS CREDENCIAIS DO HIVEMQ CLOUD:
// 1. Acesse: https://www.hivemq.com/mqtt-cloud-broker/
// 2. Crie uma conta gratuita e um cluster
// 3. Copie as credenciais abaixo

const char *MQTT_BROKER = "7bf2160d4f4849f0885901cae207a260.s1.eu.hivemq.cloud";  // Ex: abc123.s1.eu.hivemq.cloud
const int MQTT_PORT = 8883;                                    // Porta TLS
const char *MQTT_USER = "esp32cam_001";                        // Seu username do HiveMQ
const char *MQTT_PASS = "Esp32cam_001";                          // Sua senha do HiveMQ
const char *MQTT_CLIENT_ID = "esp32cam_device_001";                  // ID único do cliente

// Tópicos MQTT
const char *MQTT_TOPIC_FRAMES = "esp32cam/frames";            // Publica frames aqui
const char *MQTT_TOPIC_STATUS = "esp32cam/status";            // Publica status aqui
const char *MQTT_TOPIC_COMMANDS = "esp32cam/commands";         // Recebe comandos aqui

// Configurações de publicação
const unsigned long MQTT_PUBLISH_INTERVAL = 2000;             // Publica a cada 2 segundos (0.5 FPS)
const int MQTT_JPEG_QUALITY = 12;                              // Qualidade JPEG (1-63, menor = melhor) - reduzido para frames menores
const int MQTT_MAX_FRAME_SIZE = 20000;                        // Tamanho máximo do frame JPEG em bytes (20KB) - reduzido para evitar problemas de memória
bool mqttEnabled = true;                                       // Ativar/desativar MQTT

#endif // CONFIG_H

