#include "PUSPINORFlash.h"

#ifndef ARDUINO_ARCH_RP2040
PUSPINORFlash::PUSPINORFlash(uint8_t cs, SPIClass& sspi)
  : flash(cs, &sspi)
{

}

PUSPINORFlash::PUSPINORFlash(uint8_t cs, SPIClass& sspi, uint32_t nmetadatasect, uint32_t sectsiz, uint32_t metamagic)
  : flash(cs, &sspi), nmetadatasectors(nmetadatasect), sectorsize(sectsiz), metadatamagic(metamagic)
{

}
#else
PUSPINORFlash::PUSPINORFlash(uint8_t cs, SPIClassRP2040& sspi)
  : flash(cs, &sspi)
{

}

PUSPINORFlash::PUSPINORFlash(uint8_t cs, SPIClassRP2040& sspi, uint32_t nmetadatasect, uint32_t sectsiz, uint32_t metamagic)
  : flash(cs, &sspi), nmetadatasectors(nmetadatasect), sectorsize(sectsiz), metadatamagic(metamagic)
{

}
#endif

bool PUSPINORFlash::begin(uint32_t flashChipSize)
{
  bool inited = flash.begin(flashChipSize);

  if (inited)
  {
    JEDEC_ID = flash.getJEDECID();
    reportedCapacity = flash.getCapacity();
    uid = flash.getUniqueID();
    Meta lastmeta = _lastMetadata();
    if (lastmeta.magic == metadatamagic)
    {
      sector = lastmeta.sector;
      position = lastmeta.pos;
    }
  }

  __flash = &flash;
  
  return inited;
}

PUSPINORFlash::Meta PUSPINORFlash::_lastMetadata()
{
  Meta lastmeta{};
  lastmeta.num = 0;

  lastmetasector = 0;
  lastmetaposition = 0;

  uint32_t maxnum = 0;

  for (uint32_t sec = 0; sec < nmetadatasectors; sec++)
  {
    uint32_t startaddr = sec * sectorsize;
    for (uint32_t pos = 0; pos <= sectorsize - sizeof(Meta); pos += sizeof(Meta))
    {
      Meta m;
      flash.readAnything(startaddr+pos, m);

      if (m.magic == CLEANSECT)
      {
        break;
      }

      if (m.magic != metadatamagic)
      {
        continue;
      }

      // if (m.ecc != ecc(m))
      // {
      //   continue;
      // }

      if (m.num > maxnum)
      {
        maxnum = m.num;
        lastmeta = m;
        lastmetasector = sec;
        lastmetaposition = pos + sizeof(Meta);
      }
    }
  }

  _lastmeta = lastmeta;
  return lastmeta;
}

bool PUSPINORFlash::_flush()
{
  bool status = false;
  for (uint32_t i = 0; i < buffindex; i += pagesize)
  {
    uint32_t addr = sector * sectorsize + i;
    status = flash.writeByteArray(addr, &buffer[i], min(pagesize, buffindex-i), true);
    if (!status)
    {
      sector++;
      if (sector * sectorsize >= reportedCapacity)
      {
        Meta nm;
        nm.magic = metadatamagic;
        nm.num = _lastmeta.num + 1;
        nm.sector = sector;
        nm.pos = position;
        if (_appendMetadata(nm))
        {
          _lastmeta = nm;
        }
        return false;
      }
    }
  }
  buffindex = 0;
  sector++;
  Meta nm;
  nm.magic = metadatamagic;
  nm.num = _lastmeta.num + 1;
  nm.sector = sector;
  nm.pos = position;
  if (_appendMetadata(nm))
  {
    _lastmeta = nm;
  }
  return status;
}

bool PUSPINORFlash::_appendMetadata(Meta m)
{
  uint32_t addr = lastmetasector * sectorsize + lastmetaposition;
  bool status = flash.writeAnything(addr, m);
  lastmetaposition += sizeof(Meta);
  if (lastmetaposition + sizeof(Meta) > sectorsize)
  {
    lastmetasector++;
    if (lastmetasector >= nmetadatasectors)
    {
      lastmetasector = 0;
    }
    flash.eraseSector(lastmetasector);
    lastmetaposition = 0;
  }
  return status;
}

bool PUSPINORFlash::eraseAll()
{
  #ifdef ERASEALLENABLED
  return flash.eraseChip();
  #endif
  return false;
}

void PUSPINORFlash::dumpSerial()
{
  Serial.println("-----");
  for (uint32_t i = 0; i < reportedCapacity; i++)
  {
    uint8_t bbyt = flash.readByte(i, true);
    Serial.print(bbyt);
  }
  Serial.println();
  Serial.println("-----");
}