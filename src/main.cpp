#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
///////////////////////
#define LIGA LOW
#define DESLIGA HIGH

#define NIVEL_ALTO D0
#define NIVEL_MEDIO D1
#define NIVEL_BAIXO D2

#define MOTOR D5
#define DRENO D6
#define AGUA D7
#define BUZZER D8
boolean P0[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
boolean P1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
boolean P2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
boolean P3[4] = {0, 0, 0, 0};

String msg = "Esta é a mensagem que eu quero postar";
String ESTADO;
String programa;
String nivel;
boolean novoDado = true;
bool dreno = false;
unsigned long sendDataPrevMillis = 0;
void SeMexe(String ciclo);
void buzina(void);
void desligar(void);
void sendData(const char *data);
void handleMsg();

////////////////////////////
const char *ssid = "ZEUS_2G";
const char *password = "tito2000";

ESP8266WebServer server(80);

void handleRoot();
String getPage();
void handleData();

void setup()
{
    Serial.begin(115200);

    // Conecte-se à rede Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Conectando ao WiFi...");
    }

    Serial.println("Conectado ao WiFi!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());

    // Roteamento de URLs
    server.on("/", HTTP_GET, handleRoot);
    server.on("/msg", HTTP_GET, handleMsg);
    server.on("/data", HTTP_POST, handleData);

    server.begin();
    Serial.println("Servidor HTTP iniciado.");

    ///////////// TANQUE SETUP //////////////////

    pinMode(BUZZER, OUTPUT);
    pinMode(MOTOR, OUTPUT);
    digitalWrite(MOTOR, DESLIGA);
    pinMode(DRENO, OUTPUT);
    digitalWrite(DRENO, DESLIGA);
    pinMode(AGUA, OUTPUT);
    digitalWrite(AGUA, DESLIGA);
    pinMode(NIVEL_ALTO, INPUT);
    pinMode(NIVEL_MEDIO, INPUT);
    pinMode(NIVEL_BAIXO, INPUT);
    pinMode(BUZZER, OUTPUT);

    ///////////////////////////
}

void loop()
{
    server.handleClient();
    if (novoDado)
    {
        msg = ESTADO;
        if (ESTADO == "LIGAR")
        {
            // SeMexe(programa);
            Serial.println(ESTADO);
            delay(1000);
        }
        else if (ESTADO == "DESLIGAR")
        {
            Serial.println(ESTADO);
            delay(1000);
            desligar(); // Chama a função desligar() caso contrário
        }
        else if (ESTADO == "PAUSAR")
        {
            Serial.println(ESTADO);
            delay(1000);
        }
        else if (ESTADO == "REINICIAR")
        {
            Serial.println(ESTADO);
            delay(1000);
        }
    }
    novoDado = false;
}

void handleRoot()
{
    // Envie a página HTML para o cliente
    server.send(200, "text/html", getPage());
}

void handleData()
{

    // Manipule os dados recebidos do cliente
    // StaticJsonDocument<200> doc;
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));

    String action = doc["action"];
    String programa = doc["programa"];
    String nivel = doc["nivel"];
    ESTADO = action;

    // Serial.print("Ação: ");
    // Serial.println(action);
    // Serial.print("Programa: ");
    // Serial.println(programa);
    // Serial.print("Nível: ");
    // Serial.println(nivel);

    server.send(200, "text/plain", "Dados recebidos pelo ESP8266.");
    novoDado = true;
}
void handleMsg()
{
    String msg = "Esta é a mensagem número " + String(random(10)); // gera uma mensagem com um número aleatório entre 0 e 9
    Serial.println(msg);
    server.send(200, "text/plain", msg);                           
}

