// slang-emit-dependency-file.h
#pragma once

//
// This file defines the interface for emitting a
// dependency file (in the same format used by `make`,
// `gcc`, and various other tools) based on a compile
// request using the `slangc` tool.
//

#include <slang.h>

namespace Slang
{
class EndToEndCompileRequest;

SlangResult writeDependencyFile(EndToEndCompileRequest* compileRequest);


} // namespace Slang
