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
#ifndef CCSDS_DU_hpp
#define CCSDS_DU_hpp

#include "UTIL_DU.hpp"

namespace SPW
{
  namespace PACKET
  {
    ///////////////
    // constants //
    ///////////////
    namespace PROTOCOL_ID
    {
      static const uint8_t EXTENDED = 0;
      static const uint8_t RMAP = 1;
      static const uint8_t CCSDS = 2;
      static const uint8_t GOES = 238;
      static const uint8_t STUP = 239;
      static const uint8_t INVALID = 255;
    }
    static const uint8_t UNKNOWN_LOG_ADDR = 254;

    //-------------------------------------------------------------------------
    class Packet: public UTIL::DU
    //-------------------------------------------------------------------------
    {
    public:
      // constructors and destructur
      Packet();
      // structure of the SpaceWire packet:
      // - SPW address: * bytes (p_spwAddrSize)
      // - SPW data field: * bytes (p_spwDataSize), includes
      //   - logical address: 1 byte
      //   - protocol ID: 1 byte
      //   - other data fields: * bytes, size depends on instruction
      Packet(size_t p_spwAddrSize, size_t p_spwDataSize);
      // the buffer shall contain a correct SpaceWire packet
      Packet(void* p_buffer, size_t p_bufferSize, size_t p_spwAddrSize);
      // the buffer shall contain a correct SpaceWire packet
      Packet(const void* p_buffer,
             size_t p_bufferSize,
             bool p_copyBuffer,
             size_t p_spwAddrSize);
      Packet(const Packet& p_du);
      const Packet& operator=(const Packet& p_du);
      virtual ~Packet();

      // access methods
      virtual size_t getSPWaddrSize() const throw(UTIL::Exception);
      virtual void setSPWaddr(size_t p_byteLength, const void* p_bytes)
        throw(UTIL::Exception);
      virtual const uint8_t* getSPWaddr() const throw(UTIL::Exception);
      virtual void setLogAddr(uint8_t p_logAddr) throw(UTIL::Exception);
      virtual uint8_t getLogAddr() const throw(UTIL::Exception);
      virtual void setProtocolID(uint8_t p_id) throw(UTIL::Exception);
      virtual uint8_t getProtocolID() const throw(UTIL::Exception);
      // includes the logical address and the protocol ID
      virtual size_t getSPWdataSize() const throw(UTIL::Exception);
      // unused bytes in the buffer are kept
      // if the data are smaller than the available buffer
      // includes the logical address and the protocol ID
      virtual void setSPWdata(size_t p_byteLength, const void* p_bytes)
        throw(UTIL::Exception);
      // includes the logical address and the protocol ID
      virtual const uint8_t* getSPWdata() const throw(UTIL::Exception);

    private:
      size_t m_spwAddrSize;
    };

    //-------------------------------------------------------------------------
    class RMAPpacket: public Packet
    //-------------------------------------------------------------------------
    {
    public:
      enum ReplyAddressLength
      {
        BYTES_0 = 0,
        BYTES_4 = 1,
        BYTES_8 = 8,
        BYTES_12 = 12
      };

      enum CommandCode
      {
        CMD_INVALID,
        CMD_READ_SINGLE_ADDR,
        CMD_READ_INCR_ADDR,
        CMD_READ_MOD_WRITE_INCR_ADDR,
        CMD_WRITE_SINGLE_ADDR,
        CMD_WRITE_INCR_ADDR,
        CMD_WRITE_SINGLE_ADDR_RPLY,
        CMD_WRITE_INCR_ADDR_RPLY,
        CMD_WRITE_SINGLE_ADDR_VERIF,
        CMD_WRITE_INCR_ADDR_VERIF,
        CMD_WRITE_SINGLE_ADDR_VERIF_RPLY,
        CMD_WRITE_INCR_ADDR_VERIF_RPLY
      };

      // constructors and destructur
      RMAPpacket();
      // structure of an RMAP packet:
      // - SPW address: * bytes (p_spwAddrSize), from Target or Reply
      // - header field: * bytes, size depends on instruction, includes
      //   - logical address: 1 byte, from Target or Reply
      //   - protocol ID: 1 byte, SPW::PACKET::PROTOCOL_ID::RMAP
      //   - instruction: 1 byte
      //   - other header fields: * bytes, size depends on instruction
      //   - optional data length: 3 bytes, depends on instruction
      //   - header CRC
      // - data field: * bytes (p_dataSize), includes
      //   - data fields: * bytes
      //   - data CRC
      // (EOP) not part of the buffer, added on the transport layer
      RMAPpacket(size_t p_spwAddrSize,
                 uint8_t p_instruction,
                 size_t p_dataSize);
      RMAPpacket(void* p_buffer, size_t p_bufferSize, size_t p_spwAddrSize);
      RMAPpacket(const void* p_buffer,
                 size_t p_bufferSize,
                 bool p_copyBuffer,
                 size_t p_spwAddrSize);
      RMAPpacket(const RMAPpacket& p_du);
      const RMAPpacket& operator=(const RMAPpacket& p_du);
      virtual ~RMAPpacket();

      // access methods
      virtual uint8_t getInstruction() const throw(UTIL::Exception);
      virtual size_t getHeaderSize() const throw(UTIL::Exception);
      virtual const uint8_t* getHeader() const throw(UTIL::Exception);
      virtual void setHeaderCRC() throw(UTIL::Exception);
      virtual uint8_t getHeaderCRC() const throw(UTIL::Exception);
      virtual size_t getDataSize() const throw(UTIL::Exception);
      virtual uint32_t getDataLength() const throw(UTIL::Exception);
      virtual void setData(size_t p_byteLength, const void* p_bytes)
        throw(UTIL::Exception);
      virtual const uint8_t* getData() const throw(UTIL::Exception);
      virtual void setDataCRC() throw(UTIL::Exception);
      virtual uint8_t getDataCRC() const throw(UTIL::Exception);

      // predicates
      static bool isCommand(uint8_t p_instruction);
      static bool isReply(uint8_t p_instruction);
      static bool isRead(uint8_t p_instruction);
      static bool isWrite(uint8_t p_instruction);
      static bool verifyDataBeforeWrite(uint8_t p_instruction);
      static bool hasReply(uint8_t p_instruction);
      static bool incrementAddress(uint8_t p_instruction);

      // helper functions
      static ReplyAddressLength replyAddrLength(uint8_t p_instruction);
      static CommandCode commandCode(uint8_t p_instruction);
      static uint8_t headerSize(uint8_t p_instruction);
      static uint8_t instruction(bool p_isCommand,
                                 ReplyAddressLength p_rplyAddrLength,
                                 CommandCode p_commandCode);
    };

