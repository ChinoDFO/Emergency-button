/*
 * =====================================================
 *   SISTEMA DE ALARMA COMUNITARIA - NEGOCIOS
 *   v4 — Portal WiFi con diseño profesional
 * =====================================================
 *
 * Librerías necesarias (instalar desde el Library Manager):
 *   - WiFiManager  by tzapu
 *   - HTTPClient   (incluida con el core de ESP32)
 *
 * Placa: ESP32 Dev Module
 *
 * PINES:
 *   GPIO14 → Botón de emergencia (otro extremo a GND)
 *   GPIO5  → LED VERDE (con resistencia 220Ω a GND)
 *   GPIO2  → LED ROJO  (con resistencia 220Ω a GND)
 * =====================================================
 */

#include <WiFiManager.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Preferences.h>

// ─────────────────────────────────────────────
//  CONFIGURACIÓN
// ─────────────────────────────────────────────
String NOMBRE_NEGOCIO = "Negocio";
String DIRECCION = "Dirección";
const char* NTFY_TOPIC = "alertas-negocios-j7x92k-SNB";

// ─────────────────────────────────────────────
//  PINES
// ─────────────────────────────────────────────
const int PIN_BOTON = 14;
const int PIN_LED_VERDE = 5;
const int PIN_LED_ROJO = 2;

// ─────────────────────────────────────────────
//  VARIABLES DE CONTROL
// ─────────────────────────────────────────────
Preferences preferences;
WiFiMulti wifiMulti;

unsigned long ultimaAlerta = 0;
const unsigned long COOLDOWN = 10000;

unsigned long ultimoChequeoWifi = 0;
const unsigned long INTERVALO_WIFI = 3000;

bool botonPresionadoAntes = false;

unsigned long inicioSinWifi = 0;
const unsigned long TIEMPO_MAX_SIN_WIFI = 120000;

String ssid1, pass1, ssid2, pass2;

// ─────────────────────────────────────────────
//   HTML/CSS DEL PORTAL PERSONALIZADO (Sobreescritura Real)
// ─────────────────────────────────────────────
const char* PORTAL_CSS = R"(
<style>
  /* Base del cuerpo */
  body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    background-color: #f0f2f5 !important;
    color: #1a1a1a !important;
    margin: 0;
    padding: 20px;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 100vh;
  }

  /* El contenedor principal que genera WiFiManager */
  div {
    background: white;
    border-radius: 16px;
    box-shadow: 0 4px 24px rgba(0,0,0,0.10);
    padding: 32px 28px;
    width: 100%;
    max-width: 400px;
    box-sizing: border-box;
  }

  /* Títulos principales */
  h1, h2 {
    font-size: 1.4rem !important;
    font-weight: 700 !important;
    color: #1a1a1a !important;
    text-align: center;
    margin-bottom: 20px !important;
  }

  /* Inputs de texto, password y los campos personalizados */
  input[type="text"], 
  input[type="password"], 
  input[type="number"] {
    width: 100% !important;
    padding: 12px 14px !important;
    border: 1.5px solid #e0e0e0 !important;
    border-radius: 10px !important;
    font-size: 0.95rem !important;
    color: #1a1a1a !important;
    background: #fafafa !important;
    margin-bottom: 14px !important;
    box-sizing: border-box;
    outline: none;
    transition: border-color 0.2s;
  }

  input:focus {
    border-color: #c0392b !important;
    background: white !important;
  }

  /* Botones del menú y formularios */
  button, 
  input[type="submit"], 
  a.btn {
    display: block;
    width: 100% !important;
    padding: 14px !important;
    background: #c0392b !important;
    color: white !important;
    border: none !important;
    border-radius: 10px !important;
    font-size: 1rem !important;
    font-weight: 600 !important;
    cursor: pointer !important;
    margin-top: 10px !important;
    text-decoration: none;
    text-align: center;
    box-sizing: border-box;
    transition: background 0.2s;
  }

  button:hover, input[type="submit"]:hover {
    background: #a93226 !important;
  }

  button:active, input[type="submit"]:active {
    background: #922b21 !important;
  }

  /* Enlaces de las listas de redes (SSID) */
  div a {
    color: #1a1a1a !important;
    font-weight: 600;
    text-decoration: none;
  }
  
  /* Separadores */
  hr {
    border: none;
    border-top: 1px solid #eee;
    margin: 20px 0;
  }
</style>
)";

