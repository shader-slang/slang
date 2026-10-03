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

/// Writes the `-depfile` output, if one was requested. Returns failure only when the file cannot
/// be opened; errors while writing to it are not reported.
SlangResult writeDependencyFile(EndToEndCompileRequest* compileRequest);


} // namespace Slang
