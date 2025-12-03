# Documentação Técnica - ESP32-CAM com Gemini 2.5 Flash-Lite

## 1. Visão Geral do Projeto

Sistema embarcado que captura imagens através de uma câmera ESP32-CAM, envia para o modelo de IA Gemini 2.5 Flash-Lite da Google para detecção de pessoas, e exibe o resultado através de LEDs e interface web local.

---

## 2. Hardware Utilizado

### 2.1. ESP32-CAM (AI-Thinker)

- **Microcontrolador**: ESP32 (dual-core, 240 MHz)
- **Câmera**: OV2640 (2 MP)
- **Memória**: RAM interna + PSRAM opcional
- **Wi-Fi**: 802.11 b/g/n integrado
- **GPIO**: 34 pinos configuráveis

### 2.2. Componentes Periféricos

- **Botão push-button**: GPIO 13 (pull-up externo, ativo em LOW)
- **LED vermelho**: GPIO 14 (indica pessoa detectada)
- **LED verde**: GPIO 15 (indica ausência de pessoa)
- **Resistor pull-up**: ~1kΩ (para o botão)
- **Resistores limitadores**: ~220Ω (para os LEDs)

---

## 3. Software e Bibliotecas

### 3.1. Framework e Ambiente de Desenvolvimento

- **Arduino Core for ESP32**: Framework base
- **Linguagem**: C++ (estilo Arduino)
- **Compilador**: GCC (toolchain ESP-IDF)

### 3.2. Bibliotecas do ESP-IDF

#### `esp_camera.h`
- **Função**: Inicialização e controle da câmera OV2640
- **Estruturas principais**: `camera_config_t`, `camera_fb_t`
- **Funções principais**: `esp_camera_init()`, `esp_camera_fb_get()`, `esp_camera_fb_return()`
- **Formatos suportados**: JPEG, YUV422, RGB565, GRAYSCALE
- **Resoluções disponíveis**: QVGA, VGA, SVGA, XGA, SXGA, UXGA

#### `soc/soc.h` e `soc/rtc_cntl_reg.h`
- **Função**: Controle de registradores do SoC
- **Uso específico**: Desabilitação do brownout detector (`RTC_CNTL_BROWN_OUT_REG`)

#### `driver/rtc_io.h`
- **Função**: Controle de GPIOs do RTC (não usado diretamente, mas incluído)

### 3.3. Bibliotecas de Rede Wi-Fi

#### `WiFi.h` (ESP32)
- **Função**: Cliente e servidor Wi-Fi
- **Classes principais**: `WiFi`, `WiFiServer`, `WiFiClient`
- **Métodos principais**: `WiFi.begin()`, `WiFi.localIP()`, `WiFi.status()`
- **Modos**: `WIFI_STA` (station), `WIFI_AP` (access point)

#### `WiFiClientSecure.h` (ESP32)
- **Função**: Cliente HTTPS/TLS
- **Classe**: `WiFiClientSecure`
- **Métodos principais**: `connect()`, `setInsecure()`, `print()`, `available()`
- **Protocolo**: TLS 1.2/1.3 sobre TCP

### 3.4. Biblioteca de Codificação

#### `mbedtls/base64.h` (mbed TLS)
- **Função**: Codificação/decodificação Base64
- **Função principal**: `mbedtls_base64_encode()`
- **Uso**: Converter JPEG binário para string Base64 (necessário para JSON)

---

## 4. Protocolos de Comunicação

### 4.1. Wi-Fi (IEEE 802.11)

- **Padrão**: 802.11 b/g/n (2.4 GHz)
- **Modo**: Station (STA)
- **Autenticação**: WPA/WPA2-PSK
- **Configuração**: SSID e senha hardcoded no firmware

### 4.2. HTTP/1.1

- **Cliente HTTP**: Requisições ao Gemini
- **Servidor HTTP**: Interface web local (porta 80)
- **Métodos**: GET, POST
- **Headers**: `Content-Type`, `Content-Length`, `Host`, `Connection`

### 4.3. HTTPS/TLS

