// API route para receber imagens da ESP32-CAM
import { setLatestImage } from './latest';

export default async function handler(req, res) {
  // Apenas aceitar POST
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  try {
    // Verificar se há imagem no body
    if (!req.body || !req.body.image) {
      return res.status(400).json({ error: 'No image provided' });
    }

    // A imagem vem como base64 ou como buffer
    let imageData;
    let decision = req.body.decision || 'unknown';

    if (typeof req.body.image === 'string') {
      // Se for base64, manter como string
      imageData = req.body.image;
    } else {
      // Se for buffer, converter para base64
      imageData = `data:image/jpeg;base64,${req.body.image.toString('base64')}`;
    }

    // Gerar nome do arquivo com timestamp
    const timestamp = new Date().toISOString();
    const filename = `capture-${timestamp.replace(/[:.]/g, '-')}.jpg`;

    // Preparar dados da imagem
    const imageDataObj = {
      filename,
      decision,
      imageUrl: imageData,
      timestamp
    };

    // Armazenar como última imagem
    setLatestImage(imageDataObj);

    // Retornar sucesso
    return res.status(200).json({
      success: true,
      ...imageDataObj
    });

  } catch (error) {
    console.error('Error processing image:', error);
    return res.status(500).json({ error: 'Internal server error' });
  }
}

