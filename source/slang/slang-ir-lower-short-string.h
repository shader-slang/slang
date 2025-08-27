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

IRArrayType* getShortStringPackedArray32Type(
    IRBuilder& builder,
    IRShortStringType* strLitType,
    IRInst* count = nullptr);

// if charType is null, then uint is used by default
IRInst* getShortStringAsArray(
    IRBuilder& builder,
    IRStringLit* src,
    IRBasicType* charType = nullptr);

IRInst* getShortStringAsPackedArray32(IRBuilder& builder, IRStringLit* src);

struct ShortStringsOptions
{
    bool targetSupportsStringLiterals = false;
    bool targetSupports8BitsIntegrals = false;

    ShortStringsOptions(CodeGenTarget target);
};

bool lowerShortStringReturnChanged(
    TargetProgram* target,
    IRModule* module,
    ShortStringsOptions options);
} // namespace Slang
