🛰️ Desafio Bryan Kano

Desafio realizado pelo professor Bryan Cano para testar as habilidades de produção de dispositivos IoT, fomentar a mentalidade competitiva e aperfeiçoar o uso dos conceitos básicos de Arduino, C e C++ na turma 18, no módulo 04 do primeiro ano do Instituto de Tecnologia e Inovação (Inteli).

🚀 Projeto EasyWay

O EasyWay simplifica a comunicação entre diferentes partes de uma instituição sem depender de redes externas como WhatsApp, Slack ou Wi-Fi.

Por meio de comunicação via rádio, arquitetura cliente/servidor e uma plataforma web local integrada, o sistema conecta todas as salas e setores de uma organização com baixo custo e alta eficiência.

O dispositivo utiliza o módulo ESP32-WROOM-32U, equipado com antena externa IPEX, garantindo maior alcance e estabilidade na comunicação.

🖥️ Dispositivo Servidor

O dispositivo servidor é o núcleo do sistema.
Ele centraliza o recebimento e envio das mensagens e comandos, e hospeda localmente a plataforma web, eliminando a necessidade de um computador intermediário ou rede externa.

Principais características:

Comunicação via rádio com múltiplos clientes (protocolo half duplex);

Servidor web embutido rodando diretamente na ESP32;

Interface acessível via navegador em qualquer dispositivo conectado à rede local da ESP32;

Registro de logs de comunicação (ID, comando, horário).

Funcionalidades:

Enviar e receber mensagens de texto dos clientes;

Acionar ou bloquear portas remotamente via interface web;

Exibir status em tempo real (online, offline, aberto, bloqueado).

🔧 Dispositivo Cliente

O dispositivo cliente é responsável pela interação direta com o usuário e pela execução dos comandos recebidos do servidor.

Funcionalidades:

Executar abertura ou bloqueio de portas;

Enviar e receber mensagens do servidor;

Exibir feedback visual com LEDs:

🟢 Conectado

🔴 Erro ou sem resposta

🟡 Transmitindo/recebendo

🌐 Plataforma Web Local

A plataforma web é executada localmente dentro do próprio ESP32 servidor, através de um servidor HTTP nativo desenvolvido em C++/Arduino IDE.
Ela atua como a camada visual e de controle do sistema, acessível via navegador por meio do endereço IP local da placa (exemplo: http://192.168.4.1).

Funcionalidades principais:

Controle remoto de portas e dispositivos;

Envio e recepção de mensagens;

Interface responsiva compatível com celulares e computadores;

Exibição de status em tempo real dos dispositivos conectados.

Tecnologias utilizadas:

Servidor HTTP da ESPAsyncWebServer ou WebServer padrão da ESP32;

HTML, CSS e JavaScript estáticos hospedados no SPIFFS ou LittleFS da placa.

🔒 Camada de Segurança

A camada de segurança garante a autenticidade e integridade das informações trocadas entre os dispositivos.

Técnicas utilizadas:

✅ Identificação única (ID) para cada cliente;

🔁 Confirmação de recebimento (ACK/NACK);

🧮 Checksum CRC32 para validar integridade de pacotes;

🧱 Protocolo padronizado em formato JSON:

{ "id": "cliente01", "cmd": "abrir", "status": "ok" }


Essas técnicas simples aumentam a confiabilidade sem elevar a complexidade.

⚡ Eficiência e Escalabilidade

O sistema foi otimizado para operar com baixo consumo de energia e mínima dependência externa.

Boas práticas aplicadas:

Modo sleep em períodos ociosos nos clientes;

Transmissão apenas em eventos relevantes;

Estrutura modular para fácil expansão futura.

🧭 Conclusão

O EasyWay representa uma solução IoT autossuficiente e segura para comunicação interna institucional.
Com a plataforma web local rodando diretamente no ESP32 servidor, o sistema dispensa redes externas, mantendo a simplicidade, o baixo custo e a confiabilidade da comunicação.