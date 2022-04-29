#include <Arduino.h>
#include <CAN.h>

#if DEBUG == 1
#define Sprint(a) (Serial.print(a))
#define SprintBase(a, b) (Serial.print(a, b))
#define Sprintln(a) (Serial.println(a))
#define Sprintf(a, b) (Serial.printf(a, b))
#else
#define Sprint(a)
#define SprintBase(a, b)
#define Sprintln(a)
#define Sprintf(a, b)
#endif

#define CAN_FQ 100E3
#define FILTER_ID 0x130
#define RELAY_PIN 32
#define RESET_INTERVAL_SEC 30

bool power_on = false;

void onCanReceive(int packetSize)
{
  int msg_byte = 0;

  Sprint("packet with id 0x");
  SprintBase(CAN.packetId(), HEX);

  Sprint(" and length ");
  Sprintln(packetSize);

  while (CAN.available())
  {
    int msg_byte_content = CAN.read();

    if (msg_byte == 0 && (msg_byte_content == 0x00 || msg_byte_content == 0x40))
    {
      power_on = false;
    }
    else if (msg_byte == 0 && (msg_byte_content == 0x41 || msg_byte_content == 0x45))
    {
      power_on = true;
    }
    SprintBase(msg_byte_content, HEX);
    msg_byte++;
  }
  Sprintln();
}

void setup()
{
#if DEBUG == 1
  Serial.begin(115200);
#endif
  Sprintln("\nStarting CarCanPower regulator");

  // Initialize relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Initialize CAN controller
  Sprint("Starting CAN: ");
  if (!CAN.begin(CAN_FQ))
  {
    Sprintln("failed");
    delay(RESET_INTERVAL_SEC * 1000);
    ESP.restart();
  }
  Sprintln("success");

  // Setup CAN filtering
  Sprint("Setting CAN filter: ");
  if (!CAN.filter(FILTER_ID, 0x7ff))
  {
    Sprintln("failed");
    delay(RESET_INTERVAL_SEC * 1000);
    ESP.restart();
  }
  Sprintln("success");

  // Register CAN receive callback
  Sprintln("Registering CAN callback");
  CAN.onReceive(onCanReceive);
}

void loop() {
  if(digitalRead(RELAY_PIN) != power_on) {
    digitalWrite(RELAY_PIN, power_on);
  }
}