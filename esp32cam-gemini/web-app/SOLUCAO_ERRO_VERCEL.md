# Solução para Erro de Espaços no Nome da Função Vercel

## Problema
```
Error: A Serverless Function has an invalid name: "'Esp32S-CAM com Gemini/web-app/___next_launcher.cjs'". 
They must be less than 128 characters long and must not contain any space.
```

## Causa
O caminho contém espaços ("Esp32S-CAM com Gemini"), o que não é permitido pela Vercel para funções serverless.

## Soluções

### Solução 1: Ajustar Root Directory na Vercel (Mais Rápida)

Na tela de configuração do projeto na Vercel:

1. **Root Directory:** Altere de:
   ```
   Esp32S-CAM com Gemini/web-app
   ```
   
   Para:
   ```
   web-app
   ```

2. **Framework Preset:** Certifique-se de estar como **"Next.js"**

3. Clique em **"Deploy"**

### Solução 2: Renomear Pasta do Projeto (Se Solução 1 não funcionar)

1. Renomeie a pasta do projeto para remover espaços:
   - De: `Esp32S-CAM com Gemini`
   - Para: `Esp32S-CAM-com-Gemini` ou `esp32cam-gemini`

2. Faça commit e push das mudanças

3. Na Vercel, configure:
   - **Root Directory:** `Esp32S-CAM-com-Gemini/web-app` (ou o novo nome)
   - **Framework Preset:** Next.js

### Solução 3: Mover web-app para Raiz do Repositório

Se o repositório permitir, mova a pasta `web-app` para a raiz do repositório:

1. Mova `web-app` para a raiz do repositório GitHub
2. Na Vercel, configure:
   - **Root Directory:** `web-app`
   - **Framework Preset:** Next.js

## Verificação

Após aplicar qualquer solução, verifique:
- ✅ Framework Preset = "Next.js"
- ✅ Root Directory não contém espaços
- ✅ Build Command = "npm run build" (automático para Next.js)

