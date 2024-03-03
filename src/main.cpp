#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
const char* ssid = "ZEUS_2G";
const char* password = "tito2000";

ESP8266WebServer server(80);

void handleRoot();
String getPage() ;
void handleData();


void setup() {
  Serial.begin(115200);

  // Conecte-se à rede Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Roteamento de URLs
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_POST, handleData);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  // Envie a página HTML para o cliente
  server.send(200, "text/html", getPage());
}

void handleData() {

  // Manipule os dados recebidos do cliente
  StaticJsonDocument<200> doc;
  deserializeJson(doc, server.arg("plain"));

  String action = doc["action"];
  String programa = doc["programa"];
  String nivel = doc["nivel"];

  Serial.print("Ação: ");
  Serial.println(action);
  Serial.print("Programa: ");
  Serial.println(programa);
  Serial.print("Nível: ");
  Serial.println(nivel);

  server.send(200, "text/plain", "Dados recebidos pelo ESP8266.");
}

String getPage() {
  // Crie a página HTML
  const char* webpage = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Malob: Tanquinho Esperto</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        .container {
            max-width: 90%;
            padding: 20px;
            background-color: #ffffff;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }
        h1 {
            text-align: center;
            color: #333;
            margin-top: 10px;
            margin-bottom: 20px;
            font-size: 20px;
        }
        .label {
            font-weight: bold;
            margin-bottom: 5px;
            margin-top: 15px;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
            gap: 10px;
        }
        .button {
            background-color: green;
            color: white;
            border: none;
            padding: 10px 20px;
            cursor: pointer;
            border-radius: 5px;
            text-align: center;
        }
        .button.selected {
            background-color: blue;
        }

        #toggleButton {
            margin: 20px auto 0;
            display: block;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Malob: Tanquinho Esperto</h1>
        <div class="label">Programas:</div>
        <div class="grid" id="ciclo">
            <button class="button" onclick="selectProgramCiclo(this, 'LONGO')">LONGO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'MÉDIO')">MÉDIO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'CURTO')">CURTO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'MOLHO')">MOLHO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'ESVAZIAR')">ESVAZIAR</button>
            <button class="button" onclick="selectProgramCiclo(this, 'ENCHER')">ENCHER</button>
        </div>
        <div class="label">Nível de Água:</div>
        <div class="grid" id="nivel">
            <button class="button" onclick="selectNivelAgua(this, 'ALTO')">ALTO</button>
            <button class="button" onclick="selectNivelAgua(this, 'MÉDIO')">MÉDIO</button>
            <button class="button" onclick="selectNivelAgua(this, 'BAIXO')">BAIXO</button>
        </div>
        <button class="button" id="toggleButton" onclick="checkSelectedButtons()">LIGAR</button>
    </div>
    <script>
        let selectedProgramCiclo = "";
        let selectedWaterLevel = "";
        let action = "";
        const button = document.getElementById("toggleButton");
        button.style.backgroundColor = "green";
        function selectProgramCiclo(button, programa) {
            selectedProgramCiclo = programa;
            const buttons = document.querySelectorAll(`#${'ciclo'} .button`);
            buttons.forEach(btn => {
                if (btn !== button) {
                    btn.classList.remove('selected');
                   // button.style.backgroundColor = "green";
                }
            });
            button.classList.add('selected');
        }
        function selectNivelAgua(button, nivel) {
            selectedWaterLevel = nivel;
            const buttons = document.querySelectorAll(`#${'nivel'} .button`);
            buttons.forEach(btn => {
                if (btn !== button) {
                    btn.classList.remove('selected');
                }
            });
            button.classList.add('selected');
        }
        function togglePowerButton() {
            const button = document.getElementById("toggleButton");
            let action = "";
            let programa = "";
            let nivel = "";
            if (button.style.backgroundColor === "green") {
                button.style.backgroundColor = "red";
                button.innerText = "DESLIGAR";
                action = "ligar";
                programa = selectedProgramCiclo;
                nivel = selectedWaterLevel;
            } else {
                button.style.backgroundColor = "green";
                button.innerText = "LIGAR";
                action = "desligar";
            }
            sendData({ action: action, programa: selectedProgramCiclo, nivel: selectedWaterLevel });
        }
        function checkSelectedButtons() {
            const selectedCiclo = document.querySelectorAll(`#${'ciclo'} .button.selected`);
            const selectedNivel = document.querySelectorAll(`#${'nivel'} .button.selected`);
            if (selectedCiclo.length !== 1 || selectedNivel.length !== 1) {
                alert("Por favor, selecione um programa e um nível de água antes de ligar.");
            } else {
                togglePowerButton();
            }
        }
        function forceButtonStyle() {
            const button = document.getElementById("toggleButton");
            button.style.backgroundColor = "green";
        }
       function sendData(data) {
    const xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                console.log("Dados enviados com sucesso.");
            } else {
                console.error("Erro ao enviar dados:", xhr.statusText);
            }
        }
    };
    xhr.open("POST", "/data", true); // Alterado o caminho para "/data"
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(JSON.stringify(data));
}

    </script>
</body>
</html>
)=====";
  return webpage;
}
