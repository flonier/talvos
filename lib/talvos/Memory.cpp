// Copyright (c) 2018 the Talvos developers. All rights reserved.
//
// This file is distributed under a three-clause BSD license. For full license
// terms please see the LICENSE file distributed with this source code.

/// \file Memory.cpp
/// This file defines the Memory class.

#include <cassert>
#include <iomanip>
#include <iostream>

#include "talvos/Memory.h"

// TODO: Allow different number of buffer bits depending on address space

/// Number of bits used for the buffer ID.
#define BUFFER_BITS (16)

/// Number of bits used for the address offset.
#define OFFSET_BITS (64 - BUFFER_BITS)

namespace talvos
{

Memory::Memory()
{
  // Skip the first buffer identifier (0).
  Buffers.resize(1);
}

Memory::~Memory()
{
  // Release all allocations.
  for (size_t Id = 1; Id < Buffers.size(); Id++)
    delete[] Buffers[Id].Data;
}

uint64_t Memory::allocate(uint64_t NumBytes)
{
  // Allocate buffer.
  Buffer B;
  B.NumBytes = NumBytes;
  B.Data = new uint8_t[NumBytes];

  // Get the next available buffer identifier.
  uint64_t Id;
  if (FreeBuffers.size())
  {
    // Re-use previously released buffer identifier.
    Id = FreeBuffers.back();
    FreeBuffers.pop_back();
    Buffers[Id] = B;
  }
  else
  {
    // Allocate new buffer identifier.
    Id = Buffers.size();
    Buffers.push_back(B);
  }

  return (Id << OFFSET_BITS);
}

void Memory::dump() const
{
  for (size_t Id = 1; Id < Buffers.size(); Id++)
  {
    if (Buffers[Id].Data)
      dump(Id << OFFSET_BITS);
  }
}

void Memory::dump(uint64_t Address) const
{
  uint64_t Id = (Address >> OFFSET_BITS);

  if (!Buffers[Id].Data)
  {
    std::cerr << "Memory::dump() invalid address: " << Address << std::endl;
    return;
  }

  for (uint64_t i = 0; i < Buffers[Id].NumBytes; i++)
  {
    if (i % 4 == 0)
    {
      std::cout << std::endl
                << std::hex << std::uppercase << std::setw(16)
                << std::setfill(' ') << std::right
                << ((((uint64_t)Id) << OFFSET_BITS) | i) << ":";
    }
    std::cout << " " << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << (int)Buffers[Id].Data[i];
  }
  std::cout << std::endl;
}

void Memory::load(uint8_t *Data, uint64_t Address, uint64_t NumBytes) const
{
  uint64_t Id = (Address >> OFFSET_BITS);
  uint64_t Offset = (Address & (((uint64_t)-1) >> BUFFER_BITS));

  // TODO: Generate useful error message for invalid memory accesses
  assert(Id < Buffers.size());
  assert(Buffers[Id].Data);
  assert((Offset + NumBytes) <= Buffers[Id].NumBytes);

  memcpy(Data, Buffers[Id].Data + Offset, NumBytes);
}

void Memory::release(uint64_t Address)
{
  uint64_t Id = (Address >> OFFSET_BITS);
  assert(Buffers[Id].Data != nullptr);

  // Release memory used by buffer.
  delete[] Buffers[Id].Data;
  Buffers[Id].Data = nullptr;

  FreeBuffers.push_back(Id);
}

void Memory::store(uint64_t Address, uint64_t NumBytes, const uint8_t *Data)
{
  uint64_t Id = (Address >> OFFSET_BITS);
  uint64_t Offset = (Address & (((uint64_t)-1) >> BUFFER_BITS));

  // TODO: Generate useful error message for invalid memory accesses
  assert(Id < Buffers.size());
  assert(Buffers[Id].Data);
  assert((Offset + NumBytes) <= Buffers[Id].NumBytes);

  memcpy(Buffers[Id].Data + Offset, Data, NumBytes);
}

} // namespace talvos
