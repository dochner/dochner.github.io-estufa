#include <DHT.h>
#include <LiquidCrystal.h>
#include <string.h>
#include <SPI.h>      //Biblioteca do Shield Ethernet¹
#include <Ethernet.h> //Biblioteca do Shield Ethernet²
#include <SoftwareSerial.h>

/*
* @file estufaV1.ino
* @author beatriz
/*
* pinagem
*sensor dht11 A0
*sensor nivel agua (out)13 (in)12
*sensor umidade solo (out)A1 (in)A2
*display (rst)3 (ce)4 dc)5 (din)6 (clk)7
*reles (cooler/peltier)A3 (luz)A4 (bombaAgua)A5
/Display – Ativação do Display de Definição dos Pinos
/* Define os pinos que serão ligados ao LCD */
LiquidCrystal display(12, 11, 5, 4, 3, 2);

//Array simbolo grau
byte grau[8] = {
    B00001100,
    B00010010,
    B00010010,
    B00001100,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
};

byte term[] = {
    B00100,
    B01110,
    B01110,
    B01110,
    B01110,
    B10011,
    B10111,
    B01110};

byte gota[] = {
    B00001,
    B00011,
    B00111,
    B01101,
    B11001,
    B11111,
    B11111,
    B01110};

byte lamp[] = {
    B01110,
    B01110,
    B10101,
    B11111,
    B10001,
    B01110,
    B00000,
    B00000};
//Fim do Display

//DHT
#define DHTTYPE DHT11
#define DHTPIN A0
DHT dht11(DHTPIN, DHTTYPE);

//Solo
#define nivelPinOut 13
#define nivelPinIn 12
#define soloPin A1

//Reles
#define coolerPin 14 // RELE 1
#define luzPin 15    // RELE 2
#define bombaPin 16  // RELE 3
#define umidPin 17   // RELE 4

//Variaveis de controle
char ok_no[3], luz[3];
float h, t, l;
int luminosidade;
float sensorTemperatura, sensorUmidade;
int sensorSolo, sensorNivel;
String inputString = "";
boolean stringComplete = false;
int temperatura, umidade, solo;

//Início Ethernet
byte mac[] = {0xDE, 0xAE, 0xBC, 0xED, 0xFE, 0x02};
int sdcard = 4;
//Configurações ThingSpeak
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "H1BHOIYUW1JIKBYG";
// Intervalo de tempo para update no ThingSpeak(Segundos * 1000 = interval)
const int updateThingSpeakInterval = 16 * 1000;
long lastConnectionTime = 0;
long lastCheck = 0;
boolean lastConnected = false;
int failedCounter = 0;
EthernetClient client; // Iniciar Client Ethernet
//Fim EthernetClient

//Definição das funções
String criaString(int temperatura, int umidade, int solo, int nivel);
void imprimeTela(int temperatura, int umidade, int solo, int nivel, char *luz);
void aquecer();
void irrigar();
void apagarluz();
void acenderluz();
void updateThingSpeak(String);
void inicioEthernet();


SoftwareSerial esp8266(2,3);