    //-------------------------------------------------------------------------
    class RMAPcommand: public RMAPpacket
    //-------------------------------------------------------------------------
    {
    public:
      // constructors and destructur
      RMAPcommand();
      RMAPcommand(size_t p_targetSPWaddrSize,
                  uint8_t p_instruction,
                  size_t p_dataSize);
      RMAPcommand(void* p_buffer, size_t p_bufferSize, size_t p_targetSPWaddrSize);
      RMAPcommand(const void* p_buffer,
                  size_t p_bufferSize,
                  bool p_copyBuffer,
                  size_t p_targetSPWaddrSize);
      RMAPcommand(const RMAPcommand& p_du);
      const RMAPcommand& operator=(const RMAPcommand& p_du);
      virtual ~RMAPcommand();

      // access methods
      virtual void setReplyAddr(size_t p_byteLength, const void* p_bytes)
        throw(UTIL::Exception);
      virtual size_t getReplyAddrSize() const throw(UTIL::Exception);
      virtual const uint8_t* getReplyAddr() const throw(UTIL::Exception);
      virtual void setInitLogAddr(uint8_t p_logAddr) throw(UTIL::Exception);
      virtual uint8_t getInitLogAddr() const throw(UTIL::Exception);
      virtual void setTransactionID(uint16_t p_transID) throw(UTIL::Exception);
      virtual uint16_t getTransactionID() const throw(UTIL::Exception);
      virtual void setExtendedMemAddr(uint8_t p_extMemAddr) throw(UTIL::Exception);
      virtual uint8_t getExtendedMemAddr() const throw(UTIL::Exception);
      virtual void setMemoryAddr(uint32_t p_memAddr) throw(UTIL::Exception);
      virtual uint32_t getMemoryAddr() const throw(UTIL::Exception);
      virtual void setReadModWriteData(uint32_t p_data) throw(UTIL::Exception);
      virtual uint32_t getReadModWriteData() const throw(UTIL::Exception);
      virtual void setReadModWriteMask(uint32_t p_mask) throw(UTIL::Exception);
      virtual uint32_t getReadModWriteMask() const throw(UTIL::Exception);
    };

    //-------------------------------------------------------------------------
    class RMAPreply: public RMAPpacket
    //-------------------------------------------------------------------------
    {
    public:
      // constructors and destructur
      RMAPreply();
      RMAPreply(const RMAPcommand& p_command, size_t p_dataSize);
      RMAPreply(void* p_buffer, size_t p_bufferSize, size_t p_initSPWaddrSize);
      RMAPreply(const void* p_buffer,
                size_t p_bufferSize,
                bool p_copyBuffer,
                size_t p_initSPWaddrSize);
      RMAPreply(const RMAPreply& p_du);
      const RMAPreply& operator=(const RMAPreply& p_du);
      virtual ~RMAPreply();

      // access methods
      virtual void setStatus(uint8_t p_status) throw(UTIL::Exception);
      virtual uint8_t getStatus() const throw(UTIL::Exception);
      virtual uint8_t getInitLogAddr() const throw(UTIL::Exception);
      virtual uint16_t getTransactionID() const throw(UTIL::Exception);
      virtual void setReadModWriteData(uint32_t p_data) throw(UTIL::Exception);
      virtual uint32_t getReadModWriteData() const throw(UTIL::Exception);
    };
  }
}

#endif