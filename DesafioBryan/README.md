# Desafio Bryan Kano

Desafio realizado pelo professor Bryan CAno para testar as habilidades de produção de dispostivios IOT, fomento de mentalidade competitiva e aperfeiçoamento do uso dos conceitos básicos de Arduino, C e C++ na turma 18, no módulo 04 do primeiro ano do Instituto de Tecnologia e Inovação (Inteli).

## Projeto EasyWay

O projeto EasyWay simplifica a comunicação entre diferentes partes de uma instituição sem depender de aplicações e redes externas externas como whatsapp, slack e wifi por exemplo.

Por meio de comunicação via rádio, arquitetura cliente/servidor e plataforma web integrada é possível conectar todas as salas e partes da sua organização por um baixo investimento.

O dispostivo de comunicação funciona por meio da comunicação via rádio da ESP32-WROOM-32u com antena externa no conector IPEX. 

### Dispositivo servidor

O dispositivo servidor é aquele que irá centralizar o recebimento e envio das mensagens e comandos por meio de protocolo half duplex.

Funcionalidades do dispositivo servidor:

- Enviar e receber mensagens de texto mensagens para os clientes
- Acionar ou bloquear remotamente a abertura das portas via plataforma web
- Se conecctar com N clientes

### Dispositivo cliente

O dispositivo cliente é aquele responsável por ficar em campo e ter o contato direto com o usuário do sistema. Também com arquitetura half duplex.

Funcionalidades do dispositivo cliente:

- Executar a abertura ou bloqueio das portas
- Enviar e receber mensagens do servidor via plataforma web

### Plataforma web

A plataforma web é a camada de comunicação visual do sistema com o usuário e permite que os requisitos funcionais do projeto sejam devidamente utilizados.

Funcionalidades da plataforma web:

- Integrar hardware e software
- Garantir a usabilidade dos requisitos funcionais

### Camada de Segurança

A camada de segurança opera por conta do risco de invasão e manipulação dos dados do sistema. Ela garante que a informação enviada seja efetivamente recebida.

Técnicas utilizadas:

- [Adicionar aqui]
- [Adicionar aqui]
