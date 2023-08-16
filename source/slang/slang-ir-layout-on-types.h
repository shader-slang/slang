#pragma once

namespace Slang
{
    struct IRModule;
    struct CodeGenContext;
    void placeTypeLayoutsOnTypes(IRModule* module, CodeGenContext* codeGenContext);
}
