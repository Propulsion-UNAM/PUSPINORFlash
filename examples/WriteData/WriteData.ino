#include <PUSPINORFlash.h>

#define CS 1
#define MISO 4
#define MOSI 7
#define SCK 6

PUSPINORFlash flash(CS, SPI);

void printHelp()
{
    Serial.println();
    Serial.println("Commands:");
    Serial.println("w <num>  - write uint32");
    Serial.println("f        - flush");
    Serial.println("m        - show last metadata");
    Serial.println("i        - show internal state");
    Serial.println("ea       - errase all");
    Serial.println("r <sector> -> <pos> - errase all");
    Serial.println("h        - help");
    Serial.println();
}

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

  Serial.println("PUSPINORFlash test");
  printHelp();
}

void loop()
{
  if (!Serial.available())
  {
    return;
  }

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("w "))
  {
    uint32_t value = cmd.substring(2).toInt();

    if (flash.write(value))
    {
      Serial.print("Written: ");
      Serial.println(value);
    } else {
      Serial.println("Write failed");
    }
  } else if (cmd.startsWith("r ")) {
    uint32_t sec = cmd.substring(2).toInt();

    Serial.println("Which position");
    Serial.flush();
    while(!Serial.available());
    String poss = Serial.readStringUntil('\n');
    poss.trim();
    uint32_t pos = poss.toInt();

    uint32_t data = flash.__flash->readULong(sec*DEFAULT_PUNORFLASH_SECTOR_SIZE+pos);
    Serial.print("Data: ");
    Serial.println(data);
  } else if (cmd == "f") {
    if (flash._flush())
    {
      Serial.println("Flush OK");
    } else {
      Serial.println("Flush FAILED");
    }
  } else if (cmd == "m") {
    auto m = flash._lastMetadata();

    Serial.println("Metadata:");
    Serial.print("magic  = 0x");
    Serial.println(m.magic, HEX);

    Serial.print("num    = ");
    Serial.println(m.num);

    Serial.print("sector = ");
    Serial.println(m.sector);

    Serial.print("pos    = ");
    Serial.println(m.pos);

    Serial.print("ecc    = ");
    Serial.println(m.ecc);
  } else if (cmd == "h") {
    printHelp();
  } else if (cmd == "ea") {
    Serial.println("Borrando todo");
    flash.eraseAll();
    Serial.println("Todo borrado");
  } else {
    Serial.println("Unknown command");
  }
}