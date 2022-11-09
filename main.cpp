/* Daniel Rios Barboza - Comunicação via MQTT para monitoramento e controle*/

// Bibliotecas
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define Led 0        // D3
#define rele 15      // D8
#define entrada_1 14 // D5
#define entrada_2 12 // D6
#define entrada_3 13 // D7
#define chave 16     // D0

int estado_in1;
int estado_in2;
int estado_in3;
float valor;
char valor_analogico_str[10] = {0};

const char *SSID = "********";       // SSID / nome da rede WiFi que deseja se conectar
const char *PASSWORD = "**********"; // Senha da rede WiFi que deseja se conectar
WiFiClient wifiClient;

// MQTT Server
const char *BROKER_MQTT = "mqtt.eclipseprojects.io"; // URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                              // Porta do Broker MQTT

#define ID_MQTT "LD0101" // Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior.
#define TOPIC_SUBSCRIBE "LD0102"
#define TOPIC_PUBLISH1 "LD0103"
#define TOPIC_PUBLISH2 "LD0104"
#define TOPIC_PUBLISH3 "LD0105"
#define TOPIC_PUBLISH4 "LD0106"
#define TOPIC_PUBLISH5 "LD0107"
PubSubClient MQTT(wifiClient); // Instancia o Cliente MQTT passando o objeto espClient

// Declaração das Funções
void mantemConexoes();                                              // Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaWiFi();                                                 // Faz conexão com WiFi
void conectaMQTT();                                                 // Faz conexão com Broker MQTT
void recebePacote(char *topic, byte *payload, unsigned int length); // Assinatura e Publicação no aplicativo
void read_analogic_port();                                          // Leitura da porta analogica

void setup()
{
  Serial.begin(115200);
  pinMode(Led, OUTPUT);
  pinMode(rele, OUTPUT);
  pinMode(entrada_1, INPUT);
  pinMode(entrada_2, INPUT);
  pinMode(entrada_3, INPUT);
  pinMode(chave, INPUT);

  conectaWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(recebePacote);
}
void loop()
{
  mantemConexoes();
  MQTT.loop();
  MQTT.setCallback(recebePacote);

  estado_in1 = digitalRead(entrada_1);
  estado_in2 = digitalRead(entrada_2);
  estado_in3 = digitalRead(entrada_3);

  read_analogic_port();
  MQTT.publish(TOPIC_PUBLISH5, valor_analogico_str);

  if (estado_in1 == HIGH)
  {

    Serial.println("Entrada 1 está Energizada");
    MQTT.publish(TOPIC_PUBLISH2, "Entrada 1 está Energizada");
  }

  else
  {
    Serial.println("Entrada 1 está Desergizada");
    MQTT.publish(TOPIC_PUBLISH2, "Entrada 1 está Desenergizada");
    delay(800);
  }

  if (digitalRead(entrada_2) == HIGH)
  {
    Serial.println("Entrada 2 está Energizada");
    MQTT.publish(TOPIC_PUBLISH3, "Entrada 2 está Energizada");
  }

  else
  {
    Serial.println("Entrada 2 está Desergizada");
    MQTT.publish(TOPIC_PUBLISH3, "Entrada 2 está Desenergizada");
    delay(800);
  }

  if (digitalRead(entrada_3) == HIGH)
  {
    Serial.println("Entrada 3 está Energizada");
    MQTT.publish(TOPIC_PUBLISH4, "Entrada 3 está Energizada");
  }

  else
  {
    Serial.println("Entrada 3 está Desergizada");
    MQTT.publish(TOPIC_PUBLISH4, "Entrada 3 está Desenergizada");
    delay(800);
  }
}

void mantemConexoes()
{
  if (!MQTT.connected())
  {
    conectaMQTT();
  }

  conectaWiFi(); // se não há conexão com o WiFI, a conexão é refeita
}

void conectaWiFi()
{

  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  Serial.print("Conectando-se na rede: ");
  Serial.print(SSID);
  Serial.println("  Aguarde!");

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(SSID);
  Serial.print("  IP obtido: ");
  Serial.println(WiFi.localIP());
}

void conectaMQTT()
{
  while (!MQTT.connected())
  {
    Serial.print("Conectando ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado ao Broker com sucesso!");
      MQTT.subscribe(TOPIC_SUBSCRIBE);
    }
    else
    {
      Serial.println("Noo foi possivel se conectar ao broker.");
      Serial.println("Nova tentatica de conexao em 10s");
      delay(10000);
    }
  }
}

void recebePacote(char *topic, byte *payload, unsigned int length)
{

  String msg;
  Serial.println(msg);

  // obtem a string do payload recebido
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
    Serial.println("msg dentro do for: " + msg);
  }

  if (msg == "0")
  {
    Serial.println("Led Desligado");
    digitalWrite(rele, LOW);
    MQTT.publish(TOPIC_PUBLISH1, "Led Desligado");
  }

  if (msg == "1")
  {
    Serial.println("Led Ligado");
    digitalWrite(rele, HIGH);
    MQTT.publish(TOPIC_PUBLISH1, "Led Ligado");
  }
}

void read_analogic_port()
{

  valor = analogRead(A0);
  valor = (valor * 100) / 1023;
  Serial.println(valor);
  sprintf(valor_analogico_str, "%.2f", valor);
}
