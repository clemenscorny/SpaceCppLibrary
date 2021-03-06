//*****************************************************************************
// (C) 2014, Stefan Korner, Austria                                           *
//                                                                            *
// The Space C++ Library is free software; you can redistribute it and/or     *
// modify it under the terms of the GNU Lesser General Public License as      *
// published by the Free Software Foundation; either version 2.1 of the       *
// License, or (at your option) any later version.                            *
//                                                                            *
// The Space C++ Library is distributed in the hope that it will be useful,   *
// but WITHOUT ANY WARRANTY; without even the implied warranty of             *
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser    *
// General Public License for more details.                                   *
//*****************************************************************************
// SpaceWire - SpaceWire Packet Module                                        *
//*****************************************************************************
#include "SPW_PACKET.hpp"

#include "UTIL_CRC.hpp"

//////////////////////
// global functions //
//////////////////////

//-----------------------------------------------------------------------------
bool
SPW::PACKET::isA(uint8_t p_protocolID, void* p_buffer, size_t p_bufferSize)
//-----------------------------------------------------------------------------
{
  if(p_bufferSize < 2)
  {
    return false;
  }
  uint8_t* buffer = (uint8_t*) p_buffer;
  return (p_protocolID == buffer[2]);
}

////////////
// Packet //
////////////

//-----------------------------------------------------------------------------
SPW::PACKET::Packet::Packet(): m_spwAddrSize(0)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::Packet::Packet(size_t p_spwAddrSize, size_t p_spwDataSize):
  UTIL::DU::DU(p_spwAddrSize + p_spwDataSize), m_spwAddrSize(p_spwAddrSize)
//-----------------------------------------------------------------------------
{
  // these method can throw exceptions but this should not happen,
  // because the buffer is allocated with proper size
  setLogAddr(SPW::PACKET::UNKNOWN_LOG_ADDR);
  setProtocolID(SPW::PACKET::PROTOCOL_ID::INVALID);
}

//-----------------------------------------------------------------------------
SPW::PACKET::Packet::Packet(void* p_buffer,
                            size_t p_bufferSize,
                            size_t p_spwAddrSize):
  UTIL::DU::DU(p_buffer, p_bufferSize), m_spwAddrSize(p_spwAddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::Packet::Packet(const void* p_buffer,
                            size_t p_bufferSize,
                            bool p_copyBuffer,
                            size_t p_spwAddrSize):
  UTIL::DU::DU(p_buffer, p_bufferSize, p_copyBuffer),
  m_spwAddrSize(p_spwAddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::Packet::Packet(const SPW::PACKET::Packet& p_du):
  UTIL::DU::DU(p_du), m_spwAddrSize(p_du.m_spwAddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
const SPW::PACKET::Packet& SPW::PACKET::Packet::operator=(
  const SPW::PACKET::Packet& p_du)
//-----------------------------------------------------------------------------
{
  if(&p_du != this)
  {
    UTIL::DU::operator=(p_du);
    m_spwAddrSize = p_du.m_spwAddrSize;
  }
  return *this;
}

//-----------------------------------------------------------------------------
SPW::PACKET::Packet::~Packet()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
size_t SPW::PACKET::Packet::getSPWaddrSize() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  return m_spwAddrSize;
}

//-----------------------------------------------------------------------------
void SPW::PACKET::Packet::setSPWaddr(size_t p_byteLength, const void* p_bytes)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("SPW packet is configured for Read Only");
  }
  // the buffer must have at least the size of the SPW address
  if(p_byteLength > bufferSize())
  {
    throw UTIL::Exception("SPW address is too large for available packet buffer");
  }
  // copy the address
  setBytes(0, p_byteLength, p_bytes);
}

//-----------------------------------------------------------------------------
const uint8_t* SPW::PACKET::Packet::getSPWaddr() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  return buffer();
}

//-----------------------------------------------------------------------------
void SPW::PACKET::Packet::setLogAddr(uint8_t p_logAddr) throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("SPW packet is configured for Read Only");
  }
  // the SPW data size must be least 1 bytes large
  if(getSPWdataSize() < 1)
  {
    throw UTIL::Exception("Logical address does not fit into SPW data field");
  }
  // copy the address
  (*this)[m_spwAddrSize] = p_logAddr;  
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::Packet::getLogAddr() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // the SPW data size must be least 1 bytes large
  if(getSPWdataSize() < 1)
  {
    throw UTIL::Exception("Logical address does not fit into SPW data field");
  }
  // fetch the address
  return (*this)[m_spwAddrSize];  
}

