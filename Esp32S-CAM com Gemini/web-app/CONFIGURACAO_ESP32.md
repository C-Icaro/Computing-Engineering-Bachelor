# Configuração da ESP32-CAM para Enviar Imagens

## Passo a Passo

1. **Após fazer deploy na Vercel, você receberá uma URL como:**
   ```
   https://seu-app-vercel.vercel.app
   ```

2. **Abra o arquivo:** `DroneBotWorkshopCode/DroneBotWorkshopCode.ino`

3. **Localize a linha 58-59:**
   ```cpp
   const char* WEB_APP_HOST = "seu-app.vercel.app";  // Sem https://, será adicionado na função
   ```

4. **Substitua pela sua URL (sem https://):**
   ```cpp
   const char* WEB_APP_HOST = "seu-app-vercel.vercel.app";
   ```

5. **Exemplo completo:**
   ```cpp
   // ==== Configurações da Plataforma Web (Vercel) ====
   // TODO: substitua pela URL da sua aplicação deployada na Vercel
   // Exemplo: "https://seu-app.vercel.app"
   const char* WEB_APP_HOST = "meu-esp32-gemini.vercel.app";  // Sua URL aqui
   const char* WEB_APP_PATH = "/api/upload";
   const uint16_t WEB_APP_PORT = 443;  // HTTPS
   ```

6. **Compile e faça upload para a ESP32-CAM**

7. **Teste:**
   - Pressione o botão ou aguarde o PIR detectar movimento
   - A imagem será enviada automaticamente para a plataforma web
   - Acesse a URL da sua aplicação para ver a imagem

## Verificação

Após configurar, você verá no Serial Monitor:
```
Enviando imagem para plataforma web...
Conectando a seu-app.vercel.app...
Enviando imagem para plataforma web...
Resposta da plataforma web:
HTTP/1.1 200 OK
...
Envio para plataforma web concluído.
```

## Troubleshooting

- **Erro de conexão:** Verifique se a URL está correta (sem https://)
- **Timeout:** Verifique se a aplicação está online na Vercel
- **Erro 400:** Verifique se o formato da imagem está correto

