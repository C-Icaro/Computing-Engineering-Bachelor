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
- [Troubleshooting Completo](#troubleshooting-completo)
- [Notas Importantes](#notas-importantes)
- [Notas de Segurança](#notas-de-segurança)
- [Estrutura do Projeto](#estrutura-do-projeto)
- [Desenvolvimento](#desenvolvimento)

## 🎯 Visão Geral

Este projeto consiste em:

1. **ESP32-CAM**: Captura imagens e envia para análise via Gemini AI
2. **Gemini AI**: Analisa as imagens e detecta presença de pessoas
3. **Web App (Next.js)**: Recebe e exibe as imagens capturadas em tempo real

## ✨ Funcionalidades

- ✅ Visualização da última imagem capturada
- ✅ Exibição da decisão do Gemini (pessoa detectada ou não)
- ✅ Exibição do status da bateria (tensão e porcentagem)
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
- API Key do Google Gemini

## 🚀 Instalação e Setup

### 1. Instalar Dependências

```bash
cd src/monitoramento de via/frontend
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
   - **Root Directory:** `src/monitoramento de via/frontend`
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
cd src/monitoramento de via/frontend
vercel
```

Siga as instruções no terminal.

## 🔧 Configuração da ESP32

### Passo a Passo Completo

1. **Após fazer deploy na Vercel, você receberá uma URL como:**
   ```
   https://seu-app-vercel.vercel.app
   ```

2. **Abra o arquivo:** `src/monitoramento de via/firmware/firmware.ino` no Arduino IDE

3. **Configurar Wi-Fi:**
   
   Localize e configure suas credenciais Wi-Fi (linha ~60):
   ```cpp
   const char* WIFI_SSID     = "SeuWiFi";
   const char* WIFI_PASSWORD = "SuaSenha";
   ```

4. **Configurar API Key do Gemini:**
   
   Localize e configure sua API Key (linha ~66):
   ```cpp
   const char* GEMINI_API_KEY = "SUA_API_KEY_AQUI";
   ```
   
   Para obter uma API Key:
   - Acesse [Google AI Studio](https://makersuite.google.com/app/apikey)
   - Crie uma nova API Key
   - Copie e cole no código

5. **Configurar URL do Web App:**
   
   Localize a seção de configuração da plataforma web (linha ~75):
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
   
   **Exemplo completo:**
   ```cpp
   // ==== Configurações da Plataforma Web (Vercel) ====
   // TODO: substitua pela URL da sua aplicação deployada na Vercel
   const char* WEB_APP_HOST = "meu-esp32-gemini.vercel.app";  // Sua URL aqui
   const char* WEB_APP_PATH = "/api/upload";
   const uint16_t WEB_APP_PORT = 443;  // HTTPS
   ```

6. **Compilar e Fazer Upload:**
   - Selecione a placa: **ESP32 Wrover Module** (ou similar)
   - Selecione a porta COM
   - Compile e faça upload

7. **Teste:**
   - Pressione o botão ou aguarde o PIR detectar movimento
   - A imagem será enviada automaticamente para a plataforma web
   - Acesse a URL da sua aplicação para ver a imagem

### Verificação

Após configurar, você verá no Serial Monitor (115200 baud):

```
Wi-Fi conectado. IP: 10.79.252.176
Enviando imagem para plataforma web...
Conectando a seu-app.vercel.app...
Enviando imagem para plataforma web...
Resposta da plataforma web:
HTTP/1.1 200 OK
...
Envio para plataforma web concluído.
```

## 📡 Como Funciona

1. **ESP32-CAM captura imagem** (via botão físico ou sensor PIR)
2. **Envia para Gemini AI** para análise e detecção de pessoas
3. **Envia imagem + decisão + dados de bateria** para a plataforma web via HTTPS
4. **Plataforma web armazena** a última imagem em arquivo temporário
5. **Interface web exibe** a última imagem capturada com auto-refresh

## 🌐 Endpoints da API

### `POST /api/upload`

Recebe imagem, decisão e dados de bateria da ESP32-CAM.

**Body (JSON):**
```json
{
  "image": "data:image/jpeg;base64,...",
  "decision": "person" | "no_person",
  "battery": {
    "voltage": 3.798,
    "percentage": 64
  }
}
```

**Resposta:**
```json
{
  "success": true,
  "filename": "capture-2024-01-01T12-00-00.jpg",
  "decision": "person",
  "imageUrl": "data:image/jpeg;base64,...",
  "timestamp": "2024-01-01T12:00:00.000Z",
  "battery": {
    "voltage": 3.798,
    "percentage": 64
  }
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
  "timestamp": "2024-01-01T12:00:00.000Z",
  "battery": {
    "voltage": 3.798,
    "percentage": 64
  }
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

## 🔍 Troubleshooting Completo

Este guia contém soluções detalhadas para problemas comuns encontrados durante o desenvolvimento e deploy.

### 🔴 Erro 401 Unauthorized

#### Sintoma

Serial Monitor mostra:
```
Resposta da plataforma web:
HTTP/1.1 401 Unauthorized
```

#### Causa

O projeto na Vercel está protegido por **Vercel Authentication** ou **Password Protection**, bloqueando requisições da ESP32.

#### Solução Passo a Passo

1. **Acesse o projeto na Vercel:**
   - Vá em https://vercel.com
   - Selecione seu projeto

2. **Vá em Settings → Deployment Protection:**
   - Procure por **"Vercel Authentication"**
   - OU **"Password Protection"**

3. **Desative a proteção:**
   - Clique no toggle para desativar
   - Salve as alterações

4. **Verifique:**
   - Acesse `https://seu-app.vercel.app/api/test` no navegador
   - Se pedir login/senha, a proteção ainda está ativa
   - Se abrir normalmente, está correto

5. **Teste novamente:**
   - Reenvie uma imagem da ESP32
   - O Serial Monitor deve mostrar `HTTP/1.1 200 OK`

#### Resultado Esperado

```
Resposta da plataforma web:
HTTP/1.1 200 OK
Content-Type: application/json
...
{"success":true,"filename":"...","decision":"no_person",...}
Envio para plataforma web concluído.
```

#### ⚠️ Nota de Segurança

Ao desabilitar a proteção, qualquer pessoa com a URL poderá acessar o projeto. Para ambiente produção o ideal é:
- Adicionar autenticação na própria API (token, API key)
- Usar rate limiting
- Restringir acesso por IP (se possível)

---

### 🔴 Erro 404 NOT_FOUND nas Rotas da API

#### Sintoma

Todas as rotas da API (`/api/test`, `/api/latest`, `/api/upload`) retornam 404 NOT_FOUND.

#### Causa

Configuração incorreta do Root Directory ou Framework Preset na Vercel, ou problema no build.

#### Solução Passo a Passo

**Passo 1: Verificar Configuração na Vercel**

1. Acesse o projeto na Vercel
2. Vá em **Settings → General**
3. Verifique e ajuste:
   - **Root Directory:** `src/monitoramento de via/frontend` (sem barra no final)
   - **Framework Preset:** `Next.js`
   - **Build Command:** (deixe vazio para detecção automática)
   - **Output Directory:** (deixe vazio para detecção automática)
   - **Install Command:** (deixe vazio para detecção automática)

**Passo 2: Verificar Logs de Build**

1. Vá em **Deployments**
2. Clique no **último deployment**
3. Vá na aba **"Build Logs"**
4. Procure por:

**✅ Sinais de Sucesso:**
- `"Compiled successfully"`
- `"Route (pages)"` ou `"Route (app)"`
- Lista de rotas sendo criadas

**❌ Sinais de Problema:**
- `"No Next.js detected"`
- `"Build failed"`
- `"Error: Cannot find module"`

**Passo 3: Fazer Redeploy**

1. Vá em **Deployments**
2. Clique nos **três pontos** (⋮) do último deployment
3. Selecione **"Redeploy"**
4. Aguarde o build completar

**Passo 4: Testar as Rotas**

Após o redeploy, teste:
- `https://seu-app.vercel.app/api/test` → deve retornar JSON
- `https://seu-app.vercel.app/api/latest` → deve retornar `null` ou dados
- `https://seu-app.vercel.app/api/upload` → deve retornar 405 para GET

#### Verificação Adicional

Se o problema persistir:

1. **Verifique a estrutura do repositório:**
   ```
   src/
     └── monitoramento de via/
         └── frontend/
             ├── package.json
             ├── next.config.js
             └── pages/
                 └── api/
                     ├── test.js
                     ├── latest.js
                     ├── upload.js
                     └── store.js
   ```

2. **Teste localmente:**
   ```bash
   cd src/monitoramento de via/frontend
   npm install
   npm run build
   npm start
   ```
   Acesse `http://localhost:3000/api/test` - deve retornar JSON.

---

### 🔴 Imagens Não Aparecem no Web App

#### Sintoma

`/api/latest` retorna `null`, mesmo após enviar imagens da ESP32.

#### Diagnóstico

**1. Verificar Serial Monitor da ESP32**

Abra o Serial Monitor (115200 baud) e procure por:

**✅ Mensagens de Sucesso Esperadas:**
```
Iniciando captura e envio ao Gemini...
Envio ao Gemini concluído.
Enviando imagem para plataforma web...
Conectando a seu-app.vercel.app...
Enviando imagem para plataforma web...
Resposta da plataforma web:
HTTP/1.1 200 OK
...
Envio para plataforma web concluído.
```

**❌ Mensagens de Erro Possíveis:**
- `"Falha ao conectar ao servidor web"` → Problema de conexão HTTPS
- `"HTTP/1.1 401 Unauthorized"` → Vercel Authentication ativo (veja solução acima)
- `"HTTP/1.1 404 NOT_FOUND"` → Rotas da API não encontradas (veja solução acima)
- `"Timeout aguardando resposta da plataforma web"` → Timeout na conexão
- `"Falha ao alocar memória para base64"` → Problema de memória
- `"Erro ao codificar base64"` → Problema na codificação

**2. Verificar Logs da Vercel**

1. Acesse o projeto na Vercel
2. Vá em **Deployments → Logs**
3. Procure por:
   - `"API /api/upload chamada:"` → Indica que a requisição chegou
   - `"Imagem processada com sucesso"` → Indica que foi processada
   - Erros de processamento

**3. Possíveis Problemas e Soluções**

**Problema 1: Erro 401 Unauthorized**
- **Solução:** Desabilite Vercel Authentication (veja seção acima)

**Problema 2: Erro de Conexão HTTPS**
- **Solução:** Verifique se a URL está correta (sem https:// e sem /)
- Verifique se a porta está como `443`
- Verifique a conexão Wi-Fi da ESP32

**Problema 3: Payload muito grande**
- **Solução:** 
  - Verifique o tamanho da imagem no Serial Monitor
  - Considere reduzir a qualidade da imagem no código da ESP32
  - O limite atual é 10MB (configurável em `pages/api/upload.js`)

**Problema 4: Timeout na conexão**
- **Solução:** 
  - Aumente o timeout no código da ESP32
  - Verifique a conexão Wi-Fi
  - Verifique se a Vercel está online

**Problema 5: Estado não compartilhado (Serverless)**
- **Causa:** Em ambiente serverless, o estado em memória pode não ser compartilhado entre instâncias
- **Solução:** Para produção, use storage externo (banco de dados, S3, etc.)

#### Teste Manual

Para testar se a API está funcionando:

```bash
curl -X POST https://seu-app.vercel.app/api/upload \
  -H "Content-Type: application/json" \
  -d '{
    "image": "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEAYABgAAD...",
    "decision": "person"
  }'
```

Se funcionar, o problema está na ESP32. Se não funcionar, o problema está na API.

---

### 🔴 Erro de Conexão HTTPS

#### Sintoma

Serial Monitor mostra: `"Falha ao conectar ao servidor web"`

#### Causas Possíveis

1. URL incorreta
2. Porta incorreta
3. Problema de Wi-Fi
4. Problema com certificado SSL

#### Soluções

**1. Verificar URL**

Certifique-se de que:
- ✅ `WEB_APP_HOST` está correto (sem https:// e sem /)
- ✅ `WEB_APP_PATH` está como `/api/upload`
- ✅ `WEB_APP_PORT` está como `443`

**Exemplo correto:**
```cpp
const char* WEB_APP_HOST = "monitoramento-de-91k7y08uy-carlosicaro13-gmailcoms-projects.vercel.app";
```

**Exemplo incorreto:**
```cpp
const char* WEB_APP_HOST = "https://monitoramento-de-...vercel.app/";
```

**2. Verificar Conexão Wi-Fi**

- Verifique se a ESP32 está conectada ao Wi-Fi
- Verifique se o Serial Monitor mostra: `"Wi-Fi conectado. IP: ..."`
- Teste a conectividade da ESP32

**3. Verificar Certificado SSL**

O código já usa `client.setInsecure()` para ignorar verificação de certificado. Se ainda houver problemas:
- Verifique se há atualizações do firmware da ESP32
- Verifique se há problemas conhecidos com certificados SSL

---

### 🔴 Build Falha na Vercel

#### Sintoma

Build não completa ou falha com erros.

#### Soluções

1. **Verifique os Build Logs:**
   - Vá em **Deployments → Build Logs**
   - Procure por erros específicos
   - Erros comuns:
     - `"Cannot find module"` → Dependências faltando
     - `"Build failed"` → Erro de compilação
     - `"No Next.js detected"` → Root Directory incorreto

2. **Verifique Dependências:**
   - Certifique-se de que `package.json` está correto
   - Verifique se todas as dependências estão listadas

3. **Verifique Estrutura:**
   - Certifique-se de que todos os arquivos foram commitados
   - Verifique se o Root Directory está correto

4. **Tente Redeploy:**
   - Às vezes, um redeploy resolve problemas temporários

---

### 🔴 Warning: Invalid next.config.js

#### Sintoma

Warning sobre configuração inválida:
```
⚠ Invalid next.config.js options detected: 
⚠     Unrecognized key(s) in object: 'api'
```

#### Causa

Configuração obsoleta `api` no Next.js 14. A configuração `api` foi movida para dentro das rotas da API.

#### Solução

Já corrigido no código. Se aparecer, verifique se o `next.config.js` não contém:

```javascript
// ❌ INCORRETO (Next.js 14)
const nextConfig = {
  api: {
    bodyParser: {
      sizeLimit: '10mb',
    },
  },
}
```

A configuração correta está em `pages/api/upload.js`:

```javascript
// ✅ CORRETO
export const config = {
  api: {
    bodyParser: {
      sizeLimit: '10mb',
    },
  },
}
```

---

### 🔴 Erro de Espaços no Nome da Função Vercel

#### Sintoma

```
Error: A Serverless Function has an invalid name: "'Esp32S-CAM com Gemini/web-app/___next_launcher.cjs'". 
They must be less than 128 characters long and must not contain any space.
```

#### Causa

O caminho contém espaços, o que não é permitido pela Vercel para funções serverless.

#### Soluções

**Solução 1: Ajustar Root Directory (Mais Rápida)**

1. Na Vercel, vá em **Settings → General**
2. Altere o **Root Directory** para:
   ```
   src/monitoramento de via/frontend
   ```

3. Certifique-se de que **Framework Preset** está como **"Next.js"**
4. Faça redeploy

**Solução 2: Renomear Pasta do Projeto**

1. Se necessário, renomeie a pasta do projeto para remover espaços

2. Faça commit e push das mudanças

3. Na Vercel, configure:
   - **Root Directory:** `src/monitoramento de via/frontend`
   - **Framework Preset:** Next.js

---

### 📞 Ainda com Problemas?

Se nenhuma das soluções acima resolver seu problema:

1. **Verifique os logs:**
   - Serial Monitor da ESP32
   - Logs da Vercel (Deployments → Logs)
   - Build Logs da Vercel

2. **Teste localmente:**
   - Execute `npm run dev` e teste localmente
   - Isso ajuda a identificar se o problema é no código ou na configuração da Vercel

3. **Verifique a documentação:**
   - Documentação do Next.js: https://nextjs.org/docs
   - Documentação da Vercel: https://vercel.com/docs
   - Documentação do ESP32: https://docs.espressif.com/

## 📝 Notas Importantes

### Armazenamento de Imagens

- As imagens são armazenadas **em arquivo temporário** (no Vercel: `/tmp`, localmente: diretório temporário do sistema)
- Em ambiente serverless, o estado pode não ser compartilhado entre instâncias diferentes
- Para produção, o ideal é:
  - Banco de dados (PostgreSQL, MongoDB)
  - Storage (AWS S3, Cloudinary, Vercel Blob)
  - Sistema de arquivos persistente

### Limites

- A aplicação aceita imagens até **10MB**
- O limite pode ser ajustado em `pages/api/upload.js` (constante `BODY_SIZE_LIMIT`)
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

## 📚 Estrutura do frontend

```
src/monitoramento de via/frontend/
├── pages/
│   ├── api/
│   │   ├── upload.js      # Recebe imagens da ESP32
│   │   ├── latest.js       # Retorna última imagem
│   │   ├── test.js         # Rota de teste
│   │   └── store.js        # Armazenamento persistente
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

### Como Usar Esta Documentação

**Para Setup Inicial:**
1. Comece com a seção "Instalação e Setup"
2. Siga as seções na ordem:
   - Deploy na Vercel
   - Configuração da ESP32
   - Como Funciona

**Para Resolver Problemas:**
1. Consulte a seção "Troubleshooting Completo"
2. Use o índice para encontrar o problema específico
3. Siga as soluções passo a passo