//-----------------------------------------------------------------------------
void SPW::PACKET::Packet::setProtocolID(uint8_t p_id) throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("SPW packet is configured for Read Only");
  }
  // the SPW data size must be least 2 bytes large
  // (with logical address and protocol ID)
  if(getSPWdataSize() < 2)
  {
    throw UTIL::Exception("Protocol ID does not fit into SPW data field");
  }
  // copy the protocol ID
  (*this)[m_spwAddrSize + 1] = p_id;  
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::Packet::getProtocolID() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // the SPW data size must be least 2 bytes large
  // (with logical address and protocol ID)
  if(getSPWdataSize() < 2)
  {
    throw UTIL::Exception("Protocol ID does not fit into SPW data field");
  }
  // fetch the protocol ID
  return (*this)[m_spwAddrSize + 1];  
}

//-----------------------------------------------------------------------------
size_t SPW::PACKET::Packet::getSPWdataSize() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // the buffer must have at least the size of the SPW address
  if(m_spwAddrSize > bufferSize())
  {
    throw UTIL::Exception("Data out of SPW packet buffer");
  }
  return (bufferSize() - m_spwAddrSize);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::Packet::setSPWdata(size_t p_byteLength, const void* p_bytes)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("SPW packet is configured for Read Only");
  }
  // the existing buffer must have at least the size for the passed data
  if(p_byteLength > getSPWdataSize())
  {
    throw UTIL::Exception("Data do not fit into SPW packet buffer");
  }
  // copy the data
  return setBytes(m_spwAddrSize, p_byteLength, p_bytes);
}

//-----------------------------------------------------------------------------
const uint8_t* SPW::PACKET::Packet::getSPWdata() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // this checks also the consistency of the buffer
  // and throws an exception if there are no data
  size_t spwDataSize = getSPWdataSize();
  return getBytes(m_spwAddrSize, spwDataSize);
}

////////////////
// RMAPpacket //
////////////////

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::RMAPpacket()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::RMAPpacket(size_t p_spwAddrSize,
                                    uint8_t p_instruction,
                                    size_t p_dataLength):
  SPW::PACKET::Packet::Packet(p_spwAddrSize,
                              hasData(p_instruction) ?
                              (headerSize(p_instruction) + p_dataLength + 1) :
                              headerSize(p_instruction)) 
