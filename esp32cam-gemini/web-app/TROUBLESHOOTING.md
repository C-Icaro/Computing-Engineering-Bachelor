# Guia Completo de Troubleshooting

Este documento contém soluções detalhadas para problemas comuns encontrados durante o desenvolvimento e deploy deste projeto.

## 📋 Índice

- [Erro 401 Unauthorized](#erro-401-unauthorized)
- [Erro 404 NOT_FOUND nas Rotas da API](#erro-404-not_found-nas-rotas-da-api)
- [Imagens Não Aparecem no Web App](#imagens-não-aparecem-no-web-app)
- [Erro de Conexão HTTPS](#erro-de-conexão-https)
- [Build Falha na Vercel](#build-falha-na-vercel)
- [Warning: Invalid next.config.js](#warning-invalid-nextconfigjs)
- [Erro de Espaços no Nome da Função Vercel](#erro-de-espaços-no-nome-da-função-vercel)

## 🔴 Erro 401 Unauthorized

### Sintoma

Serial Monitor mostra:
```
Resposta da plataforma web:
HTTP/1.1 401 Unauthorized
```

### Causa

O projeto na Vercel está protegido por **Vercel Authentication** ou **Password Protection**, bloqueando requisições da ESP32.

### Solução Passo a Passo

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

### Resultado Esperado

```
Resposta da plataforma web:
HTTP/1.1 200 OK
Content-Type: application/json
...
{"success":true,"filename":"...","decision":"no_person",...}
Envio para plataforma web concluído.
```

### ⚠️ Nota de Segurança

Ao desabilitar a proteção, qualquer pessoa com a URL poderá acessar seu projeto. Para produção, considere:
- Adicionar autenticação na própria API (token, API key)
- Usar rate limiting
- Restringir acesso por IP (se possível)

---

## 🔴 Erro 404 NOT_FOUND nas Rotas da API

### Sintoma

Todas as rotas da API (`/api/test`, `/api/latest`, `/api/upload`) retornam 404 NOT_FOUND.

### Causa

Configuração incorreta do Root Directory ou Framework Preset na Vercel, ou problema no build.

### Solução Passo a Passo

#### Passo 1: Verificar Configuração na Vercel

1. Acesse o projeto na Vercel
2. Vá em **Settings → General**
3. Verifique e ajuste:
   - **Root Directory:** `esp32cam-gemini/web-app` (sem barra no final)
   - **Framework Preset:** `Next.js`
   - **Build Command:** (deixe vazio para detecção automática)
   - **Output Directory:** (deixe vazio para detecção automática)
   - **Install Command:** (deixe vazio para detecção automática)

#### Passo 2: Verificar Logs de Build

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

#### Passo 3: Fazer Redeploy

1. Vá em **Deployments**
2. Clique nos **três pontos** (⋮) do último deployment
3. Selecione **"Redeploy"**
4. Aguarde o build completar

#### Passo 4: Testar as Rotas

Após o redeploy, teste:
- `https://seu-app.vercel.app/api/test` → deve retornar JSON
- `https://seu-app.vercel.app/api/latest` → deve retornar `null` ou dados
- `https://seu-app.vercel.app/api/upload` → deve retornar 405 para GET

### Verificação Adicional

Se o problema persistir:

1. **Verifique a estrutura do repositório:**
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

2. **Teste localmente:**
   ```bash
   cd web-app
   npm install
   npm run build
   npm start
   ```
   Acesse `http://localhost:3000/api/test` - deve retornar JSON.

---

## 🔴 Imagens Não Aparecem no Web App

### Sintoma

`/api/latest` retorna `null`, mesmo após enviar imagens da ESP32.

### Diagnóstico

#### 1. Verificar Serial Monitor da ESP32

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

#### 2. Verificar Logs da Vercel

1. Acesse o projeto na Vercel
2. Vá em **Deployments → Logs**
3. Procure por:
   - `"API /api/upload chamada:"` → Indica que a requisição chegou
   - `"Imagem processada com sucesso"` → Indica que foi processada
   - Erros de processamento

#### 3. Possíveis Problemas e Soluções

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

### Teste Manual

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

## 🔴 Erro de Conexão HTTPS

### Sintoma

Serial Monitor mostra: `"Falha ao conectar ao servidor web"`

### Causas Possíveis

1. URL incorreta
2. Porta incorreta
3. Problema de Wi-Fi
4. Problema com certificado SSL

### Soluções

#### 1. Verificar URL

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

#### 2. Verificar Conexão Wi-Fi

- Verifique se a ESP32 está conectada ao Wi-Fi
- Verifique se o Serial Monitor mostra: `"Wi-Fi conectado. IP: ..."`
- Teste a conectividade da ESP32

#### 3. Verificar Certificado SSL

O código já usa `client.setInsecure()` para ignorar verificação de certificado. Se ainda houver problemas:
- Verifique se há atualizações do firmware da ESP32
- Verifique se há problemas conhecidos com certificados SSL

---

## 🔴 Build Falha na Vercel

### Sintoma

Build não completa ou falha com erros.

### Soluções

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

## 🔴 Warning: Invalid next.config.js

### Sintoma

Warning sobre configuração inválida:
```
⚠ Invalid next.config.js options detected: 
⚠     Unrecognized key(s) in object: 'api'
```

### Causa

Configuração obsoleta `api` no Next.js 14. A configuração `api` foi movida para dentro das rotas da API.

### Solução

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

## 🔴 Erro de Espaços no Nome da Função Vercel

### Sintoma

```
Error: A Serverless Function has an invalid name: "'Esp32S-CAM com Gemini/web-app/___next_launcher.cjs'". 
They must be less than 128 characters long and must not contain any space.
```

### Causa

O caminho contém espaços, o que não é permitido pela Vercel para funções serverless.

### Soluções

#### Solução 1: Ajustar Root Directory (Mais Rápida)

1. Na Vercel, vá em **Settings → General**
2. Altere o **Root Directory** de:
   ```
   Esp32S-CAM com Gemini/web-app
   ```
   Para:
   ```
   web-app
   ```
   (ou `esp32cam-gemini/web-app` se a pasta foi renomeada)

3. Certifique-se de que **Framework Preset** está como **"Next.js"**
4. Faça redeploy

#### Solução 2: Renomear Pasta do Projeto

1. Renomeie a pasta do projeto para remover espaços:
   - De: `Esp32S-CAM com Gemini`
   - Para: `esp32cam-gemini` (ou similar)

2. Faça commit e push das mudanças

3. Na Vercel, configure:
   - **Root Directory:** `esp32cam-gemini/web-app`
   - **Framework Preset:** Next.js

---

## 📞 Ainda com Problemas?

Se nenhuma das soluções acima resolver seu problema:

1. **Verifique os logs:**
   - Serial Monitor da ESP32
   - Logs da Vercel (Deployments → Logs)
   - Build Logs da Vercel

2. **Teste localmente:**
   - Execute `npm run dev` e teste localmente
   - Isso ajuda a identificar se o problema é no código ou na configuração da Vercel

3. **Verifique a documentação:**
   - README.md principal
   - Documentação do Next.js
   - Documentação da Vercel

---

**Última atualização:** Dezembro 2024


