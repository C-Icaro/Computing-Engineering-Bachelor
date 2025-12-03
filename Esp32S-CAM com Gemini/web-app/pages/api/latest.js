// API route para obter a última imagem capturada
// Em produção, isso deveria buscar de um banco de dados ou storage
let latestImage = null;

export default function handler(req, res) {
  if (req.method === 'GET') {
    if (!latestImage) {
      return res.status(404).json({ error: 'No image available' });
    }
    return res.status(200).json(latestImage);
  } else if (req.method === 'POST') {
    // Permitir atualizar a última imagem via POST também
    latestImage = req.body;
    return res.status(200).json({ success: true });
  } else {
    return res.status(405).json({ error: 'Method not allowed' });
  }
}

// Função para armazenar a última imagem (chamada pela API de upload)
export function setLatestImage(imageData) {
  latestImage = imageData;
}

