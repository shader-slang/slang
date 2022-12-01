// slang-ir-autodiff-rev.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-fwd.h"

namespace Slang
{

struct IRReverseDerivativePassOptions
{
    // Nothing for now..
};

bool processReverseDerivativeCalls(
    AutoDiffSharedContext*                  autodiffContext,
    DiagnosticSink*                         sink,
    IRReverseDerivativePassOptions const&   options = IRReverseDerivativePassOptions());


}