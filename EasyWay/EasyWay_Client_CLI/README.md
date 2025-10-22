# EasyWay Client CLI

Sistema de comunicação IoT via Wi-Fi com arquitetura modular para dispositivos clientes ESP32.

## 📋 Funcionalidades

- **Comunicação Wi-Fi UDP**: Conecta-se ao servidor EasyWay
- **Controle de Dispositivos**: Gerencia relés, LEDs e botões
- **Interface Física**: LEDs de status e botão local
- **Heartbeat Automático**: Mantém conexão ativa com o servidor
- **Comandos Locais e Remotos**: Controle via botão ou comandos via rede
- **Segurança**: Validação de mensagens com checksum CRC32

## 🔧 Arquitetura Modular

### Módulos Principais

1. **WiFiCommunication**: Gerencia comunicação UDP com o servidor
2. **DeviceController**: Controla dispositivos físicos (relés, LEDs, botões)
3. **SecurityLayer**: Validação e criação de mensagens seguras
4. **StatusManager**: Monitora e gerencia status do sistema

### Pinos Utilizados (Seguros para ESP32)

- **GPIO2**: LED Verde (Conectado)
- **GPIO4**: LED Vermelho (Erro)
- **GPIO5**: LED Amarelo (Transmitindo)
- **GPIO13**: Relé para controle da porta
- **GPIO14**: Botão para comandos locais

## 🚀 Configuração

### Configurações do Sistema

```cpp
const char* WIFI_SSID = "EasyWay-Server";
const char* WIFI_PASSWORD = "easyway123";
const int SERVER_PORT = 8080;
const String DEVICE_ID = "cliente01";
```

### Compilação

```bash
arduino-cli compile --fqbn esp32:esp32:esp32da .
```

## 📡 Protocolo de Comunicação

### Tipos de Mensagem

- **status**: Informa status do dispositivo
- **command**: Executa comandos (abrir/fechar/status)
- **heartbeat**: Mantém conexão ativa
- **ack**: Confirmação de recebimento

### Formato das Mensagens

```json
{
  "id": "cliente01",
  "type": "status",
  "status": "online",
  "timestamp": "12345",
  "checksum": 1234567890
}
```

## 🎮 Controles

### Comandos Remotos (via servidor)

- **abrir**: Abre a porta por 1 segundo
- **fechar**: Fecha a porta
- **status**: Retorna status atual da porta

### Comandos Locais (botão físico)

- **Pressionar botão**: Alterna estado da porta (abrir/fechar)
- **Status automático**: Envia status após comando local

## 🔍 Monitoramento

### LEDs de Status

- **Verde**: Conectado ao servidor
- **Vermelho**: Erro de conexão ou sistema
- **Amarelo**: Transmitindo dados (200ms)

### Serial Monitor

- Logs detalhados de todas as operações
- Status de conexão em tempo real
- Informações de rede e dispositivos

## 🔒 Segurança

- **Validação CRC32**: Verifica integridade das mensagens
- **IDs Válidos**: Validação de formato dos identificadores
- **Timestamps**: Controle de tempo das mensagens
- **Campos Obrigatórios**: Validação de estrutura JSON

## 📊 Status do Sistema

### Indicadores Visuais

- LEDs indicam status em tempo real
- Serial monitor mostra logs detalhados
- Heartbeat mantém conexão ativa

### Informações de Rede

- IP do dispositivo
- IP do servidor
- Força do sinal Wi-Fi
- Endereço MAC

## 🛠️ Solução de Problemas

### Problemas Comuns

1. **Não conecta ao Wi-Fi**
   - Verificar SSID e senha
   - Verificar se o servidor está ativo
   - LED vermelho indica erro de conexão

2. **Não recebe comandos**
   - Verificar se está conectado (LED verde)
   - Verificar logs no Serial Monitor
   - Testar heartbeat

3. **Dispositivos não respondem**
   - Verificar configuração dos pinos
   - Testar botão local
   - Verificar alimentação dos relés

### Logs de Debug

O sistema gera logs detalhados para facilitar o debug:

```
EasyWay Client CLI - Sistema inicializado com sucesso!
ID do dispositivo: cliente01
Conectado à rede: EasyWay-Server
Sinal de presença enviado
```

## 📈 Próximas Funcionalidades

- [ ] Interface web local
- [ ] Configuração via OTA
- [ ] Múltiplos dispositivos
- [ ] Logs persistentes
- [ ] Configuração de pinos via software

## 🤝 Contribuição

Para contribuir com o projeto:

1. Fork o repositório
2. Crie uma branch para sua feature
3. Commit suas mudanças
4. Push para a branch
5. Abra um Pull Request

## 📄 Licença

Este projeto está sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.

---

**Desenvolvido para Inteli - Instituto de Tecnologia e Liderança**
