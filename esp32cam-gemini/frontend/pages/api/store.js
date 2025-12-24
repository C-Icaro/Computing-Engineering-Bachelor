/**
 * Módulo de armazenamento persistente para última imagem capturada
 * 
 * Usa sistema de arquivos para persistência em ambiente serverless (Vercel).
 * Implementa cache em memória para melhor performance.
 * 
 * @module pages/api/store
 * @requires fs
 * @requires path
 * @requires os
 */

import fs from 'fs';
import path from 'path';
import os from 'os';

// ============================================================================
// ==== CONSTANTES DE CONFIGURAÇÃO ====
// ============================================================================

/** Tempo de vida do cache em memória (milissegundos) */
const CACHE_TTL = 5000; // 5 segundos

/** Nome do arquivo de armazenamento */
const STORAGE_FILENAME = 'esp32cam-latest-image.json';

/** Caminho do arquivo no Vercel (diretório temporário) */
const VERCEL_STORAGE_PATH = '/tmp/latest-image.json';

/** Encoding usado para leitura/escrita de arquivos */
const FILE_ENCODING = 'utf8';

/** Modo de criação de diretório (recursivo) */
const DIR_CREATE_MODE = { recursive: true };

// ============================================================================
// ==== VARIÁVEIS DE ESTADO (Encapsuladas) ====
// ============================================================================

/**
 * Cache em memória para melhor performance
 * @type {Object|null}
 * @private
 */
let latestImageCache = null;

/**
 * Timestamp da última atualização do cache
 * @type {number}
 * @private
 */
let cacheTimestamp = 0;

// ============================================================================
// ==== FUNÇÕES AUXILIARES ====
// ============================================================================

/**
 * Determina o caminho de armazenamento baseado no ambiente
 * 
 * @returns {string} Caminho completo do arquivo de armazenamento
 * 
 * @description
 * - No Vercel: usa /tmp (diretório temporário persistente durante execução)
 * - Localmente: usa diretório temporário do sistema operacional
 */
function getStoragePath() {
  if (process.env.VERCEL) {
    return VERCEL_STORAGE_PATH;
  }
  // Localmente, usar diretório temporário do sistema
  return path.join(os.tmpdir(), STORAGE_FILENAME);
}

/**
 * Caminho completo do arquivo de armazenamento
 * @constant
 * @type {string}
 */
const STORAGE_PATH = getStoragePath();

/**
 * Garante que o diretório de armazenamento existe
 * 
 * @returns {boolean} true se o diretório existe ou foi criado, false em caso de erro
 * 
 * @description
 * Cria o diretório recursivamente se não existir.
 * Trata erros de permissão ou sistema de arquivos.
 */
function ensureStorageDir() {
  try {
    const dir = path.dirname(STORAGE_PATH);
    
    // Verificar se o diretório já existe
    if (fs.existsSync(dir)) {
      return true;
    }
    
    // Criar diretório recursivamente
    fs.mkdirSync(dir, DIR_CREATE_MODE);
    return true;
  } catch (error) {
    console.error('✗ ERRO ao criar diretório de storage:', error);
    console.error('Diretório:', path.dirname(STORAGE_PATH));
    console.error('Erro:', error.message);
    return false;
  }
}

/**
 * Obtém a última imagem armazenada
 * 
 * @returns {Object|null} Objeto com dados da imagem ou null se não houver imagem
 * 
 * @description
 * Prioriza cache em memória (se válido) para melhor performance.
 * Se cache expirado ou inexistente, lê do arquivo.
 * Em caso de erro, tenta retornar cache como fallback.
 * 
 * @example
 * const image = getLatestImage();
 * if (image) {
 *   console.log('Última imagem:', image.filename);
 * }
 */
export function getLatestImage() {
  try {
    // Log apenas em desenvolvimento
    if (process.env.NODE_ENV === 'development') {
      console.log('getLatestImage: STORAGE_PATH =', STORAGE_PATH);
      console.log('getLatestImage: VERCEL =', process.env.VERCEL);
    }
    
    // Verificar cache primeiro (mais rápido)
    const now = Date.now();
    if (latestImageCache && (now - cacheTimestamp) < CACHE_TTL) {
      if (process.env.NODE_ENV === 'development') {
        console.log('getLatestImage: retornando do cache');
      }
      return latestImageCache;
    }

    // Cache expirado ou inexistente, ler do arquivo
    const fileExists = fs.existsSync(STORAGE_PATH);
    
    if (process.env.NODE_ENV === 'development') {
      console.log('getLatestImage: arquivo existe?', fileExists);
    }
    
    if (fileExists) {
      // Ler e parsear arquivo JSON
      const fileContent = fs.readFileSync(STORAGE_PATH, FILE_ENCODING);
      
      // Validação: verificar se o conteúdo não está vazio
      if (!fileContent || fileContent.trim().length === 0) {
        console.warn('AVISO: Arquivo de storage está vazio');
        return null;
      }
      
      const imageData = JSON.parse(fileContent);
      
      // Validação: verificar se os dados são válidos
      if (!imageData || typeof imageData !== 'object') {
        console.warn('AVISO: Dados do arquivo inválidos');
        return null;
      }
      
      // Atualizar cache
      latestImageCache = imageData;
      cacheTimestamp = now;
      
      if (process.env.NODE_ENV === 'development') {
        console.log('✓ getLatestImage: lido do arquivo com sucesso:', {
          hasImage: !!imageData,
          filename: imageData?.filename,
          timestamp: imageData?.timestamp
        });
      }
      
      return imageData;
    } else {
      if (process.env.NODE_ENV === 'development') {
        console.log('getLatestImage: arquivo não existe em', STORAGE_PATH);
        console.log('getLatestImage: retornando null');
      }
      return null;
    }
  } catch (error) {
    // Tratamento de erros robusto
    console.error('✗ ERRO ao ler latestImage:', error);
    console.error('Erro name:', error.name);
    console.error('Erro message:', error.message);
    if (error.stack) {
      console.error('Erro stack:', error.stack);
    }
    
    // Fallback: retornar cache se disponível (mesmo expirado)
    if (latestImageCache) {
      console.log('getLatestImage: retornando do cache devido a erro (fallback)');
      return latestImageCache;
    }
    
    return null;
  }
}

