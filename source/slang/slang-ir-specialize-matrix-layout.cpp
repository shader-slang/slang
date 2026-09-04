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

    IRIntegerValue defaultLayout = target->getOptionSet().getMatrixLayoutMode();
    if (defaultLayout == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
        defaultLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    IRBuilder builder(module);
    auto unknownLayout =
        builder.getIntValue(matrixLayoutModeType, SLANG_MATRIX_LAYOUT_MODE_UNKNOWN);
    auto resolvedLayout = builder.getIntValue(matrixLayoutModeType, defaultLayout);

    // `Unknown` is one deduplicated constant, so every unspecified layout is a use of it: a matrix
    // type's layout operand, or a `specialize` argument headed into one. Other uses are enum values
    // the user wrote (e.g. a `switch` case label) and must keep their value. `replaceOperand`
    // re-deduplicates the user, so no duplicate matrix types are left behind.
    for (auto use = unknownLayout->firstUse; use;)
    {
        auto nextUse = use->nextUse; // `replaceOperand` unlinks `use`.
        auto user = use->getUser();
        if (as<IRMatrixType>(user) || as<IRSpecialize>(user))
            builder.replaceOperand(use, resolvedLayout);
        use = nextUse;
    }
}

} // namespace Slang