// ─────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SISTEMA DE ALARMA COMUNITARIA v4 ===");

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);

  setLEDs(false);

  // ── Reset si el botón está presionado al encender ──
  Serial.println("Mantén presionado el botón para reconfigurar...");
  delay(3000);
  if (digitalRead(PIN_BOTON) == LOW) {
    Serial.println("¡Reset activado!");
    for (int i = 0; i < 6; i++) {
      digitalWrite(PIN_LED_ROJO, i % 2 == 0 ? HIGH : LOW);
      digitalWrite(PIN_LED_VERDE, i % 2 == 0 ? LOW : HIGH);
      delay(200);
    }
    WiFiManager wm;
    wm.resetSettings();
    preferences.begin("negocio", false);
    preferences.clear();
    preferences.end();
    Serial.println("Configuración borrada. Reiniciando...");
    delay(1000);
    ESP.restart();
  }

  cargarConfiguracion();
  conectarWiFi();
  cargarConfiguracion();

  if (ssid1.length() > 0) wifiMulti.addAP(ssid1.c_str(), pass1.c_str());
  if (ssid2.length() > 0) wifiMulti.addAP(ssid2.c_str(), pass2.c_str());

  setLEDs(true);

  Serial.println("✓ Sistema listo.");
  Serial.print("  Negocio   : ");
  Serial.println(NOMBRE_NEGOCIO);
  Serial.print("  Dirección : ");
  Serial.println(DIRECCION);
  Serial.print("  Canal     : ");
  Serial.println(NTFY_TOPIC);
  Serial.print("  Red 1     : ");
  Serial.println(ssid1);
  Serial.print("  Red 2     : ");
  Serial.println(ssid2.length() > 0 ? ssid2 : "(no configurada)");

  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED_VERDE, LOW);
    delay(150);
    digitalWrite(PIN_LED_VERDE, HIGH);
    delay(150);
  }
}

// ─────────────────────────────────────────────
//  LOOP PRINCIPAL
// ─────────────────────────────────────────────
void loop() {
  unsigned long ahora = millis();

  if (ahora - ultimoChequeoWifi >= INTERVALO_WIFI) {
    ultimoChequeoWifi = ahora;

    if (WiFi.status() != WL_CONNECTED) {
      setLEDs(false);

      if (inicioSinWifi == 0) {
        inicioSinWifi = ahora;
        Serial.println("⚠ WiFi perdido. Buscando red...");
      }

      if (ahora - inicioSinWifi >= TIEMPO_MAX_SIN_WIFI) {
        Serial.println("⏱ Sin WiFi 2 min. Reiniciando...");
        delay(500);
        ESP.restart();
      }

      if (wifiMulti.run(5000) == WL_CONNECTED) {
        Serial.print("✓ Reconectado a: ");
        Serial.println(WiFi.SSID());
        inicioSinWifi = 0;
        setLEDs(true);
      } else {
        unsigned long seg = (ahora - inicioSinWifi) / 1000;
        Serial.print("✗ Sin red. Tiempo: ");
        Serial.print(seg);
        Serial.println("s");
      }
    } else {
      inicioSinWifi = 0;
      setLEDs(true);
    }
  }

  bool botonPresionado = (digitalRead(PIN_BOTON) == LOW);

  if (botonPresionado && !botonPresionadoAntes) {
    if (ahora - ultimaAlerta >= COOLDOWN) {
      ultimaAlerta = ahora;
      Serial.println("🚨 ¡Botón presionado! Enviando alerta...");
      enviarAlerta();
    } else {
      unsigned long restante = (COOLDOWN - (ahora - ultimaAlerta)) / 1000;
      Serial.print("⏳ Cooldown. Espera ");
      Serial.print(restante);
      Serial.println("s");
      parpadearLED(PIN_LED_ROJO, 2, 100);
    }
  }

  botonPresionadoAntes = botonPresionado;
  delay(50);
}

// ─────────────────────────────────────────────
//  CONTROL DE LEDs
// ─────────────────────────────────────────────
void setLEDs(bool wifiConectado) {
  digitalWrite(PIN_LED_VERDE, wifiConectado ? HIGH : LOW);
  digitalWrite(PIN_LED_ROJO, wifiConectado ? LOW : HIGH);
}

