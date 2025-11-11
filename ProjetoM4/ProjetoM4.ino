#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

// ===== PINAGEM RFID =====
#define SS_PIN     21
#define RST_PIN    22

// ===== LEDS =====
#define LED_GREEN  27
#define LED_RED    26

// ===== BUZZER =====
#define BUZZER_PIN 12

// ===== RELÉ (aciona trava 12V) =====
#define RELAY_PIN  25
#define RELAY_ACTIVE_LOW true  // Relé ativa com LOW

// ===== BOTÃO (pull-up interno) =====
#define BUTTON_PIN 14   // pino do botão

// ===== LCD I2C =====
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
#define SDA_LCD  32
#define SCL_LCD  33

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// ===== TEMPOS =====
const unsigned long GREEN_TIME_MS = 4000;
const unsigned long RED_TIME_MS   = 2000;

// ===== UIDs autorizados =====
String authorizedUIDs[] = {
  "F3949B1A",
};
const int authorizedCount = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

// ===== RFID =====
MFRC522 rfid(SS_PIN, RST_PIN);

// ===== ESTADOS =====
String lastSeenUID = "";
bool cardPresent = false;
unsigned long greenOffAt = 0;
unsigned long redOffAt   = 0;
unsigned long relayOffAt = 0;

// ---------- BUZZER ----------
void beep(unsigned int freq, unsigned int ms) {
  tone(BUZZER_PIN, freq);
  delay(ms);
  noTone(BUZZER_PIN);
}
void buzzApproved() { beep(1200,120); delay(100); beep(1200,120); }
void buzzDenied()   { beep(400,400); }

// ---------- LCD ----------
void lcdShowIdle() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Aproxime o");
  lcd.setCursor(0,1); lcd.print("cartao!");
}
void lcdShowApproved() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Acesso liberado!");
}
void lcdShowDenied() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Acesso negado!");
}

// ---------- RELÉ ----------
void relayWrite(bool on) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  }
}

void relayForceOff() {
  // Força o relé a desligar, garantindo o estado correto do pino
  relayOffAt = 0;
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, HIGH);  // HIGH = desligado para relé active-low
  } else {
    digitalWrite(RELAY_PIN, LOW);   // LOW = desligado para relé active-high
  }
}

bool isAuthorized(const String &uid) {
  for (int i = 0; i < authorizedCount; i++) {
    if (uid.equalsIgnoreCase(authorizedUIDs[i])) return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== RFID + LEDs + Buzzer + LCD + RELE + BOTAO ===");

  // LCD
  Wire.begin(SDA_LCD, SCL_LCD);
  lcd.init();
  lcd.backlight();
  lcdShowIdle();

  // LEDs, buzzer, relé, botão
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // botão com pull-up interno

  // Garante estado inicial desligado
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  // CRÍTICO: Para relé active-low, HIGH = desligado. Define ANTES de qualquer outra operação
  digitalWrite(RELAY_PIN, HIGH);  // HIGH = desligado para relé active-low
  delay(10);  // Pequeno delay para garantir que o estado seja estabelecido
  relayOffAt = 0;  // garante timer zerado
  Serial.println("Relé inicializado: DESLIGADO (HIGH)");

  // RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RC522 iniciado:");
  rfid.PCD_DumpVersionToSerial();

  Serial.println("Aproxime o cartao ou pressione o botao...");
}

void loop() {
  unsigned long now = millis();

  // ======== Leitura do botão ========
  static bool lastButtonState = HIGH;
  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW; // LOW = pressionado
  
  // Detecta apenas a transição de solto para pressionado (edge detection)
  if (buttonPressed && !lastButtonState) {
    Serial.println("Botão pressionado — acionando relé manualmente!");
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    greenOffAt = now + GREEN_TIME_MS;
    redOffAt = 0;
    relayWrite(true);
    relayOffAt = now + GREEN_TIME_MS; // mesmo tempo do LED verde
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("Botao acionado!");
  }
  lastButtonState = buttonPressed;

  // ======== Controle de temporização dos LEDs e Relé ========
  // LED verde e relé devem desligar juntos (sincronizados)
  if (greenOffAt > 0 && now >= greenOffAt) {
    digitalWrite(LED_GREEN, LOW);
    greenOffAt = 0;
    // Desliga o relé junto com o LED verde (mesmo tempo)
    relayForceOff();
    lcdShowIdle();
  }
  if (redOffAt > 0 && now >= redOffAt) {
    digitalWrite(LED_RED, LOW);
    redOffAt = 0;
    lcdShowIdle();
  }
  
  // Verificação de segurança: se o relé ainda estiver ativo mas o LED verde já desligou
  // (não deveria acontecer, mas garante sincronização)
  if (relayOffAt > 0 && greenOffAt == 0 && digitalRead(LED_GREEN) == LOW) {
    relayForceOff();
  }

  // ======== Leitura do RFID ========
  bool present = rfid.PICC_IsNewCardPresent();

  if (present && !cardPresent) {
    if (!rfid.PICC_ReadCardSerial()) return;

    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    Serial.print("Cartao detectado: ");
    Serial.println(uid);

    lastSeenUID = uid;
    cardPresent = true;

    if (isAuthorized(uid)) {
      Serial.println("APROVADO");
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      greenOffAt = now + GREEN_TIME_MS;
      redOffAt = 0;
      relayWrite(true);
      relayOffAt = now + GREEN_TIME_MS;
      buzzApproved();
      lcdShowApproved();
    } else {
      Serial.println("NEGADO");
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      redOffAt = now + RED_TIME_MS;
      greenOffAt = 0;
      relayWrite(false);
      relayOffAt = 0;
      buzzDenied();
      lcdShowDenied();
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  else if (!present && cardPresent) {
    Serial.print("Cartao removido: ");
    Serial.println(lastSeenUID);
    lastSeenUID = "";
    cardPresent = false;
    // Não desliga o relé aqui - ele será desligado junto com o LED verde quando o timer expirar
    // Isso garante que o relé e o LED verde sempre desliguem juntos
  }

  delay(50);
}
