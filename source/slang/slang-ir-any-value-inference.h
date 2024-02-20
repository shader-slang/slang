// slang-ir-any-value-inference.h
#pragma once

#include "../core/slang-common.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

namespace Slang
{
    void inferAnyValueSizeWhereNecessary(
        TargetProgram* targetProgram,
        IRModule* module);
}
