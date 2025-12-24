/**
 * API Route: /api/upload
 * 
 * Recebe imagens capturadas pela ESP32-CAM junto com dados de detecção e bateria.
 * Armazena a última imagem recebida para visualização no frontend.
 * 
 * @module pages/api/upload
 * @requires ./store
 */

import { setLatestImage } from './store';

// ============================================================================
// ==== CONSTANTES DE CONFIGURAÇÃO ====
// ============================================================================

/** Limite de tamanho do body parser (necessário para imagens base64 grandes) */
const BODY_SIZE_LIMIT = '10mb';

/** Valor padrão para decisão quando não fornecida */
const DEFAULT_DECISION = 'unknown';

/** Prefixo do nome do arquivo gerado */
const FILENAME_PREFIX = 'capture-';

/** Extensão do arquivo gerado */
const FILENAME_EXTENSION = '.jpg';

/** Caracteres a serem substituídos no timestamp para nome de arquivo */
const TIMESTAMP_REPLACE_PATTERN = /[:.]/g;

/** Caractere de substituição no timestamp */
const TIMESTAMP_REPLACE_CHAR = '-';

/** Limite de caracteres para sample do body em logs de erro */
const ERROR_BODY_SAMPLE_LENGTH = 200;

/**
 * Configuração do body parser para Next.js
 * Define limite de tamanho do body para 10MB (necessário para imagens base64)
 */
export const config = {
  api: {
    bodyParser: {
      sizeLimit: BODY_SIZE_LIMIT,
    },
  },
}

/**
 * Handler principal da API route
 * 
 * @param {Object} req - Objeto de requisição do Next.js
 * @param {string} req.method - Método HTTP (deve ser POST)
 * @param {Object} req.body - Corpo da requisição
 * @param {string} req.body.image - Imagem em base64 (data URI ou string)
 * @param {string} req.body.decision - Decisão do Gemini: "person" ou "no_person"
 * @param {Object} req.body.battery - Dados da bateria
 * @param {number} req.body.battery.voltage - Tensão da bateria em Volts
 * @param {number} req.body.battery.percentage - Porcentagem de carga (0-100)
 * 
 * @param {Object} res - Objeto de resposta do Next.js
 * 
 * @returns {Promise<void>}
 * 
 * @throws {400} Se o método não for POST
 * @throws {400} Se o body estiver vazio ou não contiver imagem
 * @throws {500} Em caso de erro interno do servidor
 */
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
      console.log('Body sample (primeiros chars):', JSON.stringify(req.body).substring(0, ERROR_BODY_SAMPLE_LENGTH));
      return res.status(400).json({ 
        error: 'No image provided',
        receivedKeys: Object.keys(req.body),
        bodySample: JSON.stringify(req.body).substring(0, 200)
      });
    }

    // Processar dados recebidos
    let imageData;
    let decision = req.body.decision || DEFAULT_DECISION;
    let battery = req.body.battery || null; // Dados de bateria (voltage e percentage)

    console.log('Processando imagem...');
    console.log('Decision:', decision);
    console.log('Battery:', battery ? JSON.stringify(battery) : 'null');

    // Converter imagem para formato padrão (data URI base64)
    if (typeof req.body.image === 'string') {
      // Se já for string (base64), manter como está
      imageData = req.body.image;
      console.log('Imagem é string, tamanho:', imageData.length);
    } else if (Buffer.isBuffer(req.body.image)) {
      // Se for buffer, converter para base64 data URI
      imageData = `data:image/jpeg;base64,${req.body.image.toString('base64')}`;
      console.log('Imagem é buffer, convertido para base64');
    } else {
      // Tipo de imagem não suportado
      console.error('ERRO: Tipo de imagem não suportado:', typeof req.body.image);
      return res.status(400).json({ 
        error: 'Invalid image format',
        receivedType: typeof req.body.image
      });
    }

    // Validação: verificar se a imagem não está vazia
    if (!imageData || imageData.length === 0) {
      console.error('ERRO: Imagem está vazia');
      return res.status(400).json({ error: 'Image data is empty' });
    }

    // Gerar nome do arquivo com timestamp (formato ISO)
    const timestamp = new Date().toISOString();
    const filename = `${FILENAME_PREFIX}${timestamp.replace(TIMESTAMP_REPLACE_PATTERN, TIMESTAMP_REPLACE_CHAR)}${FILENAME_EXTENSION}`;

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

