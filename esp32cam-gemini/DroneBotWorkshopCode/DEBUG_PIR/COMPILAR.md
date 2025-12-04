# Como Compilar o Código de Debug PIR

## 📋 Opção 1: Arduino IDE (Recomendado)

1. Abra o Arduino IDE
2. Vá em **File → Open**
3. Navegue até `DroneBotWorkshopCode/DEBUG_PIR/`
4. Abra `DEBUG_PIR.ino`
5. Configure a placa: **ESP32 Wrover Module**
6. Selecione a porta COM
7. Clique em **Upload**

## 📋 Opção 2: Arduino CLI

### Compilar:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32cam DroneBotWorkshopCode\DEBUG_PIR
```

### Fazer Upload:
```bash
arduino-cli upload --fqbn esp32:esp32:esp32cam -p COM17 DroneBotWorkshopCode\DEBUG_PIR
```

(Substitua `COM17` pela sua porta)

### Compilar e Fazer Upload em um comando:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32cam DroneBotWorkshopCode\DEBUG_PIR && arduino-cli upload --fqbn esp32:esp32:esp32cam -p COM17 DroneBotWorkshopCode\DEBUG_PIR
```

## ⚠️ Importante

- O código de debug está na pasta `DEBUG_PIR/` separada do código principal
- Isso evita conflitos de compilação
- Após testar, volte para o código principal: `DroneBotWorkshopCode.ino`


