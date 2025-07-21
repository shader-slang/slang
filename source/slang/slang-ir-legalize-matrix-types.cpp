#include "slang-ir-legalize-matrix-types.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct MatrixTypeLoweringContext
{
    TargetProgram* targetProgram;
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    Dictionary<IRInst*, IRInst*> replacements;

    MatrixTypeLoweringContext(TargetProgram* targetProgram, IRModule* module)
        : targetProgram(targetProgram), module(module), workList(module), workListSet(module)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    bool shouldLowerTarget()
    {
        auto target = targetProgram->getTargetReq()->getTarget();
        switch (target)
        {
        case CodeGenTarget::SPIRV:
        case CodeGenTarget::SPIRVAssembly:
        case CodeGenTarget::GLSL:
        case CodeGenTarget::WGSL:
        case CodeGenTarget::WGSLSPIRV:
        case CodeGenTarget::WGSLSPIRVAssembly:
            return true;
        default:
            return false;
        }
    }

    bool shouldLowerMatrixType(IRMatrixType* matrixType)
    {
        if (!shouldLowerTarget())
            return false;

        auto elementType = matrixType->getElementType();
        return as<IRBoolType>(elementType) || as<IRUIntType>(elementType) ||
               as<IRIntType>(elementType);
    }

    IRInst* getReplacement(IRInst* inst)
    {
        if (auto replacement = replacements.tryGetValue(inst))
            return *replacement;

        IRInst* newInst = inst;

        if (auto matrixType = as<IRMatrixType>(inst))
        {
            if (shouldLowerMatrixType(matrixType))
            {
                // Lower matrix<T, R, C> to T[R][C] (array of R vectors of length C)
                auto elementType = matrixType->getElementType();
                auto rowCount = matrixType->getRowCount();
                auto columnCount = matrixType->getColumnCount();

                IRBuilder builder(matrixType);
                builder.setInsertBefore(matrixType);

                // Create vector type for columns: vector<T, C>
                auto vectorType = builder.getVectorType(elementType, columnCount);

                // Create array type for rows: vector<T, C>[R]
                auto arrayType = builder.getArrayType(vectorType, rowCount);

                newInst = arrayType;
            }
        }

        replacements[inst] = newInst;
        return newInst;
    }

    void processModule()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            // Run this inst through the replacer
            getReplacement(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Apply all replacements
        for (const auto& [old, replacement] : replacements)
        {
            if (old != replacement)
            {
                old->replaceUsesWith(replacement);
                old->removeAndDeallocate();
            }
        }
    }
};

void legalizeMatrixTypes(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink)
{
    MatrixTypeLoweringContext context(targetProgram, module);
    context.sink = sink;
    context.processModule();
}

} // namespace Slang