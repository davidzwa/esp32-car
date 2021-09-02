#include "ESP32Adhoc.h"
#include <WiFi.h>
#include <IPAddress.h>
#include <WiFiUdp.h>

#define WIFILED LED_BUILTIN
#define __DEBUG__ 1
const char * udpAddress = "255.255.255.255";
unsigned int udpPort = 44444; // local port to listen on
char ssid[] = "DESKTOP-NM1M0RV 3638";
char password[] = "90W01y9^";

WiFiUDP Udp;
WiFiClient client;
ESP32Adhoc Adhoc;

int TCPBytes = 0;
hw_timer_t *timer = NULL;
bool WLEDState = false;
bool BlinkWLED = false;
enum LEDSTATE
{
  OFF,
  ON,
  BLINK
} ledState;

/*
  WiFi LED timer for blinking state
*/
void IRAM_ATTR onTimer()
{
  if (BlinkWLED)
  {
    if (WLEDState)
    {
      digitalWrite(WIFILED, HIGH);
    }
    else
    {
      digitalWrite(WIFILED, LOW);
    }
    WLEDState = !WLEDState;
  }
}

void initWireless()
{
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".\n");
  }
  Serial.println(" connected");

  Udp.begin(udpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), udpPort);
}

void setup()
{
  Serial.begin(115200);
  delay(10);

#ifdef __DEBUG__
  Serial.println("Booting ESP module");
#endif

  pinMode(WIFILED, OUTPUT);
  digitalWrite(WIFILED, LOW);

  //Setup timer
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function (value in microseconds).
  // Repeat the alarm (third parameter)
  //trigger every 200ms
  timerAlarmWrite(timer, 200000, true);

  // Start an alarm
  timerAlarmEnable(timer);

  initWireless();

#ifdef __DEBUG__
  Serial.println("Setup done ... ");
#endif
}

void loop()
{
  //data will be sent to server
  uint8_t buffer[50] = "hello world";
  //send hello world to server
  Udp.beginPacket(udpAddress, udpPort);
  Udp.write(buffer, 11);
  Udp.endPacket();
  memset(buffer, 0, 50);
  //processing incoming packet, must be called before reading the buffer
  Udp.parsePacket();
  //receive response from server, it will be HELLO WORLD
  if (Udp.read(buffer, 50) > 0)
  {
    Serial.print("Server to client: ");
    Serial.println((char *)buffer);
  }
  //Wait for 1 second
  delay(1000);
}
