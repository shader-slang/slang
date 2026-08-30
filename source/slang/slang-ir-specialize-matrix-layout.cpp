#include "slang-ir-specialize-matrix-layout.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

// Returns true if `inst` is the `MatrixLayoutMode.Unknown` literal, i.e. a layout the source
// left unspecified. `MatrixLayoutMode` is an enum rather than an `int` so that this stays
// recognizable as a generic argument, where an `int` would look like a row or column count.
static bool isUnknownMatrixLayout(IRInst* inst, IRType* matrixLayoutModeType)
{
    auto lit = as<IRIntLit>(inst);
    if (!lit || lit->getFullType() != matrixLayoutModeType)
        return false;
    return lit->getValue() == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;
}

// Collects the matrix types with an unspecified layout and the `specialize` insts that pass one
// as a generic argument. The `MatrixLayoutMode` type is taken from the first matrix type seen.
struct UnresolvedMatrixLayoutCollector
{
    IRType* matrixLayoutModeType = nullptr;
    List<IRMatrixType*> matrixTypes;
    List<IRSpecialize*> specializeInsts;

    void visit(IRInst* parent)
    {
        for (auto child : parent->getChildren())
        {
            if (auto matrixType = as<IRMatrixType>(child))
            {
                // Any matrix type identifies `MatrixLayoutMode` for us.
                if (!matrixLayoutModeType)
                    matrixLayoutModeType = matrixType->getLayout()->getFullType();

                if (isUnknownMatrixLayout(matrixType->getLayout(), matrixLayoutModeType))
                    matrixTypes.add(matrixType);
            }
            visit(child);
        }
    }

    void visitSpecializeInsts(IRInst* parent)
    {
        for (auto child : parent->getChildren())
        {
            if (auto specializeInst = as<IRSpecialize>(child))
            {
                for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                {
                    if (isUnknownMatrixLayout(specializeInst->getArg(i), matrixLayoutModeType))
                    {
                        specializeInsts.add(specializeInst);
                        break;
                    }
                }
            }
            visitSpecializeInsts(child);
        }
    }
};

void specializeMatrixLayout(IRModule* module, TargetProgram* target)
{
    UnresolvedMatrixLayoutCollector collector;
    collector.visit(module->getModuleInst());
    if (!collector.matrixLayoutModeType)
        return;
    collector.visitSpecializeInsts(module->getModuleInst());

    IRIntegerValue defaultLayout = target->getOptionSet().getMatrixLayoutMode();
    if (defaultLayout == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
        defaultLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    IRBuilder builder(module);
    auto resolvedLayout = builder.getIntValue(collector.matrixLayoutModeType, defaultLayout);

    for (auto matrixType : collector.matrixTypes)
    {
        builder.setInsertBefore(matrixType);
        auto replacementMatrixType = builder.getMatrixType(
            matrixType->getElementType(),
            matrixType->getRowCount(),
            matrixType->getColumnCount(),
            resolvedLayout);
        matrixType->replaceUsesWith(replacementMatrixType);
    }

    // Also resolve the layout where it is a generic argument, or specialization would substitute
    // it into `matrix<T, N, M, L>` and mint a new unspecified-layout type after this pass ran.
    for (auto specializeInst : collector.specializeInsts)
    {
        List<IRInst*> args;
        for (UInt i = 0; i < specializeInst->getArgCount(); i++)
        {
            auto arg = specializeInst->getArg(i);
            args.add(
                isUnknownMatrixLayout(arg, collector.matrixLayoutModeType) ? resolvedLayout : arg);
        }

        builder.setInsertBefore(specializeInst);
        auto replacement = builder.emitSpecializeInst(
            specializeInst->getFullType(),
            specializeInst->getBase(),
            (UInt)args.getCount(),
            args.getBuffer());
        specializeInst->replaceUsesWith(replacement);
    }
}

} // namespace Slang
