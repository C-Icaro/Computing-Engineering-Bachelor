/**
 * Página principal do visualizador de imagens da ESP32-CAM
 * 
 * Exibe a última imagem capturada pela ESP32-CAM junto com informações de:
 * - Detecção de pessoa (via Gemini AI)
 * - Status da bateria (tensão e porcentagem)
 * - Timestamp da captura
 * 
 * @module pages/index
 * @component
 * 
 * @returns {JSX.Element} Componente React da página principal
 */
import { useState, useEffect } from 'react';
import Head from 'next/head';

/**
 * Componente principal da página Home
 * 
 * Gerencia o estado da última imagem, atualização automática e tratamento de erros.
 * 
 * @function Home
 * @returns {JSX.Element}
 */
// ============================================================================
// ==== CONSTANTES DE CONFIGURAÇÃO ====
// ============================================================================

/** Intervalo de atualização automática em milissegundos */
const AUTO_REFRESH_INTERVAL = 5000; // 5 segundos

/** Endpoint da API para buscar última imagem */
const API_LATEST_ENDPOINT = '/api/latest';

/** Locale para formatação de data */
const DATE_LOCALE = 'pt-BR';

// ============================================================================
// ==== COMPONENTE PRINCIPAL ====
// ============================================================================

export default function Home() {
  // Estados do componente (encapsulados)
  const [latestImage, setLatestImage] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [autoRefresh, setAutoRefresh] = useState(true);

  /**
   * Busca a última imagem capturada da API
   * 
   * @async
   * @function fetchLatestImage
   * @returns {Promise<void>}
   * 
   * @description
   * Faz requisição GET para /api/latest e atualiza o estado local.
   * Trata erros de rede e respostas vazias.
   */
  const fetchLatestImage = async () => {
    try {
      // Fazer requisição para API
      const response = await fetch(API_LATEST_ENDPOINT);
      
      // Verificar se a resposta foi bem-sucedida
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      // Parsear JSON da resposta
      const data = await response.json();
      
      // Validar dados recebidos
      if (data && typeof data === 'object') {
        setLatestImage(data);
        setError(null);
      } else {
        // Resposta null ou inválida
        setLatestImage(null);
        setError('Nenhuma imagem disponível ainda');
      }
    } catch (err) {
      // Tratamento de erros de rede ou parsing
      console.error('✗ ERRO ao buscar imagem:', err);
      setError('Erro ao buscar imagem. Verifique a conexão.');
      setLatestImage(null);
    } finally {
      setLoading(false);
    }
  };

  /**
   * Effect para buscar imagem inicial e configurar auto-refresh
   * 
   * @effect
   * @description
   * - Busca imagem imediatamente ao montar o componente
   * - Configura intervalo de atualização automática se habilitado
   * - Limpa intervalo ao desmontar ou quando autoRefresh muda
   */
  useEffect(() => {
    // Buscar imagem imediatamente
    fetchLatestImage();
    
    // Configurar auto-refresh se habilitado
    if (autoRefresh) {
      const interval = setInterval(fetchLatestImage, AUTO_REFRESH_INTERVAL);
      
      // Cleanup: limpar intervalo ao desmontar ou quando autoRefresh mudar
      return () => {
        clearInterval(interval);
      };
    }
    
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [autoRefresh]); // Dependência: autoRefresh

  /**
   * Retorna cor e texto baseado na decisão do Gemini
   * 
   * @param {string} decision - Decisão: "person", "no_person" ou outro
   * @returns {Object} Objeto com color (hex), text (string) e icon (emoji)
   */
  const getDecisionColor = (decision) => {
    if (decision === 'person') {
      return { color: '#ef4444', text: 'Pessoa Detectada', icon: '🔴' };
    } else if (decision === 'no_person') {
      return { color: '#22c55e', text: 'Sem Pessoa', icon: '🟢' };
    }
    return { color: '#6b7280', text: 'Desconhecido', icon: '⚪' };
  };

  /**
   * Retorna cor baseada na porcentagem da bateria
   * 
   * @param {number} percentage - Porcentagem da bateria (0-100)
   * @returns {string} Cor em hexadecimal
   * 
   * @description
   * - >= 80%: Verde (#22c55e) - Alta
   * - >= 50%: Amarelo (#f59e0b) - Média
   * - >= 20%: Laranja (#f97316) - Baixa
   * - < 20%: Vermelho (#ef4444) - Crítica
   */
  const getBatteryColor = (percentage) => {
    if (percentage >= 80) return '#22c55e'; // Verde - Alta
    if (percentage >= 50) return '#f59e0b'; // Amarelo - Média
    if (percentage >= 20) return '#f97316'; // Laranja - Baixa
    return '#ef4444'; // Vermelho - Crítica
  };

  /**
   * Retorna status textual da bateria baseado na porcentagem
   * 
   * @param {number} percentage - Porcentagem da bateria (0-100)
   * @returns {string} Status: "Alta", "Média", "Baixa" ou "Crítica"
   */
  const getBatteryStatus = (percentage) => {
    if (percentage >= 80) return 'Alta';
    if (percentage >= 50) return 'Média';
    if (percentage >= 20) return 'Baixa';
    return 'Crítica';
  };

  const decisionInfo = latestImage ? getDecisionColor(latestImage.decision) : null;

  return (
    <>
      <Head>
        <title>ESP32-CAM + Gemini - Visualizador</title>
        <meta name="description" content="Visualize imagens capturadas pela ESP32-CAM com detecção de pessoas via Gemini" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
      </Head>

      <main style={styles.container}>
        <div style={styles.header}>
          <h1 style={styles.title}>ESP32-CAM + Gemini</h1>
          <p style={styles.subtitle}>Visualizador de Capturas em Tempo Real</p>
        </div>

        <div style={styles.controls}>
          <button
            onClick={fetchLatestImage}
            style={styles.button}
            disabled={loading}
          >
            {loading ? 'Carregando...' : '🔄 Atualizar'}
          </button>
          <label style={styles.checkboxLabel}>
            <input
              type="checkbox"
              checked={autoRefresh}
              onChange={(e) => setAutoRefresh(e.target.checked)}
              style={styles.checkbox}
            />
            Auto-atualizar (5s)
          </label>
        </div>

        {loading && !latestImage && (
          <div style={styles.message}>
            <p>Carregando...</p>
          </div>
        )}

        {error && !latestImage && (
          <div style={styles.message}>
            <p style={styles.errorText}>{error}</p>
            <p style={styles.hint}>Aguarde a ESP32-CAM enviar uma imagem</p>
          </div>
        )}

        {latestImage && (
          <div style={styles.imageContainer}>
            <div style={styles.infoBar}>
              <div style={styles.infoItem}>
                <span style={styles.infoLabel}>Decisão:</span>
                <span style={{ ...styles.decisionBadge, backgroundColor: decisionInfo.color }}>
                  {decisionInfo.icon} {decisionInfo.text}
                </span>
              </div>
              {latestImage.battery && (
                <div style={styles.infoItem}>
                  <span style={styles.infoLabel}>Bateria:</span>
                  <span style={{
                    ...styles.batteryBadge,
                    backgroundColor: getBatteryColor(latestImage.battery.percentage)
                  }}>
                    🔋 {latestImage.battery.percentage}% ({latestImage.battery.voltage.toFixed(2)}V) - {getBatteryStatus(latestImage.battery.percentage)}
                  </span>
                </div>
              )}
              <div style={styles.infoItem}>
                <span style={styles.infoLabel}>Capturada em:</span>
                <span style={styles.timestamp}>
                  {new Date(latestImage.timestamp).toLocaleString(DATE_LOCALE)}
                </span>
              </div>
            </div>

            <div style={styles.imageWrapper}>
              <img
                src={latestImage.imageUrl}
                alt="Captura da ESP32-CAM"
                style={styles.image}
                onError={() => setError('Erro ao carregar imagem')}
              />
            </div>

            <div style={styles.footer}>
              <p style={styles.filename}>Arquivo: {latestImage.filename}</p>
            </div>
          </div>
        )}

        <div style={styles.footerInfo}>
          <p>Endpoint da API: <code>/api/upload</code></p>
          <p>Configure a ESP32-CAM para enviar imagens para este endpoint</p>
        </div>
      </main>
    </>
  );
}

const styles = {
  container: {
    minHeight: '100vh',
    padding: '20px',
    backgroundColor: '#f3f4f6',
    fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif',
  },
  header: {
    textAlign: 'center',
    marginBottom: '30px',
  },
  title: {
    fontSize: '2.5rem',
    fontWeight: 'bold',
    color: '#1f2937',
    margin: '0 0 10px 0',
  },
  subtitle: {
    fontSize: '1.1rem',
    color: '#6b7280',
    margin: 0,
  },
  controls: {
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
    gap: '20px',
    marginBottom: '30px',
    flexWrap: 'wrap',
  },
  button: {
    padding: '12px 24px',
    fontSize: '1rem',
    backgroundColor: '#3b82f6',
    color: 'white',
    border: 'none',
    borderRadius: '8px',
    cursor: 'pointer',
    fontWeight: '600',
    transition: 'background-color 0.2s',
  },
  checkboxLabel: {
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    fontSize: '0.95rem',
    color: '#374151',
    cursor: 'pointer',
  },
  checkbox: {
    width: '18px',
    height: '18px',
    cursor: 'pointer',
  },
  message: {
    textAlign: 'center',
    padding: '40px',
    backgroundColor: 'white',
    borderRadius: '12px',
    boxShadow: '0 1px 3px rgba(0,0,0,0.1)',
  },
  errorText: {
    color: '#ef4444',
    fontSize: '1.1rem',
    marginBottom: '10px',
  },
  hint: {
    color: '#6b7280',
    fontSize: '0.9rem',
  },
  imageContainer: {
    maxWidth: '1200px',
    margin: '0 auto',
    backgroundColor: 'white',
    borderRadius: '12px',
    boxShadow: '0 4px 6px rgba(0,0,0,0.1)',
    overflow: 'hidden',
  },
  infoBar: {
    display: 'flex',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: '20px',
    backgroundColor: '#f9fafb',
    borderBottom: '1px solid #e5e7eb',
    flexWrap: 'wrap',
    gap: '15px',
  },
  infoItem: {
    display: 'flex',
    alignItems: 'center',
    gap: '10px',
  },
  infoLabel: {
    fontSize: '0.9rem',
    color: '#6b7280',
    fontWeight: '500',
  },
  decisionBadge: {
    padding: '6px 12px',
    borderRadius: '20px',
    color: 'white',
    fontSize: '0.9rem',
    fontWeight: '600',
  },
  timestamp: {
    fontSize: '0.9rem',
    color: '#374151',
    fontWeight: '500',
  },
  batteryBadge: {
    padding: '6px 12px',
    borderRadius: '20px',
    color: 'white',
    fontSize: '0.9rem',
    fontWeight: '600',
  },
  imageWrapper: {
    width: '100%',
    display: 'flex',
    justifyContent: 'center',
    backgroundColor: '#000',
    padding: '20px',
  },
  image: {
    maxWidth: '100%',
    height: 'auto',
    borderRadius: '8px',
    boxShadow: '0 4px 12px rgba(0,0,0,0.3)',
  },
  footer: {
    padding: '15px 20px',
    backgroundColor: '#f9fafb',
    borderTop: '1px solid #e5e7eb',
  },
  filename: {
    margin: 0,
    fontSize: '0.85rem',
    color: '#6b7280',
    fontFamily: 'monospace',
  },
  footerInfo: {
    marginTop: '30px',
    textAlign: 'center',
    color: '#6b7280',
    fontSize: '0.9rem',
  },
};

