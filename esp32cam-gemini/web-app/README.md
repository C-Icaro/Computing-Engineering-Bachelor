# ESP32-CAM + Gemini - Visualizador Web

Aplicação web Next.js para visualizar imagens capturadas pela ESP32-CAM com detecção de pessoas via Gemini AI.

## 📋 Índice

- [Visão Geral](#visão-geral)
- [Funcionalidades](#funcionalidades)
- [Pré-requisitos](#pré-requisitos)
- [Instalação e Setup](#instalação-e-setup)
- [Deploy na Vercel](#deploy-na-vercel)
- [Configuração da ESP32](#configuração-da-esp32)
- [Como Funciona](#como-funciona)
- [Endpoints da API](#endpoints-da-api)
- [Troubleshooting](#troubleshooting)
- [Notas de Segurança](#notas-de-segurança)

## 🎯 Visão Geral

Este projeto consiste em:

1. **ESP32-CAM**: Captura imagens e envia para análise via Gemini AI
2. **Gemini AI**: Analisa as imagens e detecta presença de pessoas
3. **Web App (Next.js)**: Recebe e exibe as imagens capturadas em tempo real

## ✨ Funcionalidades

- ✅ Visualização da última imagem capturada
- ✅ Exibição da decisão do Gemini (pessoa detectada ou não)
- ✅ Auto-atualização a cada 5 segundos
- ✅ Atualização manual
- ✅ Interface responsiva e moderna
- ✅ Timestamp da captura
- ✅ Suporte a detecção via PIR ou botão físico

## 📦 Pré-requisitos

### Para o Web App:
- Node.js 18+ instalado
- Conta no [Vercel](https://vercel.com)
- Git (para versionamento)

### Para a ESP32:
- ESP32-CAM
- Arduino IDE ou PlatformIO
- Biblioteca ESP32 instalada
- Wi-Fi configurado

## 🚀 Instalação e Setup

### 1. Instalar Dependências

```bash
cd web-app
npm install
```

### 2. Testar Localmente (Opcional)

```bash
npm run dev
```

Acesse `http://localhost:3000` para ver a aplicação.

### 3. Build de Produção

```bash
npm run build
npm start
```

## 🌐 Deploy na Vercel

### Opção A: Via GitHub (Recomendado)

1. **Faça push do código para um repositório GitHub**

2. **Acesse [vercel.com](https://vercel.com) e clique em "New Project"**

3. **Importe o repositório**

4. **Configure o projeto:**
   - **Framework Preset:** `Next.js`
   - **Root Directory:** `esp32cam-gemini/web-app` (ou `web-app` se estiver na raiz)
   - **Build Command:** (deixe vazio para detecção automática)
   - **Output Directory:** (deixe vazio para detecção automática)
   - **Install Command:** (deixe vazio para detecção automática)

5. **Clique em "Deploy"**

6. **⚠️ IMPORTANTE: Desabilitar Vercel Authentication**
   - Após o deploy, vá em **Settings → Deployment Protection**
   - Desative **"Vercel Authentication"** ou **"Password Protection"**
   - Isso é necessário para que a ESP32 possa enviar requisições
   - Veja [Troubleshooting - Erro 401](#erro-401-unauthorized) para mais detalhes

7. **Anotar a URL:**
   - Após o deploy, você receberá uma URL como: `https://seu-app.vercel.app`
   - **IMPORTANTE:** Anote essa URL para configurar na ESP32

### Opção B: Via CLI

```bash
npm i -g vercel
cd web-app
vercel
```

Siga as instruções no terminal.

## 🔧 Configuração da ESP32

### 1. Abrir o Arquivo do Código

Abra `DroneBotWorkshopCode/DroneBotWorkshopCode.ino` no Arduino IDE.

### 2. Configurar Wi-Fi

Localize e configure suas credenciais Wi-Fi:

```cpp
const char* WIFI_SSID     = "SeuWiFi";
const char* WIFI_PASSWORD = "SuaSenha";
```

### 3. Configurar API Key do Gemini

```cpp
const char* GEMINI_API_KEY = "SUA_API_KEY_AQUI";
```

### 4. Configurar URL do Web App

Localize a seção de configuração da plataforma web (linha ~63):

```cpp
// ==== Configurações da Plataforma Web (Vercel) ====
const char* WEB_APP_HOST = "seu-app.vercel.app";  // SEM https:// e SEM / no final
const char* WEB_APP_PATH = "/api/upload";
const uint16_t WEB_APP_PORT = 443;  // HTTPS
```

**⚠️ IMPORTANTE:**
- Use apenas o hostname, **sem** `https://`
- **Sem** barra (`/`) no final
- Exemplo correto: `monitoramento-de-91k7y08uy-carlosicaro13-gmailcoms-projects.vercel.app`
- Exemplo incorreto: `https://monitoramento-de-...vercel.app/`

### 5. Compilar e Fazer Upload

1. Selecione a placa: **ESP32 Wrover Module** (ou similar)
2. Selecione a porta COM
3. Compile e faça upload

### 6. Verificar Funcionamento

Abra o Serial Monitor (115200 baud) e verifique:

```
Wi-Fi conectado. IP: 10.79.252.176
Enviando imagem para plataforma web...
Conectando a seu-app.vercel.app...
Resposta da plataforma web:
HTTP/1.1 200 OK
...
Envio para plataforma web concluído.
```

## 📡 Como Funciona

1. **ESP32-CAM captura imagem** (via botão físico ou sensor PIR)
2. **Envia para Gemini AI** para análise e detecção de pessoas
3. **Envia imagem + decisão** para a plataforma web via HTTPS
4. **Plataforma web armazena** a última imagem em memória
5. **Interface web exibe** a última imagem capturada com auto-refresh

## 🌐 Endpoints da API

### `POST /api/upload`

Recebe imagem e decisão da ESP32-CAM.

**Body (JSON):**
```json
{
  "image": "data:image/jpeg;base64,...",
  "decision": "person" | "no_person"
}
```

**Resposta:**
```json
{
  "success": true,
  "filename": "capture-2024-01-01T12-00-00.jpg",
  "decision": "person",
  "imageUrl": "data:image/jpeg;base64,...",
  "timestamp": "2024-01-01T12:00:00.000Z"
}
```

### `GET /api/latest`

Retorna a última imagem recebida.

**Resposta:**
```json
{
  "filename": "capture-2024-01-01T12-00-00.jpg",
  "decision": "person",
  "imageUrl": "data:image/jpeg;base64,...",
  "timestamp": "2024-01-01T12:00:00.000Z"
}
```

Ou `null` se nenhuma imagem foi recebida ainda.

### `GET /api/test`

Rota de teste para verificar se as rotas da API estão funcionando.

**Resposta:**
```json
{
  "message": "API route is working!",
  "timestamp": "2024-01-01T12:00:00.000Z"
}
```

## 🔍 Troubleshooting

> **📖 Para um guia completo de troubleshooting com soluções detalhadas, consulte [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)**

### Erro 401 Unauthorized

**Sintoma:** Serial Monitor mostra `HTTP/1.1 401 Unauthorized`

**Causa:** O projeto na Vercel está protegido por autenticação.

**Solução:**
1. Acesse o projeto na Vercel
2. Vá em **Settings → Deployment Protection**
3. Desative **"Vercel Authentication"** ou **"Password Protection"**
4. Salve as alterações
5. Teste novamente

**Verificação:**
- Acesse `https://seu-app.vercel.app/api/test` no navegador
- Se pedir login/senha, a proteção ainda está ativa
- Se abrir normalmente, está correto

### Erro 404 NOT_FOUND nas Rotas da API

**Sintoma:** Todas as rotas da API retornam 404

**Causa:** Configuração incorreta do Root Directory ou Framework Preset na Vercel.

**Solução:**
1. Vá em **Settings → General**
2. Verifique:
   - **Root Directory:** `esp32cam-gemini/web-app` (sem barra no final)
   - **Framework Preset:** `Next.js`
   - **Build Command:** (deixe vazio)
   - **Output Directory:** (deixe vazio)
3. Faça um redeploy

**Verificação:**
- Verifique os Build Logs
- Deve aparecer "Compiled successfully"
- Deve listar as rotas da API sendo criadas

### Imagens Não Aparecem no Web App

**Sintoma:** `/api/latest` retorna `null`, mesmo após enviar imagens

**Diagnóstico:**

1. **Verifique o Serial Monitor da ESP32:**
   - Deve aparecer: `"Enviando imagem para plataforma web..."`
   - Deve aparecer: `"HTTP/1.1 200 OK"` (não 401 ou 404)
   - Deve aparecer: `"Envio para plataforma web concluído."`

2. **Verifique os Logs da Vercel:**
   - Vá em **Deployments → Logs**
   - Procure por: `"API /api/upload chamada:"`
   - Procure por: `"Imagem processada com sucesso"`

3. **Possíveis Problemas:**
   - **Erro 401:** Desabilite Vercel Authentication (veja acima)
   - **Erro de conexão:** Verifique se a URL está correta (sem https:// e sem /)
   - **Timeout:** Verifique a conexão Wi-Fi da ESP32
   - **Payload muito grande:** Reduza a qualidade da imagem no código da ESP32

### Erro de Conexão HTTPS

**Sintoma:** `"Falha ao conectar ao servidor web"`

**Soluções:**
- Verifique se a URL está correta (sem https:// e sem /)
- Verifique se a porta está como `443`
- Verifique a conexão Wi-Fi da ESP32
- O código já usa `client.setInsecure()` para ignorar verificação de certificado

### Build Falha na Vercel

**Sintoma:** Build não completa ou falha

**Soluções:**
1. Verifique os Build Logs para erros específicos
2. Certifique-se de que todos os arquivos foram commitados
3. Verifique se o Root Directory está correto
4. Tente fazer um redeploy

### Warning: Invalid next.config.js

**Sintoma:** Warning sobre configuração inválida no `next.config.js`

**Causa:** Configuração obsoleta `api` no Next.js 14

**Solução:** Já corrigido no código. Se aparecer, verifique se o `next.config.js` não contém a configuração `api`.

## 📝 Notas Importantes

### Armazenamento de Imagens

- As imagens são armazenadas **em memória** (não persistem após reiniciar)
- Cada reinicialização do servidor limpa as imagens
- Para produção, considere usar:
  - Banco de dados (PostgreSQL, MongoDB)
  - Storage (AWS S3, Cloudinary, Vercel Blob)
  - Sistema de arquivos persistente

### Limites

- A aplicação aceita imagens até **10MB**
- O limite pode ser ajustado em `pages/api/upload.js`
- Imagens muito grandes podem causar timeout

### Ambiente Serverless

- O Next.js na Vercel roda em ambiente serverless
- O estado em memória pode não ser compartilhado entre instâncias diferentes
- Para produção, use storage externo

## 🔒 Notas de Segurança

### Para Produção

Considere adicionar:

1. **Autenticação na API:**
   - Token de acesso
   - API Key
   - OAuth

2. **Rate Limiting:**
   - Limitar requisições por IP
   - Prevenir abuso

3. **Validação de Origem (CORS):**
   - Restringir quais domínios podem acessar a API
   - Validar origem das requisições

4. **Armazenamento Seguro:**
   - Não armazenar imagens sensíveis em memória
   - Usar storage criptografado
   - Implementar políticas de retenção

5. **Monitoramento:**
   - Logs de acesso
   - Alertas de erro
   - Métricas de uso

## 📚 Estrutura do Projeto

```
web-app/
├── pages/
│   ├── api/
│   │   ├── upload.js      # Recebe imagens da ESP32
│   │   ├── latest.js       # Retorna última imagem
│   │   ├── test.js         # Rota de teste
│   │   └── store.js        # Armazenamento em memória
│   └── index.js            # Interface web
├── next.config.js          # Configuração do Next.js
├── package.json            # Dependências
└── README.md               # Esta documentação
```

## 🛠️ Desenvolvimento

### Scripts Disponíveis

```bash
npm run dev      # Desenvolvimento local
npm run build    # Build de produção
npm start        # Iniciar servidor de produção
npm run lint     # Verificar código
```

### Adicionar Novas Funcionalidades

1. **Novas rotas da API:** Adicione em `pages/api/`
2. **Novas páginas:** Adicione em `pages/`
3. **Componentes:** Crie uma pasta `components/` se necessário

## 📞 Suporte

Para problemas específicos, consulte:

- **[TROUBLESHOOTING.md](./TROUBLESHOOTING.md)** - Guia completo de troubleshooting
- **[DOCUMENTACAO.md](./DOCUMENTACAO.md)** - Índice de toda a documentação
- Logs do Serial Monitor da ESP32
- Logs da Vercel (Deployments → Logs)
- Build Logs da Vercel

## 📄 Licença

Este projeto é open source e está disponível para uso educacional e pessoal.

---

**Última atualização:** Dezembro 2024
