#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN            D6  //pin de salida D1
#define NUMPIXELS      425  //numero de pixeles a controlar

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid     = "SSID";  //nombre del wifi
const char* password = "PASSWORD"; //clave router
IPAddress ip(192, 168, 0, 200	);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
unsigned int localPort = 65506;      // local port to listen for UDP packets
const int PACKET_SIZE = 1357;
byte packetBuffer[PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

typedef struct
{
      byte r;
      byte g;
      byte b;
} colorpin;

colorpin led;
int led_index = 0;

void WiFiEvent(WiFiEvent_t event)
{
      switch (event)
      {
            case WIFI_EVENT_STAMODE_GOT_IP:
                  pixels.begin();
                  break;
            case WIFI_EVENT_STAMODE_DISCONNECTED:
                  break;
      }
}

void setup()
{
      //  Serial.begin(115200);
      WiFi.disconnect(true);
      delay(1000);
      WiFi.onEvent(WiFiEvent);
      WiFi.config(ip, gateway, subnet);
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED)
      {
            delay(500);
            //  Serial.print("x");
      }

      // Serial.println("");
      // Serial.println("WiFi connected");
      // Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      // Serial.println("Starting UDP");
      udp.begin(localPort);
      // Serial.print("Local port: ");
      // Serial.println(udp.localPort());
}

void loop()
{
      int cb = udp.parsePacket();
      if (!cb)
      {
            //    Serial.setDebugOutput(true);
      }
      else
      {
            // We've received a packet, read the data from it
            udp.read(packetBuffer, PACKET_SIZE); // read the packet into the buffer
            if (cb >= 6 && packetBuffer[0] == 0x9C)
            {
                  // header identifier (packet start)
                  byte blocktype = packetBuffer[1]; // block type (0xDA)
                  unsigned int framelength = ((unsigned int)packetBuffer[2] << 8) | (unsigned int)packetBuffer[3]; // frame length (0x0069) = 105 leds
                  //    Serial.print("Frame.");
                  //    Serial.println(framelength); // chan/block
                  byte packagenum = packetBuffer[4];   // packet number 0-255 0x00 = no frame split (0x01)
                  byte numpackages = packetBuffer[5];   // total packets 1-255 (0x01)

                  if (blocktype == 0xDA)
                  {
                        // data frame ...
                        //        Serial.println("command");

                        int packetindex;

                        if (cb >= framelength + 7 && packetBuffer[6 + framelength] == 0x36)
                        {
                              // header end (packet stop)
                              //Serial.println("s:");
                              int i = 0;
                              packetindex = 6;
                              if (packagenum == 1)
                              {
                                    led_index = 0;
                              }
                              while (packetindex < (framelength + 6))
                              {
                                    led.r = ((int)packetBuffer[packetindex]);
                                    led.g = ((int)packetBuffer[packetindex + 1]);
                                    led.b = ((int)packetBuffer[packetindex + 2]);
                                    pixels.setPixelColor(led_index, led.r, led.g, led.b);
                                    led_index++;
                                    Serial.println(led_index);

                                    packetindex += 3;
                              }
                        }
                        //  Serial.print(packagenum);
                        //  Serial.print("/");
                        //  Serial.println(numpackages);

                  }

                  if ((packagenum == numpackages) && (led_index == NUMPIXELS))
                  {
                        pixels.show();
                        led_index == 0;
                  }

            }

      }

}
