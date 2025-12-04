// API route para receber imagens da ESP32-CAM
import { setLatestImage } from './store';

// Configurar limite de tamanho do body para 10MB
export const config = {
  api: {
    bodyParser: {
      sizeLimit: '10mb',
    },
  },
}

export default async function handler(req, res) {
  // Log para debug
  console.log('API /api/upload chamada:', {
    method: req.method,
    hasBody: !!req.body,
    hasImage: !!(req.body && req.body.image),
    imageType: req.body?.image ? typeof req.body.image : 'none',
    imageLength: req.body?.image ? (typeof req.body.image === 'string' ? req.body.image.length : 'buffer') : 0
  });

  // Apenas aceitar POST
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  try {
    // Verificar se há imagem no body
    if (!req.body || !req.body.image) {
      console.log('Erro: No image provided', { body: req.body });
      return res.status(400).json({ error: 'No image provided' });
    }

    // A imagem vem como base64 ou como buffer
    let imageData;
    let decision = req.body.decision || 'unknown';
    let battery = req.body.battery || null; // Dados de bateria (voltage e percentage)

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
      timestamp,
      battery // Incluir dados de bateria
    };

    // Armazenar como última imagem
    setLatestImage(imageDataObj);

    console.log('Imagem processada com sucesso:', {
      filename: imageDataObj.filename,
      decision: imageDataObj.decision,
      timestamp: imageDataObj.timestamp,
      battery: imageDataObj.battery
    });

    // Retornar sucesso
    return res.status(200).json({
      success: true,
      ...imageDataObj
    });

  } catch (error) {
    console.error('Error processing image:', error);
    console.error('Error stack:', error.stack);
    return res.status(500).json({ error: 'Internal server error', details: error.message });
  }
}

