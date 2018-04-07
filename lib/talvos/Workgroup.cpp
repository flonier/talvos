// Copyright (c) 2018 the Talvos developers. All rights reserved.
//
// This file is distributed under a three-clause BSD license. For full license
// terms please see the LICENSE file distributed with this source code.

/// \file Workgroup.cpp
/// This file defines the Workgroup class.

#include "talvos/Workgroup.h"
#include "talvos/DispatchCommand.h"
#include "talvos/Invocation.h"
#include "talvos/Memory.h"
#include "talvos/Module.h"
#include "talvos/Pipeline.h"

namespace talvos
{

Workgroup::Workgroup(Device &Dev, const DispatchCommand &Dispatch, Dim3 GroupId)
{
  this->GroupId = GroupId;
  LocalMemory = new Memory(Dev, MemoryScope::Workgroup);

  // Allocate workgroup variables.
  for (auto V : Dispatch.getPipeline()->getModule()->getWorkgroupVariables())
  {
    size_t NumBytes = V.second->getElementType()->getSize();
    uint64_t Address = LocalMemory->allocate(NumBytes);
    Variables.push_back({V.first, Object(V.second, Address)});
  }

  // Create invocations for this group.
  Dim3 GroupSize = Dispatch.getPipeline()->getGroupSize();
  for (uint32_t LZ = 0; LZ < GroupSize.Z; LZ++)
    for (uint32_t LY = 0; LY < GroupSize.Y; LY++)
      for (uint32_t LX = 0; LX < GroupSize.X; LX++)
        WorkItems.push_back(std::make_unique<Invocation>(Dev, Dispatch, this,
                                                         Dim3(LX, LY, LZ)));
}

Workgroup::~Workgroup() { delete LocalMemory; }

} // namespace talvos