String getPage()
{
    // Crie a página HTML
    const char *webpage = R"=====(<!DOCTYPE html>
<html lang="pt-BR">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Três Containers com Botões</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 10% 15%;
            padding: 0;
        }

        h1 {
            font-size: 24px;
            /* Altere este valor conforme necessário */
        }

        .container {
            margin: 10px;
            padding: 10px;
            background-color: #f2f2f2;
            border-radius: 5px;
        }

        .container label {
            font-weight: bold;
            display: block;
            margin-bottom: 10px;
        }

        .button {
            background-color: green;
            color: white;
            border: none;
            padding: 10px 20px;
            cursor: pointer;
            border-radius: 5px;
            text-align: center;
            display: block;
            margin: 5px auto;
        }

        .button:hover {
            background-color: #45a049;
        }

        .button.selected {
            background-color: blue;
        }

        .mensagem {
            width: calc(100% - 20px);
            height: 100px;
            resize: none;
            margin-top: 10px;
        }

        @media screen and (max-width: 600px) {
            .container {
                margin: 5px;
                padding: 5px;
            }

            .button {
                width: 100%;
            }
        }
    </style>
</head>

<body>

    <div class="container">
        <h1>Malob: TANQUINHO ESPERTO</h1>
        <label>PROGRAMAS:</label>
        <div class="grid" id="ciclo">
            <button class="button" onclick="selectProgramCiclo(this, 'LONGO')">LONGO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'MÉDIO')">MÉDIO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'CURTO')">CURTO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'MOLHO')">MOLHO</button>
            <button class="button" onclick="selectProgramCiclo(this, 'ESVAZIAR')">ESVAZIAR</button>
        </div>

        <div class="container">
            <label>NÍVEL DA ÁGUA:</label>
            <div class="grid" id="nivel">
                <button class="button" onclick="selectNivelAgua(this, 'ALTO')">ALTO</button>
                <button class="button" onclick="selectNivelAgua(this, 'MÉDIO')">MÉDIO</button>
                <button class="button" onclick="selectNivelAgua(this, 'BAIXO')">BAIXO</button>
            </div>
        </div>

        <div class="container">
            <label>ACIONAMENTO:</label>
            <button class="button" id="toggleButtonLiga" onclick="checkSelectedButtons()">LIGAR</button>
            <label> ... </label>
            <button class="button" id="toggleButtonPausa" onclick="togglePauseButton()"
                style="display: none;">PAUSAR</button>
        </div>

        <div class="container">
            <label>STATUS:</label>
            <textarea id="statusTextArea" class="mensagem"
                readonly>Informações adicionais sobre os eventos...</textarea>
        </div>


        <script>
            let selectedProgramCiclo = "";
            let selectedWaterLevel = "";
            let action = "";
            const buttonLiga = document.getElementById("toggleButtonLiga");
            buttonLiga.style.backgroundColor = "green";
            const buttonPausa = document.getElementById("toggleButtonPausa");
            buttonPausa.style.backgroundColor = "green";

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
                let action = "";
                let programa = "";
                let nivel = "";
                if (buttonLiga.style.backgroundColor === "green") {
                    buttonLiga.style.backgroundColor = "red";
                    buttonLiga.innerText = "DESLIGAR";
                    action = "LIGAR";
                    programa = selectedProgramCiclo;
                    nivel = selectedWaterLevel;
                    buttonPausa.style.display = "block"; // Exibir o botão de pausar
                } else {
                    buttonLiga.style.backgroundColor = "green";
                    buttonLiga.innerText = "LIGAR";
                    action = "DESLIGAR";
                    buttonPausa.style.display = "none"; // Ocultar o botão de pausar
                }
                sendData({ action: action, programa: selectedProgramCiclo, nivel: selectedWaterLevel });
            }
            function togglePauseButton() {
                let action = "";
                let programa = "";
                let nivel = "";
                if (buttonPausa.style.backgroundColor === "green") {
                    buttonPausa.style.backgroundColor = "red";
                    buttonPausa.innerText = "REINICIAR";
                    action = "PAUSAR";
                    programa = selectedProgramCiclo;
                    nivel = selectedWaterLevel;
                } else {
                    buttonPausa.style.backgroundColor = "green";
                    buttonPausa.innerText = "PAUSAR";
                    action = "REINICIAR";
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
            function sendData(data) {
                const xhr = new XMLHttpRequest();
                xhr.onreadystatechange = function () {
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
            // Crie uma função para tratar a resposta do servidor
            function handleResponse() {
                if (xhr.status == 200) {
                    var mensagem = xhr.responseText;
                    var textarea = document.getElementById("statusTextArea");
                    textarea.value = mensagem;
                }
            }

            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    handleResponse(); // Chama a função para tratar a resposta do servidor
                }
            };
            xhr.open("GET", "/msg");
            xhr.send();

            // Define uma função que será executada a cada 10 segundos
            function enviarRequisicao() {
                xhr.open("GET", "/msg");
                xhr.send();
            }

            // Usa o método setInterval para executar a função a cada 10 segundos
            setInterval(enviarRequisicao, 10000);


        </script>
</body>

</html>)=====";
    return webpage;
}

