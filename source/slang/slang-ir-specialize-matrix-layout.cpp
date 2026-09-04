#include "slang-ir-specialize-matrix-layout.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

// Returns the `MatrixLayoutMode` enum type, or null if `parent` contains no matrix type.
// Every matrix type carries this type on its layout operand, so any of them reveals it.
// Recurses because matrix types inside generics are not hoisted to global scope.
static IRType* findMatrixLayoutModeType(IRInst* parent)
{
    for (auto child : parent->getChildren())
    {
        if (auto matrixType = as<IRMatrixType>(child))
            return matrixType->getLayout()->getFullType();
        if (auto layoutModeType = findMatrixLayoutModeType(child))
            return layoutModeType;
    }
    return nullptr;
}

void specializeMatrixLayout(IRModule* module, TargetProgram* target)
{
    auto matrixLayoutModeType = findMatrixLayoutModeType(module->getModuleInst());
    if (!matrixLayoutModeType)
        return;

    IRBuilder builder(module);

    // `MatrixLayoutMode.Unknown` is a single deduplicated constant, so every unspecified layout
    // in the module, including one passed as a generic argument, is a use of this instruction.
    auto unknownLayout =
        builder.getIntValue(matrixLayoutModeType, SLANG_MATRIX_LAYOUT_MODE_UNKNOWN);
    if (!unknownLayout->hasUses())
        return;

    IRIntegerValue defaultLayout = target->getOptionSet().getMatrixLayoutMode();
    if (defaultLayout == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
        defaultLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    // Users are hoistable, so `replaceUsesWith` re-deduplicates them: a matrix type with the
    // resolved layout merges with an existing identical one instead of becoming a duplicate.
    unknownLayout->replaceUsesWith(builder.getIntValue(matrixLayoutModeType, defaultLayout));
}

} // namespace Slang