//-----------------------------------------------------------------------------
{
  // set protocol ID and instruction
  (*this)[p_spwAddrSize + 1] = SPW::PACKET::PROTOCOL_ID::RMAP;
  (*this)[p_spwAddrSize + 2] = p_instruction;
  // initialise the data length field if the instruction requires a data field
  if(hasDataLength(p_instruction))
  {
    // set the data length
    size_t dataLengthPos = p_spwAddrSize + headerSize(p_instruction) - 4;
    setUnsigned(dataLengthPos, 3, p_dataLength);
  }
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::RMAPpacket(void* p_buffer,
                                    size_t p_bufferSize,
                                    size_t p_spwAddrSize):
  SPW::PACKET::Packet::Packet(p_buffer, p_bufferSize, p_spwAddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::RMAPpacket(const void* p_buffer,
                                    size_t p_bufferSize,
                                    bool p_copyBuffer,
                                    size_t p_spwAddrSize):
  SPW::PACKET::Packet::Packet(
    p_buffer, p_bufferSize, p_copyBuffer, p_spwAddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::RMAPpacket(const SPW::PACKET::RMAPpacket& p_du):
  SPW::PACKET::Packet::Packet(p_du)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
const SPW::PACKET::RMAPpacket&
SPW::PACKET::RMAPpacket::operator=(const SPW::PACKET::RMAPpacket& p_du)
//-----------------------------------------------------------------------------
{
  SPW::PACKET::Packet::operator=(p_du);
  return *this;
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::~RMAPpacket()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
size_t SPW::PACKET::RMAPpacket::getHeaderSize() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  size_t spwDataSize = getSPWdataSize();
  if(spwDataSize < 3)
  {
    throw UTIL::Exception("Instruction does not fit into SPW packet buffer");
  }
  uint8_t instruction = (*this)[getSPWaddrSize() + 2];
  size_t hdrSize = headerSize(instruction);
  // header starts at the beginning of the SPW data field
  if(hdrSize > spwDataSize)
  {
    throw UTIL::Exception("Encoded RMAP header does not fit into SPW packet buffer");
  }
  return hdrSize;
}

//-----------------------------------------------------------------------------
const uint8_t* SPW::PACKET::RMAPpacket::getHeader() const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  getHeaderSize();
  // fetch the header (starts at the beginning of the SPW data field)
  return getSPWdata();
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::getInstruction() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  getHeaderSize();
  // fetch the protocol ID
  return (*this)[getSPWaddrSize() + 2];
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setSpecialByte(uint8_t p_byte)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("RMAP packet is configured for Read Only");
  }
  // force a check of the header size
  getHeaderSize();
  // copy the special byte
  (*this)[getSPWaddrSize() + 3] = p_byte;  
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::getSpecialByte() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  getHeaderSize();
  // fetch the special byte
  return (*this)[getSPWaddrSize() + 3];
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setSenderLogAddr(uint8_t p_logAddr)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("RMAP packet is configured for Read Only");
  }
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the sender logical address
  size_t instruction = getInstruction();
  size_t sndLogAddrPos = getSPWaddrSize() + headerSize;
  if(isCommand(instruction))
  {
    sndLogAddrPos -= 12;
  }
  else if(isWrite(instruction))
  {
    // write response
    sndLogAddrPos -= 4;
  }
  else
  {
    // read or read-modify-write response
    sndLogAddrPos -= 8;
  }
  (*this)[sndLogAddrPos] = p_logAddr;
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::getSenderLogAddr() const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the sender logical address
  size_t instruction = getInstruction();
  size_t sndLogAddrPos = getSPWaddrSize() + headerSize;
  if(isCommand(instruction))
  {
    sndLogAddrPos -= 12;
  }
  else if(isWrite(instruction))
  {
    // write response
    sndLogAddrPos -= 4;
  }
  else
  {
    // read or read-modify-write response
    sndLogAddrPos -= 8;
  }
  return (*this)[sndLogAddrPos];
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setTransactionID(uint16_t p_transID)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("RMAP packet is configured for Read Only");
  }
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the transaction ID
  size_t instruction = getInstruction();
  size_t transIDpos = getSPWaddrSize() + headerSize;
  if(isCommand(instruction))
  {
    transIDpos -= 11;
  }
  else if(isWrite(instruction))
  {
    // write response
    transIDpos -= 3;
  }
  else
  {
    // read or read-modify-write response
    transIDpos -= 7;
  }
  setUnsigned(transIDpos, 2, p_transID);
}

//-----------------------------------------------------------------------------
uint16_t SPW::PACKET::RMAPpacket::getTransactionID() const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the transaction ID
  size_t instruction = getInstruction();
  size_t transIDpos = getSPWaddrSize() + headerSize;
  if(isCommand(instruction))
  {
    transIDpos -= 11;
  }
  else if(isWrite(instruction))
  {
    // write response
    transIDpos -= 3;
  }
  else
  {
    // read or read-modify-write response
    transIDpos -= 7;
  }
  return ((uint16_t) getUnsigned(transIDpos, 2));
}

//-----------------------------------------------------------------------------
uint32_t SPW::PACKET::RMAPpacket::getDataLength() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // check if the instruction requires a data length field
  if(!hasDataLength(getInstruction()))
  {
    throw UTIL::Exception("Instruction does not define a data length field");
  }
  // fetch the data length
  size_t dataLengthPos = getSPWaddrSize() + headerSize - 4;
  return getUnsigned(dataLengthPos, 3);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setHeaderCRC() throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("RMAP packet is configured for Read Only");
  }
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // calculate CRC exclusive CRC byte
  size_t crcPosInHeader = headerSize - 1;
  uint8_t crc = UTIL::CRC::calculate8(getHeader(), crcPosInHeader);
  // set the header CRC
  size_t crcPosInDU = getSPWaddrSize() + headerSize - 1;
  (*this)[crcPosInDU] = crc;
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::getHeaderCRC() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the header CRC
  size_t crcPosInDU = getSPWaddrSize() + headerSize - 1;
  return (*this)[crcPosInDU];
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::checkHeaderCRC() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // calculate CRC inclusive CRC byte
  uint8_t crc = UTIL::CRC::calculate8(getHeader(), headerSize);
  return (crc == 0);
}

