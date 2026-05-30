#include <PUSPINORFlash.h>

#define CS 1
#define MISO 4
#define MOSI 7
#define SCK 6

#define ERASEALLENABLED

PUSPINORFlash flash(CS, SPI);

void setup()
{
  Serial.begin(115200);

  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  SPI.setSCK(SCK);
  SPI.begin();

  while (!Serial);

  if (!flash.begin(MB(32)))
  {
    Serial.println("Flash init failed");
    while (1);
  }

  Serial.println("PUSPINORFlash dump and clean");
  Serial.println("Are you sure you want to dump and clean? (OK / NO))");
}

void loop()
{
  if (!Serial.available())
  {
    return;
  }

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd == "OK")
  {
    flash.dumpSerial();
    Serial.println("Confirm to ERASE ALL DATA (IRREVERSIBLE) (OK / NO)");
    Serial.flush();

    while(!Serial.available());
    String cmd2 = Serial.readStringUntil('\n');
    cmd2.trim();

    if (cmd2 == "OK")
    {
      Serial.println("Errasing all...");
      flash.eraseAll();
      Serial.println("All data erased");
    } else {
      Serial.println("Stopping...");
      while(1);
    }
  } else {
    Serial.println("Stopping...");
    while(1);
  }
}