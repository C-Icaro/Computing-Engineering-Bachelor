# Índice de Documentação

Este documento organiza toda a documentação do projeto ESP32-CAM + Gemini.

## 📚 Documentação Principal

### [README.md](./README.md)
Documentação principal do projeto com:
- Visão geral
- Instalação e setup
- Deploy na Vercel
- Configuração da ESP32
- Endpoints da API
- Troubleshooting básico

### [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)
Guia completo de troubleshooting com soluções detalhadas para:
- Erro 401 Unauthorized
- Erro 404 NOT_FOUND
- Imagens não aparecem
- Erros de conexão
- Problemas de build

## 📖 Documentação de Referência

Os seguintes documentos foram consolidados no README.md e TROUBLESHOOTING.md, mas são mantidos para referência:

### [CONFIGURACAO_ESP32.md](./CONFIGURACAO_ESP32.md)
Guia específico para configuração da ESP32-CAM.

### [SOLUCAO_401_UNAUTHORIZED.md](./SOLUCAO_401_UNAUTHORIZED.md)
Solução detalhada para o erro 401 Unauthorized (consolidado em TROUBLESHOOTING.md).

### [SOLUCAO_404_VERCEL.md](./SOLUCAO_404_VERCEL.md)
Solução detalhada para erros 404 nas rotas da API (consolidado em TROUBLESHOOTING.md).

### [DEBUG_ESP32.md](./DEBUG_ESP32.md)
Guia de debug para problemas com envio de imagens da ESP32 (consolidado em TROUBLESHOOTING.md).

### [DIAGNOSTICO_VERCEL.md](./DIAGNOSTICO_VERCEL.md)
Diagnóstico de problemas na Vercel (consolidado em TROUBLESHOOTING.md).

### [SOLUCAO_ERRO_VERCEL.md](./SOLUCAO_ERRO_VERCEL.md)
Solução para erro de espaços no nome da função (consolidado em TROUBLESHOOTING.md).

## 🚀 Início Rápido

Para começar rapidamente:

1. **Leia o [README.md](./README.md)** para visão geral e setup
2. **Siga as instruções de deploy** na seção "Deploy na Vercel"
3. **Configure a ESP32** seguindo "Configuração da ESP32"
4. **Se tiver problemas**, consulte [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)

## 📝 Estrutura da Documentação

```
web-app/
├── README.md                    # Documentação principal
├── TROUBLESHOOTING.md           # Guia completo de troubleshooting
├── DOCUMENTACAO.md              # Este arquivo (índice)
│
├── CONFIGURACAO_ESP32.md        # Referência: Configuração ESP32
├── SOLUCAO_401_UNAUTHORIZED.md  # Referência: Erro 401
├── SOLUCAO_404_VERCEL.md        # Referência: Erro 404
├── DEBUG_ESP32.md               # Referência: Debug ESP32
├── DIAGNOSTICO_VERCEL.md        # Referência: Diagnóstico Vercel
└── SOLUCAO_ERRO_VERCEL.md       # Referência: Erro espaços
```

## 🔍 Como Usar Esta Documentação

### Para Setup Inicial
1. Comece com [README.md](./README.md)
2. Siga as seções na ordem:
   - Instalação e Setup
   - Deploy na Vercel
   - Configuração da ESP32

### Para Resolver Problemas
1. Consulte [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)
2. Use o índice para encontrar o problema específico
3. Siga as soluções passo a passo

### Para Referência Detalhada
- Use os arquivos de referência para informações mais específicas
- Eles contêm detalhes adicionais que podem ser úteis

## 📞 Suporte

Se você não encontrar a solução na documentação:

1. Verifique os logs:
   - Serial Monitor da ESP32
   - Logs da Vercel (Deployments → Logs)
   - Build Logs da Vercel

2. Teste localmente:
   - Execute `npm run dev` e teste localmente
   - Isso ajuda a identificar se o problema é no código ou na configuração

3. Consulte a documentação oficial:
   - [Next.js Documentation](https://nextjs.org/docs)
   - [Vercel Documentation](https://vercel.com/docs)
   - [ESP32 Documentation](https://docs.espressif.com/)

---

**Última atualização:** Dezembro 2024

