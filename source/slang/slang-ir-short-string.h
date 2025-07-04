// slang-ir-string-literals.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;
class TargetProgram;

IRArrayType* getShortStringArrayType(IRBuilder& builder, IRShortStringType* strLitType);

IRInst* getShortStringAsArray(IRBuilder& builder, IRStringLit* strLit);

struct ShortStringsOptions
{
    bool replaceShortStringsWithArray = false;
};

bool replaceShortStringReturnChanged(
    TargetProgram* target,
    IRModule* module,
    ShortStringsOptions options);
} // namespace Slang