////////////////////////    ENCHER   //////////////////////

boolean Encher(String nivel)
{
    /*
        1. Quando o nível for atingido o sensor de nível retorna zero.
        2. Adotou-se a correspondencia (string=> valor): "ALTO"=1 ; "MEDIO"=2 ; "BAIXO"=3.
    */
    byte Nivel;
    digitalWrite(DRENO, DESLIGA);
    digitalWrite(MOTOR, DESLIGA);

    if (nivel.equals(String("ALTO")))
    {
        Nivel = 1;
    }
    else if (nivel.equals(String("MEDIO")))
    {
        Nivel = 2;
    }
    else
    {
        Nivel = 3;
    }
    Serial.println("Enchendo");
    switch (Nivel)
    {
    case 1:
    {
        while (digitalRead(NIVEL_ALTO))
        {
            delay(10);
            digitalWrite(AGUA, LIGA);
            delay(10);
        }
    }
    break;
    case 2:
    {
        while (digitalRead(NIVEL_MEDIO))
        {
            delay(10);
            digitalWrite(AGUA, LIGA);
            delay(10);
        }
    }
    break;
    case 3:
    {
        while (digitalRead(NIVEL_BAIXO))
        {
            delay(10);
            digitalWrite(AGUA, LIGA);
            delay(10);
        }
    }
    break;
    }
    digitalWrite(AGUA, DESLIGA);
    return true;
}

////////////////////////////////   DRENAR   ////////////////////////

boolean Drenar()
{
    /*
       Ao se entrar nesta função:

      i)   Se o nivel da agua estiver acima do nível "BAIXO", o dreno funcionará até que
           se tenha transcorrido DT segundos depois do nível "BAIXO" ter sido alcançado.
      ii)  Se o nivel da agua estiver abaixo do nível "BAIXO" o dreno ficará ligado por mais DT segundos.

    */
    long unsigned int timeDreno;
    long unsigned int DT = 40; // tempo que fica ligado depois de atingir o nível zero.
    digitalWrite(AGUA, DESLIGA);
    digitalWrite(MOTOR, DESLIGA);
    digitalWrite(DRENO, LIGA);
    while (!digitalRead(NIVEL_BAIXO))
    {
        delay(100);
    }
    timeDreno = millis() / 1000 + DT;
    while (millis() / 1000 < timeDreno)
    {
        Serial.println("Drenando");
        delay(1000);
    }
    digitalWrite(DRENO, DESLIGA);
    return true;
}

boolean Molho(long unsigned int DT)
{
    /*
       DT - tempo do molho em minutos
    */
    Encher(nivel);
    long unsigned int Time_Molho = millis() / 1000 + DT * 60;
    while (millis() / 1000 < Time_Molho)
    {
        Serial.println("De molho");
        delay(100);
    }
    return true;
}