void setup()
{
  display.createChar(0, grau); //Cria o caractere customizado com o símbolo do grau
  display.createChar(1, gota); //Cria o caractere customizado com o simoblo da umidade
  display.createChar(2, term); //Cria o caractere customizado com o simoblo de temperatura
  display.createChar(3, luz); //Cria o caractere customizado com o simoblo de temperatura
    
  
  inputString.reserve(200);
  /* configuracao dos pinos */
  dht11.begin();
  pinMode(nivelPinOut, OUTPUT);
  pinMode(nivelPinIn, INPUT);
  pinMode(soloPin, INPUT);
  pinMode(coolerPin, OUTPUT);
  pinMode(luzPin, OUTPUT);
  pinMode(bombaPin, OUTPUT);
  /* liga os sensores de nivel e umidade do solo */
  digitalWrite(nivelPinOut, HIGH);
  /* configura o lcd display */
  display.begin(16,2);
  display.setContrast(40);
  display.clearDisplay();
  /* inicia comunicacao serial */
  Serial.begin(9600);
  esp8266.begin(115200);
  sendData("AT+RSTrn", 2000, DEBUG);
  delay(1000);
  Serial.println("Versao de firmware");
  delay(3000);
  sendData("AT+GMRrn", 2000, DEBUG); // rst
  // Configure na linha abaixo a velocidade desejada para a
  // comunicacao do modulo ESP8266 (9600, 19200, 38400, etc)
  sendData("AT+CIOBAUD=19200rn", 2000, DEBUG);
  Serial.println("** Final **");
  strcpy(luz, "NO");
  digitalWrite(luzPin, LOW);
  digitalWrite(coolerPin, LOW);
  digitalWrite(coolerPin, LOW);
  /* inicializa valores de controle */
  temperatura = 25;
  solo = 50;
}
void loop()
{
  /* limpa a tela */
  display.clearDisplay();
  /* SENSORES */
  /* leitura da temperatura */
  sensorTemperatura = dht11.readTemperature();
  sensorUmidade = dht11.readHumidity();
  sensorSolo = analogRead(soloPin); /* leitura da umidade do */
  //float porcSoloFloat = (sensorSolo / 1024) * 100;
  int porcSolo = sensorSolo;
  sensorNivel = digitalRead(nivelPinIn); /* leitura do nivel de agua */
  luminosidade = map(analogRead(luzPin), 0, 1023, 0, 100);
  /* envia uma string para a rasp com todas as informacoes */
  Serial.println(criaString(sensorTemperatura, sensorUmidade, sensorSolo, sensorNivel));
  imprimeTela(sensorTemperatura, sensorUmidade, sensorSolo, sensorNivel, luz);
  // ThingSpeak
  char t_buffer[10];
  char h_buffer[10];                                  // Buffer da Umidade
  String temp = dtostrf(t, 0, 5, t_buffer);           // String da Temperatura
  String humid = dtostrf(h, 0, 5, h_buffer);          // String da Umidade
  char l_buffer[10];                                  // Buffer da Luminosidade
  String luz = dtostrf(luminosidade, 0, 5, l_buffer); // String da Luminosidade

  if (!client.connected() && lastConnected)
  {
    Serial.println("...desconectado");
    Serial.println();
    client.stop();
  }
  // Update do ThingSpeak
  if (!client.connected() && (millis() - lastConnectionTime >
                              updateThingSpeakInterval))
  {
    updateThingSpeak("field1=" + temp + "&field2=" + humid + "&field3=" + luz); // Envio das 3 Strings para o ThingSpeak
  }
  // Checar se o Arduino precisa ser reiniciado (reinicia a Ethernet caso a conexão falhe mais que 3x)
  if (failedCounter > 3)
  {
    inicioEthernet();
  }
  lastConnected = client.connected();
}

