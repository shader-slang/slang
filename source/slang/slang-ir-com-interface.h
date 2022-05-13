// slang-ir-com-interface.cpp
#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;

/// Lower com interface types.
/// A use of `IRInterfaceType` with `IRComInterfaceDecoration` will be translated into a `IRComPtr` type.
/// A use of `IRThisType` with a COM interface will also be translated into a `IRComPtr` type.
void lowerComInterfaces(IRModule* module, DiagnosticSink* sink);

}
