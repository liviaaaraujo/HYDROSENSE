#include <WiFi.h>
#include <WebServer.h>

// =================================================================
// --- CONFIGURAÇÕES DO USUÁRIO ---
// =================================================================

// 1. DADOS DA SUA REDE WI-FI
const char* ssid = "lili";
const char* password = "12345678";

// 2. PINOS DE CONEXÃO
#define SENSOR_PIN 34  // Pino Analógico onde o sensor de umidade está conectado (A0 do sensor). PINO 34 É SEGURO PARA USAR COM WI-FI.
#define RELE_PIN 14    // Pino Digital conectado ao pino de sinal (IN) do relé.

// 3. CALIBRAÇÃO DO SENSOR (Opcional, mas recomendado para maior precisão)
// Meça o valor com o sensor no ar (SECO) e totalmente na água (MOLHADO) e substitua os valores abaixo.
const int SENSOR_SECO = 4095;    // Valor lido com o sensor totalmente seco (padrão ESP32: 4095)
const int SENSOR_MOLHADO = 1800; // Valor lido com o sensor totalmente submerso em água (valor de exemplo)

// =================================================================
// --- VARIÁVEIS GLOBAIS DO SISTEMA ---
// =================================================================

WebServer server(80);
float umidadeMinima = 40.0; // Umidade mínima para acionar o relé (padrão: 40%). Pode ser alterado pela interface web.

// Variáveis para guardar o estado atual do sistema
int umidadeAtual = 0;
float umidadeMediaAtual = 0.0;
int releEstadoAtual = 0; // 0 = desligado, 1 = ligado

// Temporizador para executar a lógica principal sem usar delay()
unsigned long previousMillis = 0;
const long intervalo = 5000; // Executa a lógica a cada 5 segundos

// Buffer para Média Móvel (suaviza a leitura do sensor)
const int N_LEITURAS = 10;
float umidBuffer[N_LEITURAS] = {0};
int bufferIndex = 0;
int totalLeituras = 0;

