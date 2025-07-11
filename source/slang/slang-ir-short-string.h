// slang-ir-string-literals.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;
class TargetProgram;

// if charType is null, then uint is used by default
IRArrayType* getShortStringArrayType(
    IRBuilder& builder,
    IRShortStringType* strLitType,
    IRBasicType* charType = nullptr);

// if charType is null, then uint is used by default
IRInst* getShortStringAsArray(
    IRBuilder& builder,
    IRInst* src,
    IRBasicType* charType = nullptr,
    bool supportStringLiteral = false);

struct ShortStringsOptions
{
    bool targetSupportsStringLiterals = false;
};

bool lowerShortStringReturnChanged(
    TargetProgram* target,
    IRModule* module,
    ShortStringsOptions options);
} // namespace Slang
