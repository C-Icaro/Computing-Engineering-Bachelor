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
  console.log('=== API /api/upload chamada ===');
  console.log('Method:', req.method);
  console.log('Has Body:', !!req.body);
  console.log('Body keys:', req.body ? Object.keys(req.body) : 'none');
  console.log('Has Image:', !!(req.body && req.body.image));
  console.log('Has Decision:', !!(req.body && req.body.decision));
  console.log('Has Battery:', !!(req.body && req.body.battery));
  console.log('Image Type:', req.body?.image ? typeof req.body.image : 'none');
  console.log('Image Length:', req.body?.image ? (typeof req.body.image === 'string' ? req.body.image.length : 'buffer') : 0);
  if (req.body?.battery) {
    console.log('Battery Data:', JSON.stringify(req.body.battery));
  }
  console.log('================================');

  // Apenas aceitar POST
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  try {
    // Verificar se há body
    if (!req.body) {
      console.log('ERRO: Body vazio ou não parseado');
      return res.status(400).json({ error: 'No body provided', received: false });
    }

    // Verificar se há imagem no body
    if (!req.body.image) {
      console.log('ERRO: No image provided');
      console.log('Body keys recebidos:', Object.keys(req.body));
      console.log('Body sample (primeiros 200 chars):', JSON.stringify(req.body).substring(0, 200));
      return res.status(400).json({ 
        error: 'No image provided',
        receivedKeys: Object.keys(req.body),
        bodySample: JSON.stringify(req.body).substring(0, 200)
      });
    }

    // A imagem vem como base64 ou como buffer
    let imageData;
    let decision = req.body.decision || 'unknown';
    let battery = req.body.battery || null; // Dados de bateria (voltage e percentage)

    console.log('Processando imagem...');
    console.log('Decision:', decision);
    console.log('Battery:', battery ? JSON.stringify(battery) : 'null');

    if (typeof req.body.image === 'string') {
      // Se for base64, manter como string
      imageData = req.body.image;
      console.log('Imagem é string, tamanho:', imageData.length);
    } else {
      // Se for buffer, converter para base64
      imageData = `data:image/jpeg;base64,${req.body.image.toString('base64')}`;
      console.log('Imagem é buffer, convertido para base64');
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

    console.log('Armazenando imagem...');
    // Armazenar como última imagem
    setLatestImage(imageDataObj);

    console.log('✓ Imagem processada com sucesso:', {
      filename: imageDataObj.filename,
      decision: imageDataObj.decision,
      timestamp: imageDataObj.timestamp,
      battery: imageDataObj.battery,
      imageUrlLength: imageDataObj.imageUrl ? imageDataObj.imageUrl.length : 0
    });

    // Retornar sucesso
    return res.status(200).json({
      success: true,
      message: 'Image received and stored',
      filename: imageDataObj.filename,
      decision: imageDataObj.decision,
      timestamp: imageDataObj.timestamp,
      battery: imageDataObj.battery
    });

  } catch (error) {
    console.error('✗ ERRO ao processar imagem:', error);
    console.error('Error name:', error.name);
    console.error('Error message:', error.message);
    console.error('Error stack:', error.stack);
    if (error.message) {
      console.error('Error details:', error.message);
    }
    return res.status(500).json({ 
      error: 'Internal server error', 
      details: error.message,
      name: error.name
    });
  }
}

