// Copyright (c) 2018 the Talvos developers. All rights reserved.
//
// This file is distributed under a three-clause BSD license. For full license
// terms please see the LICENSE file distributed with this source code.

/// \file Pipeline.cpp
/// This file defines the Pipeline class.

#include "talvos/Pipeline.h"
#include "talvos/Function.h"
#include "talvos/Instruction.h"
#include "talvos/Invocation.h"
#include "talvos/Module.h"

#include <iostream>

namespace talvos
{

Pipeline::Pipeline(Device &D, const Module *M, const Function *F,
                   const SpecConstantMap &SM)
{
  Mod = M;
  Func = F;

  Objects = M->getObjects();

  // Update objects with specialization constant values.
  for (auto SE : SM)
    Objects[M->getSpecConstant(SE.first)] = SE.second;

  // Evaluate OpSpecConstantOp instructions.
  // Create a dummy invocation to dispatch the instructions to.
  Invocation I(D, Objects);
  for (const Instruction *Op : M->getSpecConstantOps())
  {
    // Evaluate instruction.
    I.execute(Op);

    // Copy result into object map.
    uint32_t Id = Op->getOperand(1);
    Objects[Id] = I.getObject(Id);
  }

  // Get local size from execution mode, but override with WorkgroupSize
  // decoration if present.
  this->GroupSize = M->getLocalSize(F->getId());
  if (uint32_t WorkgroupSizeId = M->getWorkgroupSizeId())
  {
    const Object &WorkgroupSize = Objects[WorkgroupSizeId];
    this->GroupSize.X = WorkgroupSize.get<uint32_t>(0);
    this->GroupSize.Y = WorkgroupSize.get<uint32_t>(1);
    this->GroupSize.Z = WorkgroupSize.get<uint32_t>(2);
  }
}

} // namespace talvos
