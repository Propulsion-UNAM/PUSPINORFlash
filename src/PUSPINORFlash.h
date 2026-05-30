#ifndef PUSPINORFLASH_H
#define PUSPINORFLASH_H

#include <Arduino.h>
#include <SPIMemory.h>

#define CLEANSECT 0xFFFFFFFF

#define DEFAULT_PUNORFLASH_SECTOR_SIZE 4096
#define DEFAULT_PUNORFLASH_BUFFER_SIZE 4096
#define DEFAULT_PUNORFLASH_PAGE_SIZE 256

#define RUNDIAGNOSTIC

#define ERASEALLENABLED

class PUSPINORFlash
{
  public:
  #pragma pack(push, 1)
  struct Meta
  {
    uint32_t magic;
    uint32_t num;
    uint32_t sector;
    uint32_t pos;
    uint32_t ecc;
  };
  #pragma pack(pop)

  private:
  uint8_t buffer[DEFAULT_PUNORFLASH_BUFFER_SIZE];
  uint32_t buffindex = 0;
  
  const uint32_t nmetadatasectors = 16;
  const uint32_t sectorsize = DEFAULT_PUNORFLASH_SECTOR_SIZE;
  const uint32_t metadatamagic = 0x50555055;
  const uint32_t pagesize = DEFAULT_PUNORFLASH_PAGE_SIZE;

  uint32_t sector = nmetadatasectors;
  uint32_t position = 0;

  Meta _lastmeta{};
  uint32_t lastmetasector = 0;
  uint32_t lastmetaposition = 0;

  SPIFlash flash;

  bool _appendMetadata(Meta m);

  public:
  PUSPINORFlash(uint8_t cs, SPIClass& sspi);
  PUSPINORFlash(uint8_t cs, SPIClass& sspi, uint32_t nmetadatasect, uint32_t sectsiz = DEFAULT_PUNORFLASH_SECTOR_SIZE, uint32_t metamagic = 0x50555055);
  PUSPINORFlash(uint8_t cs, SPIClassRP2040& sspi);
  PUSPINORFlash(uint8_t cs, SPIClassRP2040& sspi, uint32_t nmetadatasect, uint32_t sectsiz = DEFAULT_PUNORFLASH_SECTOR_SIZE, uint32_t metamagic = 0x50555055);

  // TODO: Reestructure as (size, units)
  bool begin(uint32_t flashChipSize = 0);

  uint32_t JEDEC_ID;
  uint32_t reportedCapacity = 0;
  uint64_t uid;

  Meta _lastMetadata();
  // TODO: partial writes
  bool _flush();

  template <typename T>
  bool write(const T &data)
  {
    bool status = true;
    
    if (sizeof(T) > DEFAULT_PUNORFLASH_BUFFER_SIZE)
    {
        return false;
    }

    if (buffindex + sizeof(T) > DEFAULT_PUNORFLASH_BUFFER_SIZE)
    {
      status = _flush();
    }

    uint8_t* ptr = (uint8_t*)&data;

    if (status)
    {
      for (uint32_t i = 0; i < sizeof(T); i++)
      {
        buffer[buffindex] = ptr[i];
        buffindex++;
      }
    }

    return status;
  }

  bool eraseAll();

  SPIFlash* __flash;
};

#endif