// ─────────────────────────────────────────────
//  ENVIAR ALERTA A NTFY
// ─────────────────────────────────────────────
void enviarAlerta() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("✗ Sin WiFi.");
    parpadearLED(PIN_LED_ROJO, 5, 100);
    setLEDs(false);
    return;
  }

  HTTPClient http;
  String url = "https://ntfy.sh/";
  url += NTFY_TOPIC;

  String mensaje = "🚨 EMERGENCIA\n";
  mensaje += NOMBRE_NEGOCIO;
  mensaje += "\n📍 ";
  mensaje += DIRECCION;

  for (int i = 0; i < 5; i++) {
    http.begin(url);
    http.addHeader("Content-Type", "text/plain; charset=utf-8");
    http.addHeader("Title", "⚠️ ALERTA DE ROBO");
    http.addHeader("Priority", "urgent");
    http.addHeader("Tags", "rotating_light");
    http.addHeader("Sound", "ding");
    http.addHeader("Persist", "1");

    int httpCode = http.POST(mensaje);
    http.end();

    if (httpCode == 200 || httpCode == 201) {
      Serial.print("✓ Alerta #");
      Serial.print(i + 1);
      Serial.println(" enviada.");
      parpadearLED(PIN_LED_VERDE, 2, 100);
    } else {
      Serial.print("✗ Error alerta #");
      Serial.print(i + 1);
      Serial.print(" HTTP: ");
      Serial.println(httpCode);
      parpadearLED(PIN_LED_ROJO, 2, 200);
    }

    if (i < 4) delay(5000);
  }

  setLEDs(true);
}

// ─────────────────────────────────────────────
//  CONECTAR WIFI — Portal con diseño personalizado
// ─────────────────────────────────────────────
void conectarWiFi() {
  WiFiManager wm;

  // Inyectar CSS personalizado en el portal
  wm.setCustomHeadElement(PORTAL_CSS);

  // Título y descripción del portal
  wm.setTitle("Alarma Comunitaria");

  WiFiManagerParameter param_nombre("nombre", "Nombre del negocio", NOMBRE_NEGOCIO.c_str(), 60);
  WiFiManagerParameter param_dir("direccion", "Dirección (calles)", DIRECCION.c_str(), 80);
  WiFiManagerParameter param_ssid2("ssid2", "Red de respaldo (opcional)", ssid2.c_str(), 40);
  WiFiManagerParameter param_pass2("pass2", "Contraseña red de respaldo", pass2.c_str(), 40);

  wm.addParameter(&param_nombre);
  wm.addParameter(&param_dir);
  wm.addParameter(&param_ssid2);
  wm.addParameter(&param_pass2);

  wm.setAPCallback([](WiFiManager* wm) {
    Serial.println("★ Portal activo — ALARMA-CONFIG");
    Serial.println("  Abre 192.168.4.1 en tu navegador");
  });

  bool seGuardaronParametros = false;
  wm.setSaveParamsCallback([&]() {
    seGuardaronParametros = true;
  });

  bool conectado = wm.autoConnect("ALARMA-CONFIG");

  if (!conectado) {
    Serial.println("✗ No se pudo conectar. Reiniciando...");
    parpadearLED(PIN_LED_ROJO, 5, 200);
    delay(1000);
    ESP.restart();
  }

  if (seGuardaronParametros) {
    guardarConfiguracion(
      param_nombre.getValue(),
      param_dir.getValue(),
      WiFi.SSID(),
      WiFi.psk(),
      param_ssid2.getValue(),
      param_pass2.getValue());
  }

  Serial.println("✓ WiFi conectado.");
  Serial.print("  Red activa: ");
  Serial.println(WiFi.SSID());
  Serial.print("  IP: ");
  Serial.println(WiFi.localIP());
}

// ─────────────────────────────────────────────
//  GUARDAR CONFIGURACIÓN
// ─────────────────────────────────────────────
void guardarConfiguracion(String nombre, String dir,
                          String s1, String p1,
                          String s2, String p2) {
  preferences.begin("negocio", false);
  preferences.putString("nombre", nombre);
  preferences.putString("dir", dir);
  preferences.putString("ssid1", s1);
  preferences.putString("pass1", p1);
  preferences.putString("ssid2", s2);
  preferences.putString("pass2", p2);
  preferences.end();
  Serial.println("✓ Configuración guardada.");
}

// ─────────────────────────────────────────────
//  CARGAR CONFIGURACIÓN
// ─────────────────────────────────────────────
void cargarConfiguracion() {
  preferences.begin("negocio", true);
  NOMBRE_NEGOCIO = preferences.getString("nombre", NOMBRE_NEGOCIO);
  DIRECCION = preferences.getString("dir", DIRECCION);
  ssid1 = preferences.getString("ssid1", "");
  pass1 = preferences.getString("pass1", "");
  ssid2 = preferences.getString("ssid2", "");
  pass2 = preferences.getString("pass2", "");
  preferences.end();
}

// ─────────────────────────────────────────────
//  PARPADEAR LED
// ─────────────────────────────────────────────
void parpadearLED(int pin, int veces, int intervalo) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(pin, HIGH);
    delay(intervalo);
    digitalWrite(pin, LOW);
    delay(intervalo);
  }
}