boolean Bater(long unsigned int DT)
{
    /*
        DT - tempo de bater em minutos
    */
    Encher(nivel);
    long unsigned Time_Bater = millis() / 1000 + DT * 60;
    digitalWrite(MOTOR, LIGA);
    while (millis() / 1000 < Time_Bater)
    {
        delay(1000);
        Serial.println("Batendo");
    }
    digitalWrite(MOTOR, DESLIGA);
    return true;
}

void SeMexe(String programa)
{
    boolean drenado = false, cheio = false;
    long unsigned int DT = 0;
    byte Programa = 10;

    if (programa.equals(String("LONGO")))
    {
        Programa = 0;
    }
    else if (programa.equals(String("MÉDIO")))
    {
        Programa = 1;
    }
    else if (programa.equals(String("CURTO")))
    {
        Programa = 2;
    }
    else if (programa.equals(String("ESVAZIAR")))
    {
        Programa = 3;
    }
    else if (programa.equals(String("MOLHO")))
    {
        Programa = 4;
    }
    Serial.printf("Programa: %i    programa: %s\n", Programa, programa.c_str());
    switch (Programa)
    {
    case 0: //  Muita Suja. (5342-3322-2422) => 38 minutos.
        if (!P0[0])
            P0[0] = Bater(4);
        else if (!P0[1])
            P0[1] = Molho(5);
        else if (!P0[2])
            P0[2] = Bater(3);
        else if (!P0[3])
            P0[3] = Molho(4);
        else if (!P0[4])
            P0[4] = Bater(2);
        else if (!P0[5])
            P0[5] = Molho(4);
        else if (!P0[6])
            P0[6] = Bater(2);
        else if (!P0[7])
            P0[7] = Drenar();
        else if (!P0[8])
            P0[8] = Bater(3);
        else if (!P0[9])
            P0[9] = Molho(4);
        else if (!P0[10])
            P0[10] = Bater(2);
        else if (!P0[11])
            P0[11] = Drenar();
        else if (!P0[12])
            P0[12] = Bater(2);
        else if (!P0[13])
            P0[13] = Molho(4);
        else if (!P0[14])
            P0[14] = Bater(2);
        else if (!P0[15])
            P0[15] = Drenar();
        else if (P0[15])
        {
        }
        break;
    case 1: // Normal.(3532)   => 16 minutos.
        if (!P2[0])
            P2[0] = Bater(2);
        else if (!P2[1])
            P2[1] = Molho(4);
        else if (!P2[2])
            P2[2] = Bater(2);
        else if (!P2[3])
            P2[3] = Drenar();
        else if (!P2[4])
            P2[4] = Bater(2);
        else if (!P2[5])
            P2[5] = Molho(3);
        else if (!P2[6])
            P2[6] = Bater(1);
        else if (!P2[7])
            P2[7] = Drenar();
        break;
    case 2: // Delicada. (2322)    => 07 minutos.
        if (!P3[0])
            P3[0] = Bater(2);
        else if (!P3[1])
            P3[1] = Molho(2);
        else if (!P3[2])
            P3[2] = Bater(1);
        else if (!P3[3])
            P3[3] = Drenar();
        break;
    case 3: // Drenar.
        if (!drenado)
            drenado = Drenar();
        break;
    case 4: // Molho.
        if (!cheio)
        {
            cheio = Encher(nivel);
            Bater(1);
            break;
        }
    }
    DT = millis() / 1000 + 3600;
    while (millis() / 1000 < DT)
    {
        buzina();
    }
    noTone(BUZZER);
}
void buzina()
{
    tone(BUZZER, 200);
    delay(200);
    tone(BUZZER, 10);
    delay(200);
}
void desligar()
{
    digitalWrite(DRENO, DESLIGA);
    digitalWrite(MOTOR, DESLIGA);
    digitalWrite(AGUA, DESLIGA);
}