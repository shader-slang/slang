// slang-ir-validate-dyn-and-some.h
#pragma once

namespace Slang
{
    
class DiagnosticSink;
struct IRModule;

void validateDynAndSomeUsage(
    IRModule* module,
    DiagnosticSink* sink);

}// namespace Slang
