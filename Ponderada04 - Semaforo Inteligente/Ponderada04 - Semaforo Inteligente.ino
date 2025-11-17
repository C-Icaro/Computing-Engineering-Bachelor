#include <WiFi.h>
#include <WebServer.h>
// ==================== WI-FI AP ======================
const char* ssid = "iPhone";
const char* password = "12345678";
WebServer server(80);
// =============== PINOS DO SEMÁFORO ==================
const int S1_red    = 27;
const int S1_yellow = 14;
const int S1_green  = 12;
const int S2_red    = 33;
const int S2_yellow = 25;
const int S2_green  = 26;
// =============== LDR ================================
const int LDR_PIN = 32;

// =============== CLASSES ============================
class Semaforo {
public:
  Semaforo(int redPin, int yellowPin, int greenPin)
      : pRed(redPin), pYellow(yellowPin), pGreen(greenPin) {}

  void begin() const {
    pinMode(pRed, OUTPUT);
    pinMode(pYellow, OUTPUT);
    pinMode(pGreen, OUTPUT);
    apagar();
  }

  void verde() const { setEstado(LOW, LOW, HIGH); }
  void amarelo() const { setEstado(LOW, HIGH, LOW); }
  void vermelho() const { setEstado(HIGH, LOW, LOW); }
  void amareloPisca(bool ligado) const { setEstado(LOW, ligado ? HIGH : LOW, LOW); }
  void apagar() const { setEstado(LOW, LOW, LOW); }

private:
  int pRed;
  int pYellow;
  int pGreen;

  void setEstado(uint8_t redState, uint8_t yellowState, uint8_t greenState) const {
    digitalWrite(pRed, redState);
    digitalWrite(pYellow, yellowState);
    digitalWrite(pGreen, greenState);
  }
};

class SemaforoInteligente {
public:
  struct Telemetria {
    int luz = 0;
    bool autoAtivo = true;
    bool noturnoAtivo = false;
    unsigned long timestamp = 0;
  };

  SemaforoInteligente(Semaforo& s1Ref, Semaforo& s2Ref, int ldrPin)
      : semaforo1(s1Ref),
        semaforo2(s2Ref),
        ldrPin(ldrPin) {}

  void begin() {
    Serial.println("[SemaforoInteligente] Inicializando semaforos...");
    semaforo1.begin();
    semaforo2.begin();
    pinMode(ldrPin, INPUT);
    tempoAnterior = millis();
    tempoPisca = millis();
    atualizarTelemetria();
    Serial.println("[SemaforoInteligente] Inicializacao completa");
  }

  void atualizar() {
    lerLuminosidade();
    if (modoAuto) aplicarHisterese();
    if (modoNoturno) cicloNoturno();
    else cicloNormal();
    atualizarTelemetria();
    publicarTelemetriaMQTT();
  }

  void setModoAuto() {
    modoAuto = true;
    Serial.println("[Modo] Alterado para AUTOMATICO");
  }

  void setModoNormal() {
    modoAuto = false;
    modoNoturno = false;
    Serial.println("[Modo] Alterado para NORMAL");
  }

  void setModoNoturno() {
    modoAuto = false;
    modoNoturno = true;
    Serial.println("[Modo] Alterado para NOTURNO");
  }

  bool isModoAuto() const { return modoAuto; }
  bool isModoNoturno() const { return modoNoturno; }
  bool isModoNormal() const { return !modoAuto && !modoNoturno; }
  int getLuminosidade() const { return luminosidade; }
  const Telemetria& getTelemetria() const { return telemetriaAtual; }

private:
  // Ajustados baseado nos valores reais do LDR (Noturno: 0-2000, Diurno: 2000-5000)
  static constexpr int LIMITE_ENTRAR_NOTURNO = 1800;  // Entra no modo noturno quando < 1800
  static constexpr int LIMITE_SAIR_NOTURNO = 2200;    // Sai do modo noturno quando > 2200
  static constexpr unsigned long TEMPO_VERDE = 3000;
  static constexpr unsigned long TEMPO_AMARELO = 1500;
  static constexpr unsigned long TEMPO_PISCA = 500;

  Semaforo& semaforo1;
  Semaforo& semaforo2;
  int ldrPin;

  int luminosidade = 0;
  bool modoAuto = true;
  bool modoNoturno = false;
  unsigned long tempoAnterior = 0;
  int estado = 0;
  unsigned long tempoPisca = 0;
  bool piscaEstado = false;
  Telemetria telemetriaAtual;

  void lerLuminosidade() {
    static unsigned long ultimoPrint = 0;
    luminosidade = analogRead(ldrPin);
    // Print a cada 2 segundos para não poluir o Serial
    if (millis() - ultimoPrint >= 2000) {
      Serial.printf("[LDR] Luminosidade: %d\n", luminosidade);
      ultimoPrint = millis();
    }
  }

