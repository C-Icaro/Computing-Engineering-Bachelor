// Rota de teste para verificar se as rotas da API estão funcionando
export default function handler(req, res) {
  return res.status(200).json({ 
    message: 'API route is working!',
    timestamp: new Date().toISOString()
  });
}


