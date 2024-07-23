// slang-ir-augment-global-ptr-types.h
#pragma once
#include "../core/slang-basic.h"
#include "slang-ir-spirv-snippet.h"
#include "slang-ir-insts.h"

namespace Slang
{

class DiagnosticSink;

struct IRFunc;
struct IRModule;
class TargetRequest;

void augmentGlobalPointerTypes(
    IRModule*               module,
    CodeGenContext*         codeGenContext);

}