  void aplicarHisterese() {
    bool mudou = false;
    if (luminosidade < LIMITE_ENTRAR_NOTURNO && !modoNoturno) {
      modoNoturno = true;
      mudou = true;
      Serial.printf("[Histerese] Entrando em modo NOTURNO (LDR=%d < %d)\n", 
                    luminosidade, LIMITE_ENTRAR_NOTURNO);
    }
    if (luminosidade > LIMITE_SAIR_NOTURNO && modoNoturno) {
      modoNoturno = false;
      mudou = true;
      Serial.printf("[Histerese] Saindo do modo NOTURNO - Modo NORMAL (LDR=%d > %d)\n", 
                    luminosidade, LIMITE_SAIR_NOTURNO);
    }
  }

  void cicloNormal() {
    unsigned long agora = millis();
    switch (estado) {
      case 0:
        semaforo1.verde();
        semaforo2.vermelho();
        if (agora - tempoAnterior >= TEMPO_VERDE) transicaoPara(1, agora);
        break;
      case 1:
        semaforo1.amarelo();
        semaforo2.vermelho();
        if (agora - tempoAnterior >= TEMPO_AMARELO) transicaoPara(2, agora);
        break;
      case 2:
        semaforo1.vermelho();
        semaforo2.verde();
        if (agora - tempoAnterior >= TEMPO_VERDE) transicaoPara(3, agora);
        break;
      case 3:
        semaforo1.vermelho();
        semaforo2.amarelo();
        if (agora - tempoAnterior >= TEMPO_AMARELO) transicaoPara(0, agora);
        break;
      default:
        estado = 0;
        break;
    }
  }

  void transicaoPara(int novoEstado, unsigned long agora) {
    estado = novoEstado;
    tempoAnterior = agora;
    Serial.printf("[Ciclo Normal] Transicao para estado %d\n", novoEstado);
  }

  void cicloNoturno() {
    unsigned long agora = millis();
    if (agora - tempoPisca >= TEMPO_PISCA) {
      tempoPisca = agora;
      piscaEstado = !piscaEstado;
    }
    semaforo1.amareloPisca(piscaEstado);
    semaforo2.amareloPisca(piscaEstado);
  }

  void atualizarTelemetria() {
    telemetriaAtual.luz = luminosidade;
    telemetriaAtual.autoAtivo = modoAuto;
    telemetriaAtual.noturnoAtivo = modoNoturno;
    telemetriaAtual.timestamp = millis();
  }

  void publicarTelemetriaMQTT() {
    // Placeholder para integração futura com broker Mosquitto.
    // Assim que o cliente MQTT estiver configurado, enviar telemetriaAtual aqui.
  }
};

Semaforo semaforoPrincipal(S1_red, S1_yellow, S1_green);
Semaforo semaforoSecundario(S2_red, S2_yellow, S2_green);
SemaforoInteligente controlador(semaforoPrincipal, semaforoSecundario, LDR_PIN);
// ======================================================
// ================== FUNÇÃO HTML =======================
// ======================================================
String badge(bool ativo) {
  return ativo ? "style='background:#4CAF50;color:#fff;padding:4px 8px;border-radius:4px;'" :
                 "style='background:#bbb;color:#000;padding:4px 8px;border-radius:4px;'";
}