- **Porta**: 443
- **Host**: `generativelanguage.googleapis.com`
- **Verificação de certificado**: Desabilitada no MVP (`setInsecure()`)
- **Criptografia**: TLS 1.2/1.3

### 4.4. JSON

- **Formato**: JSON para payload da API Gemini
- **Estrutura**:
```json
{
  "contents": [{
    "parts": [
      {"text": "instrução"},
      {"inline_data": {
        "mime_type": "image/jpeg",
        "data": "base64_encoded_image"
      }}
    ]
  }]
}
```

---

## 5. API do Google Gemini

### 5.1. Modelo Utilizado

- **Nome**: Gemini 2.5 Flash-Lite
- **Tipo**: Multimodal (texto + imagem)
- **Endpoint**: `/v1beta/models/gemini-2.5-flash-lite:generateContent`
- **Método**: POST

### 5.2. Autenticação

- **Tipo**: API Key via query string (`?key=...`)
- **Armazenamento**: Constante no firmware (risco de segurança)

### 5.3. Payload Enviado

- **Texto**: Prompt de instrução para detecção de pessoa
- **Imagem**: JPEG codificado em Base64 via `inline_data`
- **MIME type**: `image/jpeg`

### 5.4. Resposta Esperada

- **Formato**: JSON
- **Parsing**: Busca simples por substring (`"person"` ou `"no_person"`)

---

## 6. Gerenciamento de Memória

### 6.1. Frame Buffer da Câmera

- **Estrutura**: `camera_fb_t*`
- **Campos principais**: `buf` (ponteiro), `len` (tamanho), `format`, `width`, `height`
- **Ciclo de vida**: `esp_camera_fb_get()` → uso → `esp_camera_fb_return()`
- **Configuração**: `fb_count = 1` (um buffer apenas)

### 6.2. Alocação Dinâmica

- **Base64**: `malloc()` / `free()` para buffer temporário
- **Cópia da imagem**: `malloc()` / `free()` para armazenar última captura
- **Risco**: Fragmentação de memória e OOM em uso prolongado

### 6.3. Otimizações Implementadas

- **Resolução**: `FRAMESIZE_SVGA` (800x600) para reduzir uso de memória
- **Qualidade JPEG**: 12 (escala 10-63, menor número = maior qualidade)
- **Liberação imediata**: Frame buffer liberado logo após uso

---

## 7. Interface Web Local

### 7.1. Servidor HTTP Embarcado

- **Classe**: `WiFiServer` (porta 80)
- **Protocolo**: HTTP/1.1
- **Concorrência**: Um cliente por vez (limitação do ESP32)

### 7.2. Rotas Implementadas

- **`GET /`**: Página HTML com decisão e imagem
- **`GET /image.jpg`**: Última imagem JPEG capturada

### 7.3. HTML Gerado

- **Estrutura**: HTML5 básico
- **Elementos**: `<h1>`, `<p>`, `<img>`, `<strong>`
- **Cache busting**: Parâmetro `?ts=` no `src` da imagem
- **Encoding**: UTF-8

---

## 8. Eletrônica e GPIO

### 8.1. Configuração de Pinos

- **Botão**: `INPUT` (GPIO 13, pull-up externo)
- **LEDs**: `OUTPUT` (GPIO 14 e 15)
- **Câmera**: 16 pinos dedicados (D0-D7, XCLK, PCLK, VSYNC, HREF, SDA, SCL, PWDN, RESET)

### 8.2. Debounce de Botão

- **Algoritmo**: Debounce por software
- **Delay**: 50 ms
- **Lógica**: Detecta transição estável antes de processar

### 8.3. Controle de LEDs

- **Lógica**: Mutuamente exclusivos (um HIGH, outro LOW)
- **Vermelho**: Pessoa detectada
- **Verde**: Sem pessoa

---

## 9. Fluxo de Dados

### 9.1. Captura

1. Botão pressionado → GPIO 13 LOW
2. Debounce confirma estado estável
3. `esp_camera_fb_get()` captura frame JPEG
4. Frame armazenado em `camera_fb_t*`

### 9.2. Processamento

1. JPEG convertido para Base64 (`mbedtls_base64_encode()`)
2. Payload JSON montado com prompt + imagem
3. Requisição HTTPS POST ao Gemini

### 9.3. Resposta e Ação

1. Resposta JSON lida via `WiFiClientSecure`
2. Parsing por substring (`"person"` / `"no_person"`)
3. LEDs atualizados conforme resultado
4. Cópia da imagem salva para web

### 9.4. Visualização Web

1. Cliente acessa `http://IP_ESP32`
2. Servidor envia HTML com decisão e `<img src="/image.jpg">`
3. Cliente solicita `/image.jpg`
4. Servidor envia JPEG armazenado

---

## 10. Segurança e Limitações

### 10.1. Segurança

- **API Key exposta**: Armazenada no firmware (risco de segurança)
- **TLS desabilitado**: Verificação de certificado desabilitada (`setInsecure()`)
- **Servidor HTTP**: Sem autenticação
- **Wi-Fi**: Credenciais hardcoded

### 10.2. Limitações

- **Memória**: Risco de OOM com imagens grandes
- **Concorrência**: Servidor web atende um cliente por vez
- **Parsing**: Busca simples por substring (não usa JSON parser)
- **Persistência**: Última imagem apenas em RAM (perdida ao reiniciar)

---

## 11. Especificações Técnicas

### 11.1. Câmera

- **Sensor**: OV2640
- **Resolução**: 800x600 (SVGA)
- **Formato**: JPEG
- **Qualidade**: 12/63
- **Taxa de quadros**: Sob demanda

### 11.2. Rede

- **Wi-Fi**: 802.11 b/g/n (2.4 GHz)
- **HTTP**: Porta 80 (servidor local)
- **HTTPS**: Porta 443 (cliente Gemini)
- **Timeout**: 20s (Wi-Fi), 10s (Gemini)

### 11.3. Performance

- **Latência de captura**: ~100-500 ms
- **Latência Gemini**: ~2-5 s
- **Tamanho típico JPEG**: ~30-80 KB (SVGA)
- **Tamanho Base64**: ~40-107 KB (+33%)

---

## 12. Dependências e Versões

### 12.1. Arduino Core for ESP32

- **Versão recomendada**: 2.x ou superior
- **Repositório**: https://github.com/espressif/arduino-esp32

### 12.2. ESP-IDF

- **Versão base**: 4.x ou superior
- **Componentes**: `esp_camera`, `mbedtls`, `wifi`, `driver`

### 12.3. Bibliotecas Nativas

- `esp_camera.h`: ESP-IDF
- `WiFi.h`: Arduino Core ESP32
- `WiFiClientSecure.h`: Arduino Core ESP32
- `mbedtls/base64.h`: mbed TLS (ESP-IDF)

---

## 13. Arquitetura do Sistema

```
┌─────────────────┐
│   Botão GPIO13  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  ESP32-CAM      │
│  ┌───────────┐  │
│  │  Câmera   │──┼──► Captura JPEG
│  └───────────┘  │
│  ┌───────────┐  │
│  │   Wi-Fi   │──┼──► Conecta à rede
│  └───────────┘  │
│  ┌───────────┐  │
│  │  Servidor │──┼──► HTTP na porta 80
│  │    Web    │  │
│  └───────────┘  │
└────────┬────────┘
         │
         ├──► HTTPS POST ──► Google Gemini API
         │                  (gemini-2.5-flash-lite)
         │
         ├──► LEDs (GPIO 14/15)
         │
         └──► Servidor Web ──► Navegador
                                (visualização)
```

---

## 14. Referências

- **ESP32-CAM**: https://github.com/espressif/esp32-camera
- **Arduino Core ESP32**: https://github.com/espressif/arduino-esp32
- **Google Gemini API**: https://ai.google.dev/docs
- **mbed TLS**: https://github.com/Mbed-TLS/mbedtls
- **IEEE 802.11**: https://standards.ieee.org/standard/802_11-2016.html

---

Esta documentação cobre todas as tecnologias, bibliotecas, protocolos e componentes utilizados no projeto ESP32-CAM com Gemini 2.5 Flash-Lite.

