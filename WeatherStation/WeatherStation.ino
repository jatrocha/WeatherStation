#include "Arduino.h"
#include "dht.h"
#include <SPI.h>
#include <Ethernet.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(172, 16, 1, 199);
EthernetServer server(8080);

int ledPin = 7; //Led no pino 7

int ldrPin = 0; //LDR no pino analígico 8

int ldrValor = 0; //Valor lido do LDR

void setup() {

  pinMode(ledPin, OUTPUT); //define a porta 7 como saída

  Serial.begin(9600); //Inicia a comunicação serial

  // Inicia a conexão Ethernet e o servidor:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Servidor iniciado em: ");
  Serial.println(Ethernet.localIP());

}

void loop() {


  // Aguardando novos clientes;
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Novo Cliente");
    // Uma solicitação http termina com uma linha em branco
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // Se tiver chegado ao fim da linha (recebeu um novo
        // Caractere) e a linha estiver em branco, o pedido http terminou,
        // Para que você possa enviar uma resposta
        if (c == '\n' && currentLineIsBlank) {
          // Envia um cabeçalho de resposta HTTP padrão
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // a conexão será fechada após a conclusão da resposta
          client.println();
          client.println("{");


          //delay(5000);

          digitalWrite(ledPin, HIGH);

          float h = dht.readHumidity();
          // Read temperature as Celsius (the default)
          float t = dht.readTemperature();

          // Check if any reads failed and exit early (to try again).
          if (isnan(h) || isnan(t) ) {
            Serial.println("Failed to read from DHT sensor!");
            return;
          }

          // Compute heat index in Celsius (isFahreheit = false)
          float hic = dht.computeHeatIndex(t, h, false);

          ///ler o valor do LDR
          ldrValor = analogRead(ldrPin); //O valor lido será entre 0 e 1023

          digitalWrite(ledPin, LOW);

          client.println("\"guid\":");
          client.println("\"be90d7b4-e42e-410c-8bfa-52bd18bb24c8\"");
          client.println(",");

          client.println("\"temperatura\":");
          client.println(t);
          client.println(",");

          client.println("\"umidade\":");
          client.println(h);
          client.println(",");

          client.println("\"indiceCalor\":");
          client.println(hic);
          client.println(",");


          client.println("\"iluminancia\":");
          client.println(ldrValor);

          client.println("}");

          break;
        }
        if (c == '\n') {
          // Você está começando uma nova linha
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // Você recebeu um caracter na linha atual.
          currentLineIsBlank = false;
        }
      }
    }
    // Dar tempo ao navegador para receber os dados
    delay(1);
    // Fecha a conexão:
    client.stop();
    Serial.println("Cliente desconectado");
  }

}