void handleRoot() {
  Serial.println("[HTTP] Requisicao recebida: /");
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;padding:20px;background:#f4f4f4;}button{padding:10px 18px;margin:6px 0;width:200px;font-size:15px;cursor:pointer;} .card{background:#fff;border-radius:8px;padding:16px;margin-bottom:16px;box-shadow:0 2px 6px rgba(0,0,0,0.15);} .status{display:flex;gap:10px;flex-wrap:wrap;} .status span{font-size:14px;}</style>";
  html += "<script>async function atualizar(){const r=await fetch('/status');const d=await r.json();document.getElementById('lux').innerText=d.luminosidade;document.getElementById('modoAtual').innerText=d.modoAuto? 'Automático':(d.modoNoturno? 'Noturno':'Normal');document.getElementById('badgeAuto').style.background=d.modoAuto?'#4CAF50':'#bbb';document.getElementById('badgeNoturno').style.background=d.modoNoturno?'#4CAF50':'#bbb';document.getElementById('badgeNormal').style.background=(!d.modoNoturno && !d.modoAuto)?'#4CAF50':'#bbb';}setInterval(atualizar,2000);window.onload=atualizar;</script>";
  html += "</head><body>";
  html += "<h2>Semáforo Inteligente</h2>";
  int luxAtual = controlador.getLuminosidade();
  bool autoAtivo = controlador.isModoAuto();
  bool noturnoAtivo = controlador.isModoNoturno();
  bool normalAtivo = controlador.isModoNormal();
  String modoAtual = autoAtivo ? "Automático" : (noturnoAtivo ? "Noturno" : "Normal");
  html += "<div class='card'><p><b>Luminosidade:</b> <span id='lux'>" + String(luxAtual) + "</span></p>";
  html += "<p><b>Modo atual:</b> <span id='modoAtual'>" + modoAtual + "</span></p>";
  html += "<div class='status'><span id='badgeAuto' " + badge(autoAtivo) + ">Automático</span>";
  html += "<span id='badgeNormal' " + badge(normalAtivo) + ">Normal</span>";
  html += "<span id='badgeNoturno' " + badge(noturnoAtivo && !autoAtivo) + ">Noturno</span></div></div>";
  html += "<div class='card'><a href='/auto'><button>Modo Automático</button></a><br>";
  html += "<a href='/normal'><button>Modo Normal</button></a><br>";
  html += "<a href='/noturno'><button>Modo Noturno</button></a></div>";
  html += "<div class='card'><p>Endpoint JSON: <code>/status</code></p><p>Use-o para dashboards Web ou integração MQTT futura.</p></div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
  Serial.println("[HTTP] Resposta enviada: 200 OK");
}

void setAuto()    { Serial.println("[HTTP] Requisicao recebida: /auto"); controlador.setModoAuto();   server.sendHeader("Location", "/"); server.send(303); }
void setNormal()  { Serial.println("[HTTP] Requisicao recebida: /normal"); controlador.setModoNormal(); server.sendHeader("Location", "/"); server.send(303); }
void setNoturno() { Serial.println("[HTTP] Requisicao recebida: /noturno"); controlador.setModoNoturno();server.sendHeader("Location", "/"); server.send(303); }

void handleStatus() {
  Serial.println("[HTTP] Requisicao recebida: /status");
  const auto& telemetria = controlador.getTelemetria();
  String json = "{";
  json += "\"luminosidade\":" + String(telemetria.luz) + ",";
  json += "\"modoAuto\":" + String(telemetria.autoAtivo ? "true" : "false") + ",";
  json += "\"modoNoturno\":" + String(telemetria.noturnoAtivo ? "true" : "false") + ",";
  json += "\"timestamp\":" + String(telemetria.timestamp);
  json += "}";
  server.send(200, "application/json", json);
  Serial.printf("[HTTP] JSON enviado: luz=%d, auto=%s, noturno=%s\n", 
                telemetria.luz, 
                telemetria.autoAtivo ? "true" : "false",
                telemetria.noturnoAtivo ? "true" : "false");
}
// ======================================================
// ======================== SETUP ========================
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n========================================");
  Serial.println("  SEMAFORO INTELIGENTE - INICIANDO");
  Serial.println("========================================\n");
  
  Serial.println("[Setup] Inicializando controlador...");
  controlador.begin();
  Serial.println("[Setup] Limites LDR configurados:");
  Serial.println("  - Entrar modo NOTURNO: < 1800 (faixa: 0-2000)");
  Serial.println("  - Sair modo NOTURNO:  > 2200 (faixa: 2000-5000)");
  
  Serial.println("[Setup] Configurando Access Point...");
  bool apOk = WiFi.softAP(ssid, password);
  if (apOk) {
    Serial.print("[Setup] AP criado com sucesso! SSID: ");
    Serial.println(ssid);
    Serial.print("[Setup] IP do Access Point: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("[Setup] ERRO: Falha ao criar Access Point!");
  }
  
  Serial.println("[Setup] Configurando rotas HTTP...");
  server.on("/", handleRoot);
  server.on("/auto", setAuto);
  server.on("/normal", setNormal);
  server.on("/noturno", setNoturno);
  server.on("/status", handleStatus);
  
  Serial.println("[Setup] Iniciando servidor HTTP na porta 80...");
  server.begin();
  Serial.println("[Setup] Servidor HTTP iniciado com sucesso!");
  Serial.println("\n========================================");
  Serial.println("  SISTEMA PRONTO!");
  Serial.println("========================================\n");
}
// ======================================================
// ========================= LOOP ========================
// ======================================================
void loop() {
  static unsigned long ultimoHeartbeat = 0;
  
  server.handleClient();
  controlador.atualizar();
  
  // Heartbeat a cada 10 segundos para confirmar que está rodando
  if (millis() - ultimoHeartbeat >= 10000) {
    Serial.println("[Heartbeat] Sistema operacional");
    ultimoHeartbeat = millis();
  }
}
