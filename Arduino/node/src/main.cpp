#include <Arduino.h>

#include <WiFi.h>
#include <WiFiUdp.h>

int lamp = 18;
char packetBuffer[1];
WiFiUDP server;

void setup()
{
  pinMode(lamp, OUTPUT);
  Serial.begin(9600);

  WiFi.begin("Telstra34D373", "vje8zwwbts");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  Serial.println(WiFi.localIP());
  server.begin(1974);
}

void loop()
{
  if (server.parsePacket() && server.read(packetBuffer, 1) > 0)
  {
    Serial.println("Received a packet!");
    digitalWrite(lamp, !digitalRead(lamp));

    server.flush();
  }
}