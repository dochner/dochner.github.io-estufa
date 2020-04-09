#include <Arduino.h>
#include <Ethernet.h>

class estufa
{
    har ok_no[3], luz[3];
    float h, t, l;
    int luminosidade;

    //Início Ethernet
    byte mac[] = {0xDE, 0xAE, 0xBC, 0xED, 0xFE, 0x02};
    int sdcard = 4;
    //Configurações ThingSpeak
    char thingSpeakAddress[] = "api.thingspeak.com";
    String writeAPIKey = "H1BHOIYUW1JIKBYG";
    // Intervalo de tempo para update no ThingSpeak(Segundos * 1000 = interval)
    const int updateThingSpeakInterval = 16 * 1000;
    //Setup de Variáveis
    long lastConnectionTime = 0;
    long lastCheck = 0;
    boolean lastConnected = false;
    int failedCounter = 0;
    EthernetClient client; // Iniciar Client Ethernet
    //Fim Ethernet

    /* valores escolhidos pelo usuario */
    int temperatura, umidade, solo;
    DHT dht11(DHTPIN, DHTTYPE);
    /* valores lidos pelo arduino */
    float sensorTemperatura, sensorUmidade;
    int sensorSolo, sensorNivel;
    String inputString = "";
    boolean stringComplete = false;

    //Definição das funções
    String criaString(int temperatura, int umidade, int solo, int nivel);
    void imprimeTela(int temperatura, int umidade, int solo, int nivel, char *luz);
    void aquecer();
    void irrigar();
    void apagarluz();
    void acenderluz();
    void updateThingSpeak();
}