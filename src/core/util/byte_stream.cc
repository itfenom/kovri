/**                                                                                           //
 * Copyright (c) 2013-2017, The Kovri I2P Router Project                                      //
 *                                                                                            //
 * All rights reserved.                                                                       //
 *                                                                                            //
 * Redistribution and use in source and binary forms, with or without modification, are       //
 * permitted provided that the following conditions are met:                                  //
 *                                                                                            //
 * 1. Redistributions of source code must retain the above copyright notice, this list of     //
 *    conditions and the following disclaimer.                                                //
 *                                                                                            //
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list     //
 *    of conditions and the following disclaimer in the documentation and/or other            //
 *    materials provided with the distribution.                                               //
 *                                                                                            //
 * 3. Neither the name of the copyright holder nor the names of its contributors may be       //
 *    used to endorse or promote products derived from this software without specific         //
 *    prior written permission.                                                               //
 *                                                                                            //
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY        //
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF    //
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL     //
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       //
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,               //
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    //
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,          //
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF    //
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               //
 *                                                                                            //
 * Parts of the project are originally copyright (c) 2013-2015 The PurpleI2P Project          //
 */

#include "core/util/byte_stream.h"

#include <cstring>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "core/util/i2p_endian.h"
#include "core/util/log.h"

namespace kovri {
namespace core {

/// Input

InputByteStream::InputByteStream(
    std::uint8_t* data,
    std::size_t len)
    : m_Data(data),
      m_Length(len) {}

void InputByteStream::ConsumeData(
    std::size_t amount) {
  if (amount > m_Length)
    throw std::length_error("InputByteStream: too many bytes to consume.");
  m_Data += amount;
  m_Length -= amount;
}

std::uint8_t* InputByteStream::ReadBytes(
    std::size_t amount) {
  std::uint8_t* ptr = m_Data;
  ConsumeData(amount);
  return ptr;
}

std::uint64_t InputByteStream::ReadUInt64() {
  return bufbe64toh(ReadBytes(sizeof(std::uint64_t)));
}

std::uint32_t InputByteStream::ReadUInt32() {
  return bufbe32toh(ReadBytes(sizeof(std::uint32_t)));
}

std::uint16_t InputByteStream::ReadUInt16() {
  return bufbe16toh(ReadBytes(sizeof(std::uint16_t)));
}

std::uint8_t InputByteStream::ReadUInt8() {
  return *ReadBytes(sizeof(std::uint8_t));
}

/// Output

OutputByteStream::OutputByteStream(
  std::uint8_t* data,
  std::size_t len)
  : m_Data(data),
    m_Length(len),
    m_Counter(0),
    m_Size(len) {}

void OutputByteStream::ProduceData(std::size_t amount) {
  if (amount > m_Length)
    throw std::length_error("OutputByteStream: too many bytes to produce.");
  m_Data += amount;
  m_Length -= amount;
  m_Counter += amount;
}

void OutputByteStream::WriteData(const std::uint8_t* data, std::size_t len) {
  if (!len)
    {
      LOG(debug) << "OutputByteStream: skip empty data";
      return;
    }
  if (!data)
    throw std::runtime_error("OutputByteStream: null data");
  std::uint8_t* ptr = m_Data; 
  ProduceData(len);
  std::memcpy(ptr, data, len);
}

void OutputByteStream::WriteUInt8(std::uint8_t data) {
  WriteData(&data, sizeof(std::uint8_t));
}

void OutputByteStream::WriteUInt16(std::uint16_t data) {
  std::uint8_t buf[sizeof(std::uint16_t)] = {};
  htobe16buf(buf, data);
  WriteData(buf, sizeof(buf));
}

void OutputByteStream::WriteUInt32(std::uint32_t data) {
  std::uint8_t buf[sizeof(std::uint32_t)] = {};
  htobe32buf(buf, data);
  WriteData(buf, sizeof(buf));
}

void OutputByteStream::WriteUInt64(std::uint64_t data) {
  std::uint8_t buf[sizeof(std::uint64_t)] = {};
  htobe64buf(buf, data);
  WriteData(buf, sizeof(buf));
}

// TODO(unassigned): see comments in #510

std::uint8_t* OutputByteStream::GetPosition() const {
  return m_Data;
}

std::uint8_t* OutputByteStream::GetData() const {
  return m_Data - m_Counter;
}

std::size_t OutputByteStream::GetSize() const {
  return m_Size;
}

// Hex

const std::string GetFormattedHex(const std::uint8_t* data, std::size_t size)
{
  std::ostringstream hex;
  hex << "\n\t" << " |  ";
  std::size_t count{}, sub_count{};
  for (std::size_t i = 0; i < size; i++)
    {
      hex << std::hex << std::setfill('0') << std::setw(2)
          << static_cast<std::uint16_t>(data[i]) << " ";
      count++;
      if (count == 32 || (i == size - 1))
        {
          hex << " |" << "\n\t";
          count = 0;
        }
      sub_count++;
      if (sub_count == 8 && (i != size - 1))
        {
          hex << " |  ";
          sub_count = 0;
        }
    }
  return hex.str() + "\n";
}

std::unique_ptr<std::vector<std::uint8_t>> AddressToByteVector(
    const boost::asio::ip::address& address)
{
  bool is_v4(address.is_v4());
  auto data = std::make_unique<std::vector<std::uint8_t>>(is_v4 ? 4 : 16);
  std::memcpy(
      data->data(),
      is_v4 ? address.to_v4().to_bytes().data()
            : address.to_v6().to_bytes().data(),
      data->size());
  return data;
}

} // namespace core
} // namespace kovri
