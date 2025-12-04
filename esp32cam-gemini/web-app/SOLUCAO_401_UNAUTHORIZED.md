# Solução para Erro 401 Unauthorized na Vercel

## 🔴 Problema Identificado

O Serial Monitor mostra:
```
Resposta da plataforma web:
HTTP/1.1 401 Unauthorized
```

Isso significa que a Vercel está bloqueando as requisições da ESP32 porque o projeto está protegido por **Password Protection** ou **Vercel Authentication**.

## ✅ Solução: Desabilitar Password Protection

### Passo 1: Acessar Configurações do Projeto

1. Acesse o projeto na Vercel: https://vercel.com
2. Vá em **Settings** (Configurações)
3. Clique em **Deployment Protection** ou **Security**

### Passo 2: Desabilitar Password Protection

1. Procure por **"Password Protection"** ou **"Deployment Protection"**
2. Se estiver **ativado**, desative:
   - Clique no toggle para desativar
   - OU remova a senha configurada
   - Salve as alterações

### Passo 3: Verificar Vercel Authentication

1. Ainda em **Settings**
2. Procure por **"Vercel Authentication"** ou **"Team Access"**
3. Se houver autenticação obrigatória, você pode:
   - **Opção A:** Desabilitar a autenticação (se possível)
   - **Opção B:** Criar um token de acesso e usar na ESP32 (mais complexo)

### Passo 4: Verificar Domain Protection

1. Em **Settings → Domains**
2. Verifique se há alguma proteção no domínio
3. Se houver, desative temporariamente para testar

## 🔧 Alternativa: Usar URL de Produção

Se você não conseguir desabilitar a proteção, você pode:

1. **Promover um deployment para produção:**
   - Vá em **Deployments**
   - Clique nos três pontos do deployment
   - Selecione **"Promote to Production"**

2. **Usar o domínio de produção** (se configurado):
   - O domínio de produção geralmente não tem proteção por senha
   - Configure a ESP32 para usar esse domínio

## 📝 Verificação

Após desabilitar a proteção:

1. **Teste manualmente:**
   ```bash
   curl -X POST https://monitoramento-de-91k7y08uy-carlosicaro13-gmailcoms-projects.vercel.app/api/upload \
     -H "Content-Type: application/json" \
     -d '{"image":"data:image/jpeg;base64,/9j/4AAQ...","decision":"person"}'
   ```
   
   Deve retornar `200 OK` (não `401 Unauthorized`)

2. **Teste com a ESP32:**
   - Reenvie uma imagem
   - Verifique o Serial Monitor
   - Deve aparecer `HTTP/1.1 200 OK` em vez de `401 Unauthorized`

## 🎯 Resultado Esperado

Após desabilitar a proteção, o Serial Monitor deve mostrar:
```
Resposta da plataforma web:
HTTP/1.1 200 OK
Content-Type: application/json
...
{"success":true,"filename":"...","decision":"no_person",...}
Envio para plataforma web concluído.
```

E as imagens devem começar a aparecer no web app!

## ⚠️ Nota de Segurança

Se você desabilitar a proteção por senha, qualquer pessoa com a URL poderá acessar seu projeto. Para produção, considere:
- Adicionar autenticação na própria API (token, API key)
- Usar rate limiting
- Restringir acesso por IP (se possível)


