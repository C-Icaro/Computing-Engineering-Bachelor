# ESP32-CAM + Gemini - Visualizador Web

Aplicação web para visualizar imagens capturadas pela ESP32-CAM com detecção de pessoas via Gemini AI.

## 🚀 Deploy na Vercel

### Pré-requisitos
- Conta no [Vercel](https://vercel.com)
- Node.js instalado (para desenvolvimento local)

### Passo a passo

1. **Instalar dependências:**
   ```bash
   cd web-app
   npm install
   ```

2. **Testar localmente (opcional):**
   ```bash
   npm run dev
   ```
   Acesse `http://localhost:3000`

3. **Fazer deploy na Vercel:**
   
   **Opção A - Via CLI:**
   ```bash
   npm i -g vercel
   vercel
   ```
   
   **Opção B - Via GitHub:**
   - Faça push do código para um repositório GitHub
   - Acesse [vercel.com](https://vercel.com)
   - Clique em "New Project"
   - Importe o repositório
   - Configure:
     - Framework Preset: Next.js
     - Root Directory: `web-app`
   - Clique em "Deploy"

4. **Anotar a URL:**
   - Após o deploy, você receberá uma URL como: `https://seu-app.vercel.app`
   - **IMPORTANTE:** Anote essa URL para configurar na ESP32

## 🔧 Configuração da ESP32-CAM

1. **Abrir o arquivo `DroneBotWorkshopCode.ino`**

2. **Localizar a seção de configuração da plataforma web:**
   ```cpp
   // ==== Configurações da Plataforma Web (Vercel) ====
   const char* WEB_APP_HOST = "seu-app.vercel.app";  // Substitua pela sua URL
   ```

3. **Substituir pela URL da sua aplicação:**
   ```cpp
   const char* WEB_APP_HOST = "seu-app-vercel.vercel.app";  // Sua URL sem https://
   ```

4. **Compilar e fazer upload para a ESP32-CAM**

## 📡 Como Funciona

1. **ESP32-CAM captura imagem** (via botão ou PIR)
2. **Envia para Gemini AI** para detecção de pessoas
3. **Envia imagem + decisão** para a plataforma web
4. **Plataforma web exibe** a última imagem capturada

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

## 🎨 Funcionalidades

- ✅ Visualização da última imagem capturada
- ✅ Exibição da decisão do Gemini (pessoa detectada ou não)
- ✅ Auto-atualização a cada 5 segundos
- ✅ Atualização manual
- ✅ Interface responsiva
- ✅ Timestamp da captura

## 📝 Notas

- As imagens são armazenadas em memória (não persistem após reiniciar)
- Para produção, considere usar um banco de dados ou storage (S3, Cloudinary, etc.)
- A aplicação aceita imagens até 10MB

## 🔒 Segurança

Para produção, considere adicionar:
- Autenticação na API
- Rate limiting
- Validação de origem (CORS)
- Armazenamento seguro de imagens