//-----------------------------------------------------------------------------
size_t SPW::PACKET::RMAPpacket::getDataSize() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  size_t bufSize = bufferSize();
  size_t dataPos = getSPWaddrSize() + getHeaderSize();
  if(dataPos > bufSize)
  {
    throw UTIL::Exception("Inconsistend RMAP data field size");
  }
  return (bufSize - dataPos);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setData(size_t p_byteLength, const void* p_bytes)
        throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // this checks also the consistency of the buffer
  // and throws an exception if there are no data
  if(p_byteLength > getDataLength())
  {
    throw UTIL::Exception("Data do not fit into RMAP data field");
  }
  size_t dataPos = getSPWaddrSize() + getHeaderSize();
  setBytes(dataPos, p_byteLength, p_bytes);
}

//-----------------------------------------------------------------------------
const uint8_t* SPW::PACKET::RMAPpacket::getData() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("RMAP packet is configured for Read Only");
  }
  // this checks also the consistency of the buffer
  // and throws an exception if there are no data
  size_t dataPos = getSPWaddrSize() + getHeaderSize();
  size_t dataLength = getDataLength();
  return getBytes(dataPos, dataLength);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setDataByte(size_t p_bytePos, uint8_t p_byte)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  setUnsigned(dataPos, 1, p_byte);
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::getDataByte(size_t p_bytePos) const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  return ((uint8_t) getUnsigned(dataPos, 1));
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setDataWord(size_t p_bytePos, uint16_t p_word)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  setUnsigned(dataPos, 2, p_word);
}

//-----------------------------------------------------------------------------
uint16_t SPW::PACKET::RMAPpacket::getDataWord(size_t p_bytePos) const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  return ((uint16_t) getUnsigned(dataPos, 2));
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setData3Bytes(size_t p_bytePos, 
                                            uint32_t p_3Bytes)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  setUnsigned(dataPos, 3, p_3Bytes);
}

//-----------------------------------------------------------------------------
uint32_t SPW::PACKET::RMAPpacket::getData3Bytes(size_t p_bytePos) const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  return getUnsigned(dataPos, 3);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setDataLongWord(size_t p_bytePos,
                                              uint32_t p_longWord)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  setUnsigned(dataPos, 4, p_longWord);
}

