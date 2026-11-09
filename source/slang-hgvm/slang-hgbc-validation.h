// slang-hgbc-validation.h
#pragma once

#include "core/slang-basic.h"

namespace Slang::HGVM
{

class Program;

/// Validates the static invariants required to execute every function in an HGBC program.
SlangResult validateProgram(const Program& program, String& outError);

} // namespace Slang::HGVM
