# Diagnóstico de Problema 404 nas Rotas da API

## Problema
Todas as rotas da API (`/api/test`, `/api/latest`, `/api/upload`) estão retornando 404 NOT_FOUND.

## Possíveis Causas

### 1. Root Directory Incorreto
Se o Root Directory na Vercel está configurado como `esp32cam-gemini/web-app`, a Vercel deve encontrar:
- ✅ `package.json` dentro de `esp32cam-gemini/web-app/`
- ✅ `next.config.js` dentro de `esp32cam-gemini/web-app/`
- ✅ `pages/api/` dentro de `esp32cam-gemini/web-app/`

**Verificar:**
- Vá em Settings → General → Root Directory
- Deve estar exatamente como: `esp32cam-gemini/web-app` (sem barra no final)

### 2. Framework Preset Incorreto
**Verificar:**
- Settings → General → Framework Preset
- Deve estar como **"Next.js"** (não "Other" ou "Vite")

### 3. Build Command
**Verificar:**
- Settings → General → Build Command
- Deve estar como: `npm run build` (ou vazio para detecção automática)

### 4. Output Directory
**Verificar:**
- Settings → General → Output Directory
- Deve estar como: `.next` (ou vazio para detecção automática)

### 5. Verificar Logs de Build
1. Vá em Deployments
2. Clique no último deployment
3. Vá na aba "Build Logs"
4. Verifique se há erros durante o build
5. Procure por mensagens como:
   - "No Next.js detected"
   - "Build failed"
   - "Error: Cannot find module"

## Solução Recomendada

### Passo 1: Verificar Configuração na Vercel
1. Acesse o projeto na Vercel
2. Vá em **Settings → General**
3. Verifique:
   - **Root Directory:** `esp32cam-gemini/web-app`
   - **Framework Preset:** `Next.js`
   - **Build Command:** (deixe vazio ou `npm run build`)
   - **Output Directory:** (deixe vazio ou `.next`)
   - **Install Command:** (deixe vazio ou `npm install`)

### Passo 2: Verificar Estrutura do Repositório
Certifique-se de que a estrutura está assim:
```
esp32cam-gemini/
  └── web-app/
      ├── package.json
      ├── next.config.js
      ├── pages/
      │   ├── api/
      │   │   ├── test.js
      │   │   ├── latest.js
      │   │   ├── upload.js
      │   │   └── store.js
      │   └── index.js
      └── vercel.json
```

### Passo 3: Fazer Redeploy
1. Vá em Deployments
2. Clique nos três pontos do último deployment
3. Selecione "Redeploy"
4. Aguarde o build completar
5. Verifique os logs

### Passo 4: Testar Localmente
Para garantir que o código funciona:
```bash
cd web-app
npm install
npm run build
npm start
```
Depois acesse `http://localhost:3000/api/test` - deve retornar JSON.

## Se o Problema Persistir

### Alternativa: Mudar Root Directory para `web-app`
Se o problema persistir, tente:
1. Na Vercel, mude o Root Directory para apenas: `web-app`
2. Mas isso só funciona se o repositório tiver a estrutura:
   ```
   esp32cam-gemini/
     └── web-app/
   ```
   E você configurar o Root Directory como `web-app` (relativo à raiz do repositório)

### Verificar se o Build Está Funcionando
1. Vá em Deployments → Build Logs
2. Procure por:
   - "Compiled successfully"
   - "Route (app)" ou "Route (pages)"
   - Se não aparecer, o build pode estar falhando