// =================================================================
// --- PÁGINA WEB (HTML, CSS, JavaScript) ---
// =================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Controle de Irrigação</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap');
    * { margin: 0; padding: 0; box-sizing: border-box; user-select: none; }
    body {
      font-family: 'Roboto', sans-serif;
      background: linear-gradient(135deg, #001f21, #003133, #001c1d);
      background-size: 400% 400%;
      animation: gradientBG 20s ease infinite;
      color: #e4e4e4;
      min-height: 100vh;
      display: flex; flex-direction: column; justify-content: center; align-items: center;
      padding: 20px; text-align: center;
    }
    @keyframes gradientBG { 0%{background-position:0% 50%} 50%{background-position:100% 50%} 100%{background-position:0% 50%} }
    @keyframes fadeInUp { from { opacity: 0; transform: translateY(30px); } to { opacity: 1; transform: translateY(0); } }
    h1 { font-size: clamp(2rem, 6vw, 2.8rem); margin-bottom: 10px; color: #ffffff; text-shadow: 0 0 10px rgba(255,255,255,0.2); letter-spacing: 2px; animation: fadeInUp 0.6s ease-out both; }
    .gauges { display: flex; justify-content: center; align-items: flex-start; max-width: 600px; width: 100%; margin-top: 30px; }
    .gauge { position: relative; width: 180px; height: 180px; user-select: none; display: flex; flex-direction: column; align-items: center; }
    .gauge svg { width: 180px; height: 180px; border-radius: 50%; transform: rotate(-90deg); box-shadow: 0 0 15px #0b9dac88, inset 0 0 40px #0b9dac55; background: radial-gradient(circle at center, #0b9dac22, transparent 70%); z-index: 0; overflow: visible; }
    .circle-bg { fill: none; stroke: #084e55; stroke-width: 4; }
    .circle { fill: none; stroke: #0b9dac; stroke-width: 4; stroke-linecap: round; transition: stroke-dasharray 0.6s ease; z-index: 1; }
    .value { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 2rem; font-weight: 700; color: #0b9dac; display: flex; align-items: baseline; gap: 3px; user-select: none; line-height: 1; z-index: 2; pointer-events: none; }
    .unit { font-size: 0.9rem; font-weight: 500; color: #0b9dacbb; user-select: none; }
    .label { margin-top: 15px; font-size: 1.3rem; font-weight: 700; color: #e4e4e4; letter-spacing: 1px; user-select: none; z-index: 3; }
    .medias { margin-top: 40px; display: flex; gap: 60px; justify-content: center; flex-wrap: wrap; }
    .media-item { font-size: 1.4rem; font-weight: 700; user-select: none; display: flex; align-items: baseline; gap: 6px; }
    .media-item .label { color: #0B9DAC; font-weight: 700; }
    .media-item .valor { color: white; font-weight: 900; }
    .media-item .unit { color: white; font-weight: 500; font-size: 1.1rem; }
    .controles { background-color: rgba(0, 0, 0, 0.2); padding: 20px; border-radius: 12px; margin-top: 20px; display: flex; flex-direction: column; gap: 15px; align-items: center; min-width: 300px; animation: fadeInUp 0.8s ease-out 0.2s both; }
    .controle-item { display: flex; align-items: center; gap: 10px; font-size: 1.1rem;}
    .controle-item label { color: #0B9DAC; font-weight: 700; }
    .controle-item input { font-family: 'Roboto', sans-serif; font-size: 1.1rem; padding: 8px; width: 80px; background-color: #003133; border: 1px solid #084e55; color: white; border-radius: 5px; text-align: center; }
    .controle-item button { font-family: 'Roboto', sans-serif; font-size: 1rem; padding: 8px 15px; background-color: #0b9dac; color: white; border: none; border-radius: 5px; cursor: pointer; transition: background-color 0.3s ease; }
    .controle-item button:hover { background-color: #087a85; }
    .status-rele { font-weight: 700; }
    #statusRele.ligado { color: #23d18b; text-shadow: 0 0 8px #23d18b88; }
    #statusRele.desligado { color: #ff5252; text-shadow: 0 0 8px #ff525288; }
    #confirmMsg { color: #23d18b; font-weight: 700; margin-top: 10px; opacity: 0; transition: opacity 0.5s ease; user-select: none; height: 1.2em; }
    #confirmMsg.show { opacity: 1; }
    footer { margin-top: 40px; font-size: 0.9rem; color: #9ca3af; opacity: 0.7; animation: fadeInUp 0.8s ease-out 0.6s both; }
    @media (max-width: 600px) {
        .gauges { flex-wrap: wrap; gap: 40px; } .gauge, .gauge svg { width: 140px; height: 140px; }
        .value { font-size: 1.6rem; } .unit { font-size: 0.8rem; } .label { font-size: 1.1rem; margin-top: 10px; }
        .medias { gap: 30px; } .media-item { font-size: 1.2rem; } .media-item .unit { font-size: 1rem; } .controles { min-width: 90%; }
    }
  </style>
</head>
<body>
  <h1>Controle de Irrigação</h1>
  <div class="gauges">
    <div class="gauge">
      <svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"/><path class="circle" id="circleUmid" stroke-dasharray="0, 100" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"/></svg>
      <div class="value" id="umidValue">--<span class="unit">%</span></div><div class="label">Umidade do Solo</div>
    </div>
  </div>
  <div class="medias">
    <div class="media-item"><span class="label">Umidade Média = </span><span class="valor" id="umidMediaValue">--</span><span class="unit">%</span></div>
  </div>
  <div class="controles">
    <div class="controle-item">
      <label for="umidMinInput">Umid. Mínima (%):</label>
      <input type="number" id="umidMinInput" step="1" value="40">
      <button onclick="definirUmidadeMin(event)">Definir</button>
    </div>
    <div id="confirmMsg">Umidade mínima definida com sucesso!</div>
    <div class="controle-item">
      <label>Status da Irrigação:</label>
      <span id="statusRele" class="status-rele desligado">PARADA</span>
    </div>
  </div>
  <footer>Eletrônica Ômega | E-book IoT com ESP32</footer>
  <script>
    async function definirUmidadeMin(event) {
      const botao = event.target;
      botao.disabled = true;
      const umidMin = document.getElementById('umidMinInput').value;
      if (umidMin === '' || isNaN(parseFloat(umidMin))) {
        alert('Por favor, insira um valor numérico válido.');
        botao.disabled = false; return;
      }
      try {
        const res = await fetch(`/set?umidade=${umidMin}`);
        if (!res.ok) throw new Error('Erro no servidor');
      } catch (error) {
        alert('Erro ao definir umidade no servidor.');
        botao.disabled = false; return;
      }
      mostrarConfirmacao();
      botao.disabled = false;
    }
    function mostrarConfirmacao() {
      const msg = document.getElementById('confirmMsg');
      msg.classList.add('show');
      setTimeout(() => { msg.classList.remove('show'); }, 2500);
    }
    async function atualizarDados() {
      try {
        const res = await fetch('/dados');
        const data = await res.json();
        const umid = parseFloat(data.umidade);
        document.getElementById('umidValue').innerHTML = umid.toFixed(0) + '<span class="unit">%</span>';
        const percentUmid = Math.min(umid, 100);
        document.getElementById('circleUmid').setAttribute('stroke-dasharray', `${percentUmid}, 100`);
        const umidMedia = parseFloat(data.umidMedia);
        document.getElementById('umidMediaValue').textContent = isNaN(umidMedia) ? '--' : umidMedia.toFixed(0);
        const umidMin = parseFloat(data.umidadeMinima);
        const inputUmidMin = document.getElementById('umidMinInput');
        if(document.activeElement !== inputUmidMin) { inputUmidMin.value = umidMin.toFixed(0); }
        const statusReleEl = document.getElementById('statusRele');
        if (data.releEstado === 1) {
          statusReleEl.textContent = 'IRRIGANDO';
          statusReleEl.className = 'status-rele ligado';
        } else {
          statusReleEl.textContent = 'PARADA';
          statusReleEl.className = 'status-rele desligado';
        }
      } catch (error) { console.error('Erro ao atualizar dados:', error); }
    }
    setInterval(atualizarDados, 2000);
    window.onload = atualizarDados;
  </script>
</body>
</html>
)rawliteral";


// =================================================================
// --- FUNÇÕES DO SERVIDOR WEB ---
// =================================================================

// Função que serve a página HTML principal
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

// Função que envia os dados atuais do sistema em formato JSON
void handleDados() {
  String json = "{";
  json += "\"umidade\":" + String(umidadeAtual);
  json += ",\"umidMedia\":" + String(umidadeMediaAtual, 1);
  json += ",\"umidadeMinima\":" + String(umidadeMinima, 1);
  json += ",\"releEstado\":" + String(releEstadoAtual);
  json += "}";
  server.send(200, "application/json", json);
}

// Função que recebe e define o novo valor de umidade mínima
void handleSetUmidade() {
  if (server.hasArg("umidade")) {
    umidadeMinima = server.arg("umidade").toFloat();
    server.send(200, "text/plain", "OK");
    Serial.print("Nova umidade minima definida pela web: ");
    Serial.println(umidadeMinima);
  } else {
    server.send(400, "text/plain", "Argumento 'umidade' ausente");
  }
}

// =================================================================
// --- FUNÇÃO PRINCIPAL DE CONTROLE ---
// =================================================================

void logicaDeIrrigacao() {
  // 1. Lê o valor do sensor
  int valorAnalogico = analogRead(SENSOR_PIN);
  umidadeAtual = map(valorAnalogico, SENSOR_SECO, SENSOR_MOLHADO, 0, 100);
  umidadeAtual = constrain(umidadeAtual, 0, 100);

  // 2. Decide se liga ou desliga o relé
  if (umidadeAtual <= umidadeMinima) {
    digitalWrite(RELE_PIN, LOW); // Ativa relé (nível baixo geralmente liga)
  } else {
    digitalWrite(RELE_PIN, HIGH); // Desativa relé
  }
  releEstadoAtual = (digitalRead(RELE_PIN) == LOW) ? 1 : 0;

  // 3. Atualiza o buffer para cálculo da média móvel
  umidBuffer[bufferIndex] = umidadeAtual;
  bufferIndex = (bufferIndex + 1) % N_LEITURAS;
  if (totalLeituras < N_LEITURAS) totalLeituras++;
  
  float somaUmid = 0.0;
  for (int i = 0; i < totalLeituras; i++) {
    somaUmid += umidBuffer[i];
  }
  umidadeMediaAtual = (totalLeituras > 0) ? (somaUmid / totalLeituras) : 0.0;

  // 4. Imprime status no Monitor Serial para debug (opcional)
  Serial.print("Umidade Atual: " + String(umidadeAtual) + "%");
  Serial.print(" | Mínima Definida: " + String(umidadeMinima, 0) + "%");
  Serial.println(" | Relé: " + String(releEstadoAtual == 1 ? "LIGADO" : "DESLIGADO"));
}


// =================================================================
// --- SETUP ---
// =================================================================
void setup() {
  Serial.begin(115200);
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, HIGH); // Garante que o relé comece desligado

  // Inicia a tentativa de conexão ao Wi-Fi de forma não-bloqueante
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nTentando conectar ao Wi-Fi");

  byte tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  // Verifica o status da conexão e imprime a mensagem apropriada
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado! Endereço IP: " + WiFi.localIP().toString());
    
    // Inicia o servidor web
    server.on("/", handleRoot);
    server.on("/dados", handleDados);
    server.on("/set", HTTP_GET, handleSetUmidade);
    server.begin();
    Serial.println("Servidor web iniciado. O modo autônomo também está ativo.");
  } else {
    Serial.println("\nNão foi possível conectar ao Wi-Fi. Iniciando em modo autônomo.");
  }
}


// =================================================================
// --- LOOP ---
// =================================================================
void loop() {
  // Se o Wi-Fi estiver conectado, lida com as requisições dos clientes
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }

  // Executa a lógica de irrigação em intervalos de tempo, independentemente do Wi-Fi
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    previousMillis = currentMillis; 
    logicaDeIrrigacao();
  }
}