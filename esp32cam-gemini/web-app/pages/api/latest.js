// API route para obter a última imagem capturada
import { getLatestImage } from './store';

export default function handler(req, res) {
  if (req.method === 'GET') {
    const latestImage = getLatestImage();
    console.log('API /api/latest chamada:', {
      hasImage: !!latestImage,
      imageFilename: latestImage?.filename,
      imageTimestamp: latestImage?.timestamp
    });
    
    if (!latestImage) {
      // Retornar 200 com null para que a rota seja reconhecida
      return res.status(200).json(null);
    }
    return res.status(200).json(latestImage);
  } else {
    return res.status(405).json({ error: 'Method not allowed' });
  }
}

