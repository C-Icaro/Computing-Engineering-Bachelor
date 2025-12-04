# Debug - ESP32 não está enviando imagens

## Problema
A ESP32 está tirando fotos, mas as imagens não aparecem no web app.

## Checklist de Diagnóstico

### 1. Verificar Serial Monitor da ESP32

Abra o Serial Monitor (115200 baud) e procure por estas mensagens:

#### ✅ Mensagens de Sucesso Esperadas:
```
Iniciando captura e envio ao Gemini...
Envio ao Gemini concluído.
Enviando imagem para plataforma web...
Conectando a monitoramento-de-91k7y08uy-carlosicaro13-gmailcoms-projects.vercel.app...
Enviando imagem para plataforma web...
Resposta da plataforma web:
HTTP/1.1 200 OK
...
Envio para plataforma web concluído.
```

#### ❌ Mensagens de Erro Possíveis:
- `"Falha ao conectar ao servidor web"` → Problema de conexão HTTPS
- `"Falha no envio ao Gemini."` → Se falhar no Gemini, não envia para web app
- `"Timeout aguardando resposta da plataforma web"` → Timeout na conexão
- `"Falha ao alocar memória para base64"` → Problema de memória
- `"Erro ao codificar base64"` → Problema na codificação

### 2. Verificar Logs da Vercel

1. Acesse o projeto na Vercel
2. Vá em **Deployments → Logs**
3. Procure por:
   - `"API /api/upload chamada:"` → Indica que a requisição chegou
   - `"Imagem processada com sucesso"` → Indica que foi processada
   - Erros de processamento

### 3. Possíveis Problemas e Soluções

#### Problema 1: ESP32 só envia se Gemini funcionar
**Causa:** O código só envia para o web app se o envio ao Gemini for bem-sucedido (linha 484-490)

**Solução:** Verifique se o Gemini está funcionando:
- Verifique a API key do Gemini
- Veja se há erros no Serial Monitor relacionados ao Gemini

#### Problema 2: Erro de Conexão HTTPS
**Causa:** A ESP32 pode ter problemas com certificados SSL

**Solução:** O código já usa `client.setInsecure()`, mas pode haver problemas de memória ou timeout

#### Problema 3: Payload muito grande
**Causa:** Imagens muito grandes podem causar timeout

**Solução:** 
- Verifique o tamanho da imagem no Serial Monitor
- Considere reduzir a qualidade da imagem (já está em `jpeg_quality = 12`)

#### Problema 4: Timeout na conexão
**Causa:** A conexão pode estar demorando muito

**Solução:** Aumente o timeout ou verifique a conexão Wi-Fi

### 4. Teste Manual

Para testar se a API está funcionando, você pode enviar uma requisição manual usando curl ou Postman:

```bash
curl -X POST https://monitoramento-de-91k7y08uy-carlosicaro13-gmailcoms-projects.vercel.app/api/upload \
  -H "Content-Type: application/json" \
  -d '{
    "image": "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEAYABgAAD...",
    "decision": "person"
  }'
```

Se funcionar, o problema está na ESP32. Se não funcionar, o problema está na API.

### 5. Verificar Configuração da ESP32

Certifique-se de que:
- ✅ `WEB_APP_HOST` está correto (sem https:// e sem /)
- ✅ `WEB_APP_PATH` está como `/api/upload`
- ✅ `WEB_APP_PORT` está como `443`
- ✅ Wi-Fi está conectado
- ✅ Câmera está funcionando

### 6. Adicionar Mais Logs (Opcional)

Se necessário, adicione mais logs no código da ESP32 para debug:
- Log do tamanho do payload
- Log do status da conexão
- Log da resposta completa do servidor

## Próximos Passos

1. **Verifique o Serial Monitor** e me informe quais mensagens aparecem
2. **Verifique os Logs da Vercel** e me informe se há requisições chegando
3. **Teste manual** da API para confirmar que está funcionando

Com essas informações, poderemos identificar exatamente onde está o problema.


