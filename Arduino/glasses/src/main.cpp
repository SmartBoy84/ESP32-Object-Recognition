#include "Arduino.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <esp32cam.h>
#include <WebServer.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define LED_BUILTIN 4
#define PACKET_SIZE 4

WebServer webServer(80);

//********************************************************************************************Camera initialization***********************************************************************************************

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);

void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr)
  {
    Serial.println("CAPTURE FAIL");
    webServer.send(503, "", "");
    return;
  }
  frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size());

  webServer.setContentLength(frame->size());
  webServer.send(200, "image/jpeg");
  WiFiClient webClient = webServer.client();
  frame->writeTo(webClient);
}

void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes))
  {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes))
  {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleMjpeg()
{
  if (!esp32cam::Camera.changeResolution(hiRes))
  {
    Serial.println("SET-HI-RES FAIL");
  }

  Serial.println("STREAM BEGIN");
  WiFiClient webClient = webServer.client();

  int res = esp32cam::Camera.streamMjpeg(webClient);
  if (res <= 0)
  {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
}

//********************************************************************************client initialization****************************************************************************************

WiFiUDP server;
const char *serverIP = "192.168.0.98";
int serverPort = 2807;

char packetBuffer[PACKET_SIZE];
int ledChannel = 5;

uint8_t settings[3] = {0, 38}; //[led brightness], [low voltage - stored in multiples of 100 due to hex encoding being iffy, ya know?], [dev mode - disable low power turn off]
uint8_t status[1] = {0};       //[current voltage]

float oldTime = millis();
float reportTime = 2000; //ms

float voltMax = (4095 / 3.5) * 2.1;
float voltMin = (4095 / 3.5) * ((settings[1] / 10) / 2);
float voltRange = ((4095 / 3.5) * 2.1) - voltMin; //voltMax-voltMin

int probePin = 33;
//2.1 has been found from finding the max vOUT of the voltage divider (both resistors were 330 ohm): (4.2 * 330)/(330+330)
//the voltconstant is basically the number of "adc units"/volt, precalculated to lessen the load
int currentVoltage = 0;

void reportBack() //Pong!
{
  //Serial.println("PONG!");
  byte message[1 + sizeof(settings) + sizeof(status)]; //1 byte is the size of the header
  message[0] = 0xff;

  for (int i = 0; i < sizeof(settings); i++)
    message[i + 1] = settings[i]; //1 - header, 1 - since array starts at 0

  for (int i = 0; i < sizeof(status); i++)
    message[i + 1 + sizeof(settings)] = status[i];

  WiFiUDP client;
  client.beginPacket(serverIP, serverPort);
  client.write(message, sizeof(message));
  client.endPacket();
  client.stop();
}

//*********************************************************************************************************************************************************************************************************

TaskHandle_t mailBoyMother;

void mailBoy(void *parameter)
{
  for (;;)
  {
    if (millis() - oldTime > reportTime)
    {
      reportBack();
      oldTime = millis();
    }

    status[0] = ((analogRead(probePin) - voltMin) * 100) / voltRange; //set base on voltmax

    /*if (status[0] <= settings[1] * 1000)
    {
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, LOW);
      gpio_hold_en(GPIO_NUM_4);
      gpio_deep_sleep_hold_en();
      esp_deep_sleep_start();
    }*/

    if (server.parsePacket() && server.read(packetBuffer, PACKET_SIZE) > 0)
    {
      Serial.println("Received a packet!");

      if (packetBuffer[0] != 255)
      {
        Serial.println("It doesn't look good, cap'n");
        return; //*ghost noises* Phantommmm udp packets...
      }

      if (packetBuffer[1] == 0) //request for setting change, idk why I'm making this so elaborate...
      {
        // individual handlers
        if (packetBuffer[2] == 0)
          if (packetBuffer[3] > 255)
            return;
          else
            ledcWrite(ledChannel, packetBuffer[3]);

        if (packetBuffer[2] == 1)
          if (packetBuffer[3] < 37) //can't allow people to be stupid, can I?
            return;
        voltMin = (4095 / 3.5) * ((packetBuffer[3] / 10) / 2);
        voltRange = voltMax - voltMin;

        //if we've reached this far, it means none of the peeps have complained - time to set and report back
        settings[packetBuffer[2]] = packetBuffer[3];
        reportBack();
      }

      server.flush(); //Here comes the garbage truck!
    }
    delay(1);
  }
}

void setup()
{
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  ledcSetup(ledChannel, 5000, 8); //camera uses channel 1, so use channel 2
  ledcAttachPin(LED_BUILTIN, ledChannel);

  WiFi.begin("[SSID]", "[password]");

  while (WiFi.status() != WL_CONNECTED)
    delay(100);

  server.begin(serverPort);
  Serial.println(WiFi.localIP());

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  //name of func, name of task, stack size, task parameter, priority, handle, core

  webServer.on("/cam-lo.jpg", handleJpgLo);
  webServer.on("/cam-hi.jpg", handleJpgHi);
  webServer.on("/cam.mjpeg", handleMjpeg);

  webServer.begin();

  //server.handleClient();
  xTaskCreatePinnedToCore(mailBoy, "video stream", 5000, NULL, 0, &mailBoyMother, 1); //No fighting boys!
}

void loop() { webServer.handleClient(); }
