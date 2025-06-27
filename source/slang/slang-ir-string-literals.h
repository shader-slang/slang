// slang-ir-string-literals.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;
class TargetProgram;

IRArrayType* getStringLiteralArrayType(IRBuilder& builder, IRStringLiteralType* strLitType);

IRInst* getStringLiteralAsArray(IRBuilder& builder, IRStringLit* strLit);

struct StringLiteralsOptions
{
    bool replaceStringLiteralsWithArray = false;
};

bool replaceStringLiteralsReturnChanged(
    TargetProgram* target,
    IRModule* module,
    StringLiteralsOptions options);
} // namespace Slang
