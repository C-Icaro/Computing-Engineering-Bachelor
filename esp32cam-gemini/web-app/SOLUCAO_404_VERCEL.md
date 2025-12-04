# Solução para 404 nas Rotas da API na Vercel

## ✅ Confirmação
O código funciona localmente (`http://localhost:3000/api/test` retorna JSON), então o problema está na **configuração do deploy na Vercel**.

## 🔧 Solução Passo a Passo

### Passo 1: Verificar Configuração do Projeto na Vercel

1. Acesse seu projeto na Vercel
2. Vá em **Settings → General**
3. Verifique e ajuste as seguintes configurações:

#### Root Directory
- **Deve estar:** `esp32cam-gemini/web-app`
- **NÃO deve ter:** barra no final (`/`)
- **NÃO deve ter:** espaços extras

#### Framework Preset
- **Deve estar:** `Next.js`
- Se estiver como "Other" ou "Vite", mude para "Next.js"

#### Build Command
- **Deixe vazio** (para detecção automática)
- OU configure como: `npm run build`

#### Output Directory
- **Deixe vazio** (para detecção automática)
- OU configure como: `.next`

#### Install Command
- **Deixe vazio** (para detecção automática)
- OU configure como: `npm install`

### Passo 2: Verificar Logs de Build

1. Vá em **Deployments**
2. Clique no **último deployment**
3. Vá na aba **"Build Logs"**
4. Procure por:

#### ✅ Sinais de Sucesso:
- `"Compiled successfully"`
- `"Route (pages)"` ou `"Route (app)"`
- Lista de rotas sendo criadas, incluindo `/api/test`, `/api/latest`, `/api/upload`

#### ❌ Sinais de Problema:
- `"No Next.js detected"`
- `"Build failed"`
- `"Error: Cannot find module"`
- `"404"` ou `"NOT_FOUND"` nos logs

### Passo 3: Fazer Redeploy

1. Vá em **Deployments**
2. Clique nos **três pontos** (⋮) do último deployment
3. Selecione **"Redeploy"**
4. Aguarde o build completar
5. Verifique os logs novamente

### Passo 4: Testar as Rotas

Após o redeploy, teste:
- `https://seu-app.vercel.app/api/test` → deve retornar JSON
- `https://seu-app.vercel.app/api/latest` → deve retornar `null` ou dados da imagem
- `https://seu-app.vercel.app/api/upload` → deve retornar 405 (Method Not Allowed) para GET

## 🔍 Diagnóstico Adicional

### Se o problema persistir:

1. **Verifique se o repositório está sincronizado:**
   - Certifique-se de que fez commit e push de todos os arquivos
   - Verifique se `pages/api/test.js` existe no repositório

2. **Verifique a estrutura no repositório:**
   ```
   esp32cam-gemini/
     └── web-app/
         ├── package.json
         ├── next.config.js
         └── pages/
             └── api/
                 ├── test.js
                 ├── latest.js
                 ├── upload.js
                 └── store.js
   ```

3. **Tente criar um novo projeto na Vercel:**
   - Às vezes, configurações antigas podem causar problemas
   - Crie um novo projeto e importe o mesmo repositório
   - Configure o Root Directory como `esp32cam-gemini/web-app`

## 📝 Checklist Final

Antes de testar novamente, verifique:

- [ ] Root Directory: `esp32cam-gemini/web-app` (sem barra no final)
- [ ] Framework Preset: `Next.js`
- [ ] Build Command: vazio ou `npm run build`
- [ ] Output Directory: vazio ou `.next`
- [ ] Todos os arquivos foram commitados e enviados ao repositório
- [ ] Build Logs mostram "Compiled successfully"
- [ ] Build Logs mostram as rotas da API sendo criadas

## 🎯 Resultado Esperado

Após seguir estes passos:
- ✅ `/api/test` deve retornar: `{"message":"API route is working!","timestamp":"..."}`
- ✅ `/api/latest` deve retornar: `null` (se não houver imagem) ou dados da imagem
- ✅ `/api/upload` deve aceitar POST com imagem