void updateThingSpeak(String tsData)
{ //Fazer update dos dados no ThingSpeak
  if (client.connect(thingSpeakAddress, 80))
  {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n");
    client.print(tsData);
    lastConnectionTime = millis();
    if (client.connected())
    {
      Serial.println("Conectando ao ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
      Serial.println("A conexão ao ThingSpeak falhou (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  }
}
//Conectar o Arduino na rede e obter um IP usando o DHCP
void inicioEthernet()
{
  client.stop();
  Serial.println("Conectando o Arduino a rede...");
  Serial.println();
  delay(1000);
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP falhou, reinicie o Arduino");
    Serial.println();
  }
  else
  {
    Serial.println("Conectado a rede usando DHCP");
    Serial.println();
  }
}

String criaString(int temperatura, int umidade, int solo, int nivel)
{
  char nivelStr[3];
  if (nivel == 1)
  {
    if (nivel == 1)
    {
      strcpy(nivelStr, "OK");
    }
    else
    {
      strcpy(nivelStr, "NO");
      strcpy(nivelStr, "NO");
    }
  }
  float porcSoloFloat = solo / 1024.0 * 100.0;
  int porcSolo = porcSoloFloat;
  String temp = String(temperatura);
  String umid = String(umidade);
  String umids = String(porcSolo);
  String tudo = "";
  String t = "TEMPERATURA: ";
  String u = "UMIDADE: ";
  String s = "UMIDADESOLO: ";
  String n = "NIVEL: ";
  String div = "|";
  tudo.concat(t);
  tudo.concat(temp);
  tudo.concat(div);
  tudo.concat(u);
  tudo.concat(umid);
  tudo.concat(div);
  tudo.concat(s);
  tudo.concat(umids);
  tudo.concat(div);
  tudo.concat(n);
  tudo.concat(nivelStr);
  tudo.concat(div);
  return tudo;
}

void imprimeTela(int temperatura, int umidade, int solo, int nivel, char *luz)
{
  float porcSoloFloat = solo / 1024.0 * 100.0;
  int porcSolo = porcSoloFloat;
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(30, 0);
  display.println("v1.0");
  display.print("Temp: ");
  display.print(temperatura);
  display.println("oC");
  display.print("Umid: ");
  display.print(umidade);
  display.println("%");
  display.print("UmidSl: ");
  display.print(porcSolo);
  display.println("%");
  display.print("Nivel: ");
  if (nivel == 1)
  {
    display.println("OK");
  }
  else
  {
    display.println("NO");
  }
  display.print("Luz: ");
  display.println(luz);
  display.display();
  delay(500);
}

/*
* @brief liga a peltier com o lado quente para dentro. o mesmo rele * @brief liga a peltier com o lado quente para dentro. o mesmo rele que liga o cooler, liga a peltierque liga o cooler, liga a peltier
*/
void aquecer()
{
  digitalWrite(coolerPin, HIGH);
}
/*
* @brief liga a bomba d'agua por 2 segundos e desliga* @brief liga a bomba d'agua por 2 segundos e desliga
*/
void irrigar()
{
  digitalWrite(bombaPin, HIGH);
  delay(2000);
  digitalWrite(bombaPin, LOW);
}

/*
/*
* @brief apaga a luz
*/
void apagarluz()
{
  digitalWrite(luzPin, LOW);
  strcpy(luz, "");
  strcpy(luz, "OFF");
}

/*
* @brief acende a luz
*/

void acenderluz()
{
  digitalWrite(luzPin, HIGH);
  strcpy(luz, "");
  strcpy(luz, "ON");
}

/*
/*
* @briefinterrupcao serial* @briefinterrupcao serial
*/
void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n')
    {
      stringComplete = true;
    }
    if (inputString.equals("irrigar"))
    {
      Serial.println("COMANDO RECEBIDO PELA SERIAL: LIGAR BOMBA");
      irrigar();
    }
    else if (inputString.equals("luz0") || inputString.equals("luz1"))
    {
      if (inputString.charAt(3) == '0')
      {
        Serial.println("COMANDO RECEBIDO PELA SERIAL: APAGAR LUZ");
        apagarluz();
      }
      else
      {
        Serial.println("COMANDO RECEBIDO PELA SERIAL: ACENDER LUZ");
        acenderluz();
      }
    }
    else if (inputString.charAt(0) == 't' && inputString.charAt(1) == 'e' && inputString.charAt(2) == 'm' && inputString.charAt(3) == 'p')
    {
      String valor_temp = inputString.substring(11);
      int valor_temp_int = valor_temp.toInt();
      temperatura = valor_temp_int;
      Serial.print("COMANDO RECEBIDO PELA SERIAL: TEMPERATURA = ");
      Serial.print(valor_temp_int);
      Serial.println(" oC");
      if (sensorTemperatura < temperatura)
      {
        aquecer();
      }
      else
      {
        digitalWrite(coolerPin, LOW);
      }
    }
  }
  inputString = "";
}

String sendData(String command, const int timeout, boolean debug)
{
  // Envio dos comandos AT para o modulo
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}
