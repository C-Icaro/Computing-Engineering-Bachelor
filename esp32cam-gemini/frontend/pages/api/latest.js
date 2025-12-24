/**
 * API Route: /api/latest
 * 
 * Retorna a última imagem capturada pela ESP32-CAM junto com seus metadados.
 * Se nenhuma imagem estiver disponível, retorna null com status 200.
 * 
 * @module pages/api/latest
 * @requires ./store
 */

import { getLatestImage } from './store';

/**
 * Handler principal da API route
 * 
 * @param {Object} req - Objeto de requisição do Next.js
 * @param {string} req.method - Método HTTP (deve ser GET)
 * 
 * @param {Object} res - Objeto de resposta do Next.js
 * 
 * @returns {void}
 * 
 * @throws {405} Se o método não for GET
 * 
 * @example
 * // Resposta quando há imagem:
 * {
 *   filename: "capture-2025-12-04T23-43-08-749Z.jpg",
 *   decision: "no_person",
 *   imageUrl: "data:image/jpeg;base64,...",
 *   timestamp: "2025-12-04T23:43:08.749Z",
 *   battery: { voltage: 3.798, percentage: 64 }
 * }
 * 
 * // Resposta quando não há imagem:
 * null
 */
export default function handler(req, res) {
  // Validar método HTTP
  if (req.method !== 'GET') {
    return res.status(405).json({ error: 'Method not allowed' });
  }
  
  // Obter última imagem armazenada
  const latestImage = getLatestImage();
  
  // Log para debug (apenas em desenvolvimento)
  if (process.env.NODE_ENV === 'development') {
    console.log('API /api/latest chamada:', {
      hasImage: !!latestImage,
      imageFilename: latestImage?.filename,
      imageTimestamp: latestImage?.timestamp
    });
  }
  
  // Retornar imagem ou null (sempre com status 200 para que a rota seja reconhecida)
  if (!latestImage) {
    return res.status(200).json(null);
  }
  
  return res.status(200).json(latestImage);
}

