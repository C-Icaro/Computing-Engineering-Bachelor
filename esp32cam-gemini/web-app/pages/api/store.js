// Armazenamento compartilhado para a última imagem
// Usa sistema de arquivos para persistência em ambiente serverless
import fs from 'fs';
import path from 'path';
import os from 'os';

// Caminho para o arquivo de armazenamento
// No Vercel, usamos /tmp que é persistente durante a execução
// Localmente, usamos o diretório temporário do sistema
function getStoragePath() {
  if (process.env.VERCEL) {
    return '/tmp/latest-image.json';
  }
  // Localmente, usar diretório temporário
  return path.join(os.tmpdir(), 'esp32cam-latest-image.json');
}

const STORAGE_PATH = getStoragePath();

// Cache em memória para performance
let latestImageCache = null;
let cacheTimestamp = 0;
const CACHE_TTL = 5000; // 5 segundos

function ensureStorageDir() {
  const dir = path.dirname(STORAGE_PATH);
  if (!fs.existsSync(dir)) {
    try {
      fs.mkdirSync(dir, { recursive: true });
    } catch (error) {
      console.error('Erro ao criar diretório de storage:', error);
    }
  }
}

export function getLatestImage() {
  try {
    console.log('getLatestImage: STORAGE_PATH =', STORAGE_PATH);
    console.log('getLatestImage: VERCEL =', process.env.VERCEL);
    
    // Verificar cache primeiro
    const now = Date.now();
    if (latestImageCache && (now - cacheTimestamp) < CACHE_TTL) {
      console.log('getLatestImage: retornando do cache');
      return latestImageCache;
    }

    // Tentar ler do arquivo
    const fileExists = fs.existsSync(STORAGE_PATH);
    console.log('getLatestImage: arquivo existe?', fileExists);
    
    if (fileExists) {
      const fileContent = fs.readFileSync(STORAGE_PATH, 'utf8');
      const imageData = JSON.parse(fileContent);
      latestImageCache = imageData;
      cacheTimestamp = now;
      console.log('getLatestImage: lido do arquivo com sucesso:', {
        hasImage: !!imageData,
        filename: imageData?.filename,
        timestamp: imageData?.timestamp
      });
      return imageData;
    } else {
      console.log('getLatestImage: arquivo não existe em', STORAGE_PATH);
      console.log('getLatestImage: retornando null');
      return null;
    }
  } catch (error) {
    console.error('Erro ao ler latestImage:', error);
    console.error('Erro stack:', error.stack);
    // Retornar cache se disponível
    if (latestImageCache) {
      console.log('getLatestImage: retornando do cache devido a erro');
      return latestImageCache;
    }
    return null;
  }
}

export function setLatestImage(imageData) {
  try {
    console.log('setLatestImage chamado:', {
      filename: imageData?.filename,
      decision: imageData?.decision,
      timestamp: imageData?.timestamp,
      battery: imageData?.battery,
      imageUrlLength: imageData?.imageUrl?.length
    });
    console.log('setLatestImage: STORAGE_PATH =', STORAGE_PATH);
    console.log('setLatestImage: VERCEL =', process.env.VERCEL);

    // Atualizar cache
    latestImageCache = imageData;
    cacheTimestamp = Date.now();

    // Garantir que o diretório existe
    ensureStorageDir();

    // Salvar no arquivo (manter todos os dados incluindo imageUrl)
    const dataToStore = {
      filename: imageData.filename,
      decision: imageData.decision,
      timestamp: imageData.timestamp,
      battery: imageData.battery,
      imageUrl: imageData.imageUrl, // Manter a URL completa
    };

    fs.writeFileSync(STORAGE_PATH, JSON.stringify(dataToStore, null, 2), 'utf8');
    console.log('✓ latestImage salvo no arquivo:', STORAGE_PATH);
    console.log('✓ Tamanho do arquivo:', fs.statSync(STORAGE_PATH).size, 'bytes');
  } catch (error) {
    console.error('✗ Erro ao salvar latestImage:', error);
    console.error('Erro stack:', error.stack);
    // Mesmo com erro, manter no cache
    latestImageCache = imageData;
    cacheTimestamp = Date.now();
    console.log('Imagem mantida apenas em cache devido ao erro');
  }
}