/**
 * Armazena a última imagem capturada
 * 
 * @param {Object} imageData - Dados da imagem a serem armazenados
 * @param {string} imageData.filename - Nome do arquivo
 * @param {string} imageData.decision - Decisão do Gemini ("person" ou "no_person")
 * @param {string} imageData.imageUrl - URL da imagem (data URI base64)
 * @param {string} imageData.timestamp - Timestamp ISO da captura
 * @param {Object|null} imageData.battery - Dados da bateria (voltage, percentage)
 * 
 * @returns {boolean} true se armazenado com sucesso, false em caso de erro
 * 
 * @description
 * Atualiza cache em memória e salva no arquivo para persistência.
 * Em caso de erro ao salvar arquivo, mantém apenas em cache.
 * 
 * @example
 * setLatestImage({
 *   filename: 'capture-2025-12-04.jpg',
 *   decision: 'person',
 *   imageUrl: 'data:image/jpeg;base64,...',
 *   timestamp: '2025-12-04T23:43:08.749Z',
 *   battery: { voltage: 3.798, percentage: 64 }
 * });
 */
export function setLatestImage(imageData) {
  // Validação: verificar se imageData é válido
  if (!imageData || typeof imageData !== 'object') {
    console.error('✗ ERRO: imageData inválido ou nulo');
    return false;
  }
  
  // Validação: verificar campos obrigatórios
  if (!imageData.filename || !imageData.imageUrl || !imageData.timestamp) {
    console.error('✗ ERRO: imageData está faltando campos obrigatórios');
    console.error('Campos recebidos:', Object.keys(imageData));
    return false;
  }
  
  try {
    if (process.env.NODE_ENV === 'development') {
      console.log('setLatestImage chamado:', {
        filename: imageData?.filename,
        decision: imageData?.decision,
        timestamp: imageData?.timestamp,
        battery: imageData?.battery,
        imageUrlLength: imageData?.imageUrl?.length
      });
      console.log('setLatestImage: STORAGE_PATH =', STORAGE_PATH);
      console.log('setLatestImage: VERCEL =', process.env.VERCEL);
    }

    // Atualizar cache em memória (sempre, mesmo se falhar ao salvar arquivo)
    latestImageCache = imageData;
    cacheTimestamp = Date.now();

    // Garantir que o diretório existe antes de salvar
    if (!ensureStorageDir()) {
      console.warn('AVISO: Não foi possível criar diretório, mantendo apenas em cache');
      return false;
    }

    // Preparar dados para armazenamento (manter todos os campos)
    const dataToStore = {
      filename: imageData.filename,
      decision: imageData.decision || 'unknown',
      timestamp: imageData.timestamp,
      battery: imageData.battery || null,
      imageUrl: imageData.imageUrl, // Manter a URL completa (base64)
    };

    // Salvar no arquivo com formatação JSON legível
    const jsonData = JSON.stringify(dataToStore, null, 2);
    fs.writeFileSync(STORAGE_PATH, jsonData, FILE_ENCODING);
    
    // Verificar se o arquivo foi salvo corretamente
    if (fs.existsSync(STORAGE_PATH)) {
      const fileStats = fs.statSync(STORAGE_PATH);
      if (process.env.NODE_ENV === 'development') {
        console.log('✓ latestImage salvo no arquivo:', STORAGE_PATH);
        console.log('✓ Tamanho do arquivo:', fileStats.size, 'bytes');
      }
      return true;
    } else {
      console.warn('AVISO: Arquivo não foi criado após writeFileSync');
      return false;
    }
  } catch (error) {
    // Tratamento de erros robusto
    console.error('✗ ERRO ao salvar latestImage:', error);
    console.error('Erro name:', error.name);
    console.error('Erro message:', error.message);
    if (error.stack) {
      console.error('Erro stack:', error.stack);
    }
    
    // Fallback: manter no cache mesmo com erro ao salvar arquivo
    latestImageCache = imageData;
    cacheTimestamp = Date.now();
    console.log('AVISO: Imagem mantida apenas em cache devido ao erro');
    return false;
  }
}

