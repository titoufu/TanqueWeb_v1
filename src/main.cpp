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

String ESTADO;
String programa;
String nivel;
boolean novoDado = true;
bool dreno = false;
unsigned long sendDataPrevMillis = 0;
void SeMexe(String ciclo);
void buzina(void);
void desligar(void);

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
        if (ESTADO == "LIGAR")
        {
            // SeMexe(programa);
            Serial.print("SE MEXE  => ");
            delay(1000);
            Serial.println(ESTADO);
            delay(1000);
        }
        else
        {
            Serial.print("DESLIGA  => ");
            delay(1000);
            Serial.println(ESTADO);
            delay(1000);

            desligar(); // Chama a função desligar() caso contrário
        }
        novoDado = false;
    }
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

    Serial.print("Ação: ");
    Serial.println(action);
    Serial.print("Programa: ");
    Serial.println(programa);
    Serial.print("Nível: ");
    Serial.println(nivel);

    server.send(200, "text/plain", "Dados recebidos pelo ESP8266.");

    novoDado = true;
}

String getPage()
{
    // Crie a página HTML
    const char *webpage = R"=====(
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
                action = "LIGAR";
                programa = selectedProgramCiclo;
                nivel = selectedWaterLevel;
            } else {
                button.style.backgroundColor = "green";
                button.innerText = "LIGAR";
                action = "DESLIGAR";
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