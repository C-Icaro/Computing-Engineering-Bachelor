# Guia para Corrigir Boot Loop da ESP32-CAM

## Problema
A ESP32-CAM está em boot loop com erro:
```
E (29) qio_mode: Failed to set QIE bit, not enabling QIO mode
assert failed: __esp_system_init_fn_init_flash
```

## Solução: Erase e Reflash Completo

### Passo 1: Identificar a Porta COM
Execute:
```powershell
arduino-cli board list
```
Anote a porta COM da sua ESP32-CAM (geralmente COM3, COM4, COM7, COM8, ou COM17)

### Passo 2: Fazer Erase Completo da Flash
**IMPORTANTE**: Isso apagará tudo da ESP32-CAM!

```powershell
# Se tiver esptool.py instalado:
python -m esptool --port COM3 erase_flash

# OU usando arduino-cli (se suportar):
arduino-cli upload --fqbn esp32:esp32:esp32cam -p COM3 --upload-property erase_flash=true
```

### Passo 3: Compilar com Configurações Corretas
```powershell
arduino-cli compile --fqbn esp32:esp32:esp32cam `
    --build-property "build.flash_mode=dio" `
    --build-property "build.flash_freq=80m" `
    --build-property "build.flash_size=4MB" `
    --build-property "build.partitions=default" `
    Esp32S-CAM\Esp32S-CAM.ino
```

### Passo 4: Fazer Upload
```powershell
arduino-cli upload --fqbn esp32:esp32:esp32cam `
    --upload-property "upload.flash_mode=dio" `
    -p COM3 `
    Esp32S-CAM\Esp32S-CAM.ino
```

**Substitua COM3 pela porta correta da sua ESP32-CAM!**

### Passo 5: Verificar no Serial Monitor
Abra o Serial Monitor (115200 baud) e verifique se:
- A câmera inicializa corretamente
- O Wi-Fi conecta
- O IP é exibido

## Script Automatizado

Use o script `flash_esp32cam.ps1`:
```powershell
.\Esp32S-CAM\flash_esp32cam.ps1 -Port COM3
```

## Alternativa: Usar Arduino IDE

Se preferir usar a IDE do Arduino:

1. **Tools → Board → ESP32 Arduino → AI Thinker ESP32-CAM**
2. Configure:
   - **Flash Mode**: DIO
   - **Flash Frequency**: 80MHz
   - **Flash Size**: 4MB (32Mb)
   - **Partition Scheme**: Default 4MB with spiffs (or Huge APP)
   - **PSRAM**: Enabled
3. **Tools → Erase Flash: All Flash Contents**
4. Faça upload normalmente

## Troubleshooting

### Se o erase falhar:
- Pressione o botão RESET na ESP32-CAM durante o erase
- Tente outra porta COM
- Verifique se os drivers USB estão instalados

### Se ainda houver boot loop:
- Verifique se a placa é realmente ESP32-CAM (não ESP32 comum)
- Tente um cabo USB diferente
- Verifique se há jumper/configuração física necessária