//-----------------------------------------------------------------------------
uint32_t SPW::PACKET::RMAPpacket::getDataLongWord(size_t p_bytePos) const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the data
  size_t dataPos = getSPWaddrSize() + headerSize + p_bytePos;
  return getUnsigned(dataPos, 4);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPpacket::setDataCRC() throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // check if there is a writable buffer with proper size
  if(bufferIsReadonly())
  {
    throw UTIL::Exception("RMAP packet is configured for Read Only");
  }
  // the RMAP data size must be least 1 byte large
  size_t dataSize = getDataSize();
  if(dataSize < 1)
  {
    throw UTIL::Exception("Data CRC does not fit into RMAP data field");
  }
  // calculate CRC exclusive CRC byte
  size_t crcPosInData = dataSize - 1;
  uint8_t crc = UTIL::CRC::calculate8(getData(), crcPosInData);
  // set the header CRC
  size_t crcPosInDU = getSPWaddrSize() + getHeaderSize() + dataSize - 1;
  (*this)[crcPosInDU] = crc;
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::getDataCRC() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // the RMAP data size must be least 1 byte large
  size_t dataSize = getDataSize();
  if(dataSize < 1)
  {
    throw UTIL::Exception("Data CRC does not fit into RMAP data field");
  }
  // fetch the data CRC
  size_t crcPosInDU = getSPWaddrSize() + getHeaderSize() + dataSize - 1;
  return (*this)[crcPosInDU];
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::checkDataCRC() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  size_t dataSize = getDataSize();
  if(dataSize < 1)
  {
    throw UTIL::Exception("Data CRC does not fit into RMAP data field");
  }
  // calculate CRC inclusive CRC byte
  uint8_t crc = UTIL::CRC::calculate8(getData(), dataSize);
  // set the header CRC
  return (crc == 0);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::isCommand(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x40) == 0x40);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::isReply(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x40) == 0x00);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::isWrite(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x20) == 0x20);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::isRead(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x38) == 0x08);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::isReadModWrite(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x3C) == 0x1C);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::verifyDataBeforeWrite(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x10) == 0x10);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::hasReply(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x08) == 0x08);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::hasDataLength(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  if(isCommand(p_instruction))
  {
    return true;
  }
  if(!isWrite(p_instruction))
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::hasData(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  if(isCommand(p_instruction) && isWrite(p_instruction))
  {
    return true;
  }
  if(isReply(p_instruction) && isRead(p_instruction))
  {
    return true;
  }
  if(isReadModWrite(p_instruction))
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::incrementAddress(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  return ((p_instruction & 0x04) == 0x04);
}

//-----------------------------------------------------------------------------
bool SPW::PACKET::RMAPpacket::isOK(uint8_t p_status)
//-----------------------------------------------------------------------------
{
  return (p_status == SPW::PACKET::RMAPpacket::CMD_OK);
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::ReplyAddressLength
SPW::PACKET::RMAPpacket::replyAddrLength(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  switch(p_instruction & 0x03)
  {
  case 0x00: return BYTES_0;
  case 0x01: return BYTES_4;
  case 0x02: return BYTES_8;
  case 0x03: return BYTES_12;
  }
  // should not be reached (to satisfy the compiler)
  return BYTES_0;
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::CommandCode
SPW::PACKET::RMAPpacket::commandCode(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  switch(p_instruction & 0x3C)
  {
  case 0x00: return CMD_INVALID;
  case 0x04: return CMD_INVALID;
  case 0x08: return CMD_READ_SINGLE_ADDR;
  case 0x0C: return CMD_READ_INCR_ADDR;
  case 0x10: return CMD_INVALID;
  case 0x14: return CMD_INVALID;
  case 0x18: return CMD_INVALID;
  case 0x1C: return CMD_READ_MOD_WRITE_INCR_ADDR;
  case 0x20: return CMD_WRITE_SINGLE_ADDR;
  case 0x24: return CMD_WRITE_INCR_ADDR;
  case 0x28: return CMD_WRITE_SINGLE_ADDR_RPLY;
  case 0x2C: return CMD_WRITE_INCR_ADDR_RPLY;
  case 0x30: return CMD_WRITE_SINGLE_ADDR_VERIF;
  case 0x34: return CMD_WRITE_INCR_ADDR_VERIF;
  case 0x38: return CMD_WRITE_SINGLE_ADDR_VERIF_RPLY;
  case 0x3C: return CMD_WRITE_INCR_ADDR_VERIF_RPLY;
  }
  // should not be reached (to satisfy the compiler)
  return CMD_INVALID;
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::headerSize(uint8_t p_instruction)
//-----------------------------------------------------------------------------
{
  if(isCommand(p_instruction))
  {
    return (replyAddrLength(p_instruction) + 16);
  }
  else
  {
    // reply
    if(isWrite(p_instruction))
    {
      return 8;
    }
    else
    {
      return 12;
    }
  }
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPpacket::instruction(
  CommandCode p_commandCode,
  ReplyAddressLength p_rplyAddrLength,
  bool p_isCommand)
//-----------------------------------------------------------------------------
{
  uint8_t retVal = (p_isCommand ? 0x40 : 0x00);
  switch(p_rplyAddrLength)
  {
  case BYTES_0:                  break;
  case BYTES_4:  retVal |= 0x01; break;
  case BYTES_8:  retVal |= 0x02; break;
  case BYTES_12: retVal |= 0x03; break;
  }
  switch(p_commandCode)
  {
  case CMD_INVALID:                                      break;
  case CMD_READ_SINGLE_ADDR:             retVal |= 0x08; break;
  case CMD_READ_INCR_ADDR:               retVal |= 0x0C; break;
  case CMD_READ_MOD_WRITE_INCR_ADDR:     retVal |= 0x1C; break;
  case CMD_WRITE_SINGLE_ADDR:            retVal |= 0x20; break;
  case CMD_WRITE_INCR_ADDR:              retVal |= 0x24; break;
  case CMD_WRITE_SINGLE_ADDR_RPLY:       retVal |= 0x28; break;
  case CMD_WRITE_INCR_ADDR_RPLY:         retVal |= 0x2C; break;
  case CMD_WRITE_SINGLE_ADDR_VERIF:      retVal |= 0x30; break;
  case CMD_WRITE_INCR_ADDR_VERIF:        retVal |= 0x34; break;
  case CMD_WRITE_SINGLE_ADDR_VERIF_RPLY: retVal |= 0x38; break;
  case CMD_WRITE_INCR_ADDR_VERIF_RPLY:   retVal |= 0x3C; break;
  }
  return retVal;
}

//-----------------------------------------------------------------------------
const char* SPW::PACKET::RMAPpacket::errorTxt(uint8_t p_status)
//-----------------------------------------------------------------------------
{
  switch(p_status)
  {
  case SPW::PACKET::RMAPpacket::CMD_OK:
    return "Command executed successfully";
  case SPW::PACKET::RMAPpacket::ERR_GENERAL_ERROR_CODE:
    return "General error code";
  case SPW::PACKET::RMAPpacket::ERR_UNUSED_RMAP_PKT_TYPE_OR_CMD_CODE:
    return "Unused RMAP Packet Type or Command Code";
  case SPW::PACKET::RMAPpacket::ERR_INVALID_KEY:
    return "Invalid key";
  case SPW::PACKET::RMAPpacket::ERR_INVALID_DATA_CRC:
    return "Invalid Data CRC";
  case SPW::PACKET::RMAPpacket::ERR_EARLY_EOP:
    return "Early EOP";
  case SPW::PACKET::RMAPpacket::ERR_TOO_MUCH_DATA:
    return "Too much data";
  case SPW::PACKET::RMAPpacket::ERR_EEP:
    return "EEP";
  case SPW::PACKET::RMAPpacket::ERR_VERIFY_BUFFER_OVERRUN:
    return "Verify buffer overrun";
  case SPW::PACKET::RMAPpacket::ERR_RMAP_CMD_NOT_IMPL_OR_NOT_AUTH:
    return "RMAP Command not implemented or not authorised";
  case SPW::PACKET::RMAPpacket::ERR_RMW_DATA_LENGTH_ERROR:
    return "RMW Data Length error";
  case SPW::PACKET::RMAPpacket::ERR_INVALID_TARGET_LOGICAL_ADDRESS:
    return "Invalid Target Logical Address";
  }
  return "Reserved";
}

/////////////////
// RMAPcommand //
/////////////////

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPcommand::RMAPcommand()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
// ensure that the RMPApacket is really a command
SPW::PACKET::RMAPcommand::RMAPcommand(size_t p_targetSPWaddrSize,
                                      uint8_t p_instruction,
                                      size_t p_dataSize):
  SPW::PACKET::RMAPpacket::RMAPpacket(
    p_targetSPWaddrSize, p_instruction | 0x40, p_dataSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPcommand::RMAPcommand(void* p_buffer,
                                      size_t p_bufferSize,
                                      size_t p_targetSPWaddrSize):
  SPW::PACKET::RMAPpacket::RMAPpacket(
    p_buffer, p_bufferSize, p_targetSPWaddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPcommand::RMAPcommand(const void* p_buffer,
                                      size_t p_bufferSize,
                                      bool p_copyBuffer,
                                      size_t p_targetSPWaddrSize):
  SPW::PACKET::RMAPpacket::RMAPpacket(
    p_buffer, p_bufferSize, p_copyBuffer, p_targetSPWaddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPcommand::RMAPcommand(const SPW::PACKET::RMAPcommand& p_du):
  SPW::PACKET::RMAPpacket::RMAPpacket(p_du)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
const SPW::PACKET::RMAPcommand&
SPW::PACKET::RMAPcommand::operator=(const SPW::PACKET::RMAPcommand& p_du)
//-----------------------------------------------------------------------------
{
  SPW::PACKET::RMAPpacket::operator=(p_du);
  return *this;
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPcommand::~RMAPcommand()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPcommand::setKey(uint8_t p_key) throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  setSpecialByte(p_key);
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPcommand::getKey() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  return getSpecialByte();
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPpacket::ReplyAddressLength
SPW::PACKET::RMAPcommand::getReplyAddrLength() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  if(!hasReply(getInstruction()))
  {
    throw UTIL::Exception("Instruction does not have a reply");
  }
  return replyAddrLength(getInstruction());
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPcommand::setReplyAddr(size_t p_byteLength,
                                            const void* p_bytes)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  getHeaderSize();
  // force a check of the reply address existence
  size_t replyAddrLength = getReplyAddrLength();
  if(replyAddrLength != p_byteLength)
  {
    throw UTIL::Exception("Reply address has wrong size");
  }
  // copy the reply address
  size_t replyAddrPos = getSPWaddrSize() + 4;
  setBytes(replyAddrPos, p_byteLength, p_bytes);
}

//-----------------------------------------------------------------------------
const uint8_t* SPW::PACKET::RMAPcommand::getReplyAddr() const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  getHeaderSize();
  // force a check of the reply address existence
  size_t replyAddrLength = getReplyAddrLength();
  // fetch the reply address
  size_t replyAddrPos = getSPWaddrSize() + 4;
  return getBytes(replyAddrPos, replyAddrLength);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPcommand::setInitLogAddr(uint8_t p_logAddr)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  setSenderLogAddr(p_logAddr);
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPcommand::getInitLogAddr() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  return getSenderLogAddr();
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPcommand::setExtendedMemAddr(uint8_t p_extMemAddr)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the extended memory address
  size_t extMemAddrPos = getSPWaddrSize() + headerSize - 9;
  setUnsigned(extMemAddrPos, 4, p_extMemAddr);
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPcommand::getExtendedMemAddr() const
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the extended memory address
  size_t extMemAddrPos = getSPWaddrSize() + headerSize - 9;
  return getUnsigned(extMemAddrPos, 4);
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPcommand::setMemoryAddr(uint32_t p_memAddr)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // copy the memory address
  size_t memoryAddrPos = getSPWaddrSize() + headerSize - 8;
  setUnsigned(memoryAddrPos, 4, p_memAddr);
}

//-----------------------------------------------------------------------------
uint32_t SPW::PACKET::RMAPcommand::getMemoryAddr() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  // force a check of the header size
  size_t headerSize = getHeaderSize();
  // fetch the memory address
  size_t memoryAddrPos = getSPWaddrSize() + headerSize - 8;
  return getUnsigned(memoryAddrPos, 4);
}

///////////////
// RMAPreply //
///////////////

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPreply::RMAPreply()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
// ensure that the RMPApacket is really a reply
SPW::PACKET::RMAPreply::RMAPreply(const SPW::PACKET::RMAPcommand& p_command):
  SPW::PACKET::RMAPpacket::RMAPpacket(
    p_command.getReplyAddrLength(),
    p_command.getInstruction() & 0xBF,
    SPW::PACKET::RMAPpacket::isReadModWrite(p_command.getInstruction()) ?
      (p_command.getDataLength() / 2) :
      p_command.getDataLength())
//-----------------------------------------------------------------------------
{
  // copy data from the command into the reply
  // there is some risk of exceptions that are suppressed protect the ctor
  try
  {
    setSPWaddr(p_command.getReplyAddrLength(), p_command.getReplyAddr());
    setLogAddr(p_command.getInitLogAddr());
    // protocol ID is already copied in the SPW ctor
    // instruction is already copied in the RMAP ctor
    setTargetLogAddr(p_command.getInitLogAddr());
    setTransactionID(p_command.getTransactionID());
  }
  catch(...) {}
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPreply::RMAPreply(void* p_buffer,
                                  size_t p_bufferSize,
                                  size_t p_initSPWaddrSize):
  SPW::PACKET::RMAPpacket::RMAPpacket(
    p_buffer, p_bufferSize, p_initSPWaddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPreply::RMAPreply(const void* p_buffer,
                                  size_t p_bufferSize,
                                  bool p_copyBuffer,
                                  size_t p_initSPWaddrSize):
  SPW::PACKET::RMAPpacket::RMAPpacket(
    p_buffer, p_bufferSize, p_copyBuffer, p_initSPWaddrSize)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPreply::RMAPreply(const SPW::PACKET::RMAPreply& p_du):
  SPW::PACKET::RMAPpacket::RMAPpacket(p_du)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
const SPW::PACKET::RMAPreply&
SPW::PACKET::RMAPreply::operator=(const SPW::PACKET::RMAPreply& p_du)
//-----------------------------------------------------------------------------
{
  SPW::PACKET::RMAPpacket::operator=(p_du);
  return *this;
}

//-----------------------------------------------------------------------------
SPW::PACKET::RMAPreply::~RMAPreply()
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPreply::setStatus(uint8_t p_status) throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  setSpecialByte(p_status);
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPreply::getStatus() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  return getSpecialByte();
}

//-----------------------------------------------------------------------------
void SPW::PACKET::RMAPreply::setTargetLogAddr(uint8_t p_logAddr)
  throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  setSenderLogAddr(p_logAddr);
}

//-----------------------------------------------------------------------------
uint8_t SPW::PACKET::RMAPreply::getTargetLogAddr() const throw(UTIL::Exception)
//-----------------------------------------------------------------------------
{
  return getSenderLogAddr();
}

