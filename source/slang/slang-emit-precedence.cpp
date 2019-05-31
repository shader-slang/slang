// slang-emit-precedence.cpp
#include "slang-emit-precedence.h"

namespace Slang {

#define SLANG_OP_INFO_EXPAND(op, name, precedence) {name, kEPrecedence_##precedence##_Left, kEPrecedence_##precedence##_Right, },

/* static */const EmitOpInfo EmitOpInfo::s_infos[int(EmitOp::CountOf)] =
{
    SLANG_OP_INFO(SLANG_OP_INFO_EXPAND)
};

} // namespace Slang
