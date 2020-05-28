// slang-ir-type-set.cpp
#include "slang-ir-type-set.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

IRTypeSet::IRTypeSet(Session* session)
{
    m_sharedBuilder.module = nullptr;
    m_sharedBuilder.session = session;

    m_builder.sharedBuilder = &m_sharedBuilder;

    m_module = m_builder.createModule();

    m_sharedBuilder.module = m_module;

    m_builder.setInsertInto(m_module->getModuleInst());
}

IRTypeSet::~IRTypeSet()
{
    _clearTypes();
}

void IRTypeSet::clear()
{
    _clearTypes();

    m_cloneMap.Clear();

    m_module = m_builder.createModule();
    m_sharedBuilder.module = m_module;
    m_builder.setInsertInto(m_module->getModuleInst());
}

void IRTypeSet::_clearTypes()
{
    List<IRType*> types;
    getTypes(types);

    for (auto type : types)
    {
        // We need to destroy references to instructions in other modules
        if (type->getModule() == m_module)
        {
            // We want to remove arguments because an argument *could* be an instruction in another module,
            // and we don't want to those modules insts to have uses, in this module which is being destroyed
            type->removeArguments();
        }
    }
}

IRInst* IRTypeSet::cloneInst(IRInst* inst)
{
    if (inst == nullptr)
    {
        return nullptr;
    }

    // See if it's already cloned
    if (IRInst*const* newInstPtr = m_cloneMap.TryGetValue(inst))
    {
        return *newInstPtr;
    }

    IRModule* module = inst->getModule();
    // All inst's must belong to a module
    SLANG_ASSERT(module);

    // If it's in this module then we don't need to clone
    if (module == m_module)
    {
        return inst;
    }

    if (isNominalOp(inst->op))
    {
        // We can clone without any definition, and add the linkage

        // TODO(JS)
        // This is arguably problematic - I'm adding an instruction from another module to the map, to be it's self.
        // I did have code which created a copy of the nominal instruction and name hint, but because nominality means
        // 'same address' other code would generate a different name for that instruction (say as compared to being a member in
        // the original instruction)
        //
        // Because I use findOrAddInst which doesn't hoist instructions, the hoisting doesn't rely on parenting, that would
        // break.

        // If nominal, we just use the original inst
        m_cloneMap.Add(inst, inst);
        return inst;
    }

    // It would be nice if I could use ir-clone.cpp to do this -> but it doesn't clone
    // operands. We wouldn't want to clone decorations, and it can't clone IRConstant(!) so
    // it's no use

    IRInst* clone = nullptr;
    switch (inst->op)
    {
        case kIROp_IntLit:
        {
            auto intLit = static_cast<IRConstant*>(inst);
            IRType* clonedType = cloneType(intLit->getDataType());
            clone = m_builder.getIntValue(clonedType, intLit->value.intVal);
            break;
        }
        case kIROp_StringLit:
        {
            auto stringLit = static_cast<IRStringLit*>(inst);
            clone = m_builder.getStringValue(stringLit->getStringSlice());
            break;
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(inst);
            const Index elementCount = Index(getIntVal(vecType->getElementCount()));

            if (elementCount <= 1)
            {
                clone = cloneType(vecType->getElementType());
            }
            break;
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(inst);
            const Index columnCount = Index(getIntVal(matType->getColumnCount()));
            const Index rowCount = Index(getIntVal(matType->getRowCount()));

            if (columnCount <= 1 && rowCount <= 1)
            {
                clone = cloneType(matType->getElementType());
            }
            break;
        }
        default: break;
    }

    if (!clone)
    {
        if (IRBasicType::isaImpl(inst->op))
        {
            clone = m_builder.getType(inst->op);
        }
        else
        {
            IRType* irType = dynamicCast<IRType>(inst);
            if (irType)
            {
                auto clonedType = cloneType(inst->getFullType());
                Index operandCount = Index(inst->getOperandCount());

                List<IRInst*> cloneOperands;
                cloneOperands.setCount(operandCount);

                for (Index i = 0; i < operandCount; ++i)
                {
                    cloneOperands[i] = cloneInst(inst->getOperand(i));
                }

                //clone = m_irBuilder.findOrEmitHoistableInst(cloneType, inst->op, operandCount, cloneOperands.getBuffer());

                UInt operandCounts[1] = { UInt(operandCount) };
                IRInst*const* listOperands[1] = { cloneOperands.getBuffer() };

                clone = m_builder.findOrAddInst(clonedType, inst->op, 1, operandCounts, listOperands);
            }
            else
            {
                // This cloning style only works on insts that are not unique
                auto clonedType = cloneType(inst->getFullType());

                Index operandCount = Index(inst->getOperandCount());
                clone = m_builder.emitIntrinsicInst(clonedType, inst->op, operandCount, nullptr);
                for (Index i = 0; i < operandCount; ++i)
                {
                    auto cloneOperand = cloneInst(inst->getOperand(i));
                    clone->getOperands()[i].init(clone, cloneOperand);
                }
            }
        }
    }

    m_cloneMap.Add(inst, clone);
    return clone;
}

IRType* IRTypeSet::add(IRType* irType)
{
    if (irType->getModule() == m_module)
    {
        return irType;
    }
    // We need to clone the type
    return cloneType(irType);
}

void IRTypeSet::getTypes(List<IRType*>& outTypes) const
{
    outTypes.clear();
    for (auto inst : m_module->getModuleInst()->getChildren())
    {
        if (IRType* type = as<IRType>(inst))
        {
            outTypes.add(type);
        }
    }
}

void IRTypeSet::getTypes(Kind kind, List<IRType*>& outTypes) const
{
    outTypes.clear();

    for (auto inst : m_module->getModuleInst()->getChildren())
    {
        IRType* type = nullptr;

        switch (kind)
        {
            case Kind::Scalar:
            {
                type = as<IRBasicType>(inst);
                break;
            }
            case Kind::Vector:
            {
                type = as<IRVectorType>(inst);
                break;
            }
            case Kind::Matrix:
            {
                type = as<IRMatrixType>(inst);
                break;
            }
            default: break;
        }

        if (type)
        {
            outTypes.add(type);
        }
    }
}

IRType* IRTypeSet::addVectorType(IRType* inElementType, int colsCount)
{
    IRType* elementType = cloneType(inElementType);
    if (colsCount == 1)
    {
        return elementType;
    }
    return m_builder.getVectorType(elementType, m_builder.getIntValue(m_builder.getIntType(), colsCount));
}

void IRTypeSet::addVectorForMatrixTypes()
{
    // Make a copy so we can alter m_types dictionary
    List<IRType*> types;
    getTypes(Kind::Matrix, types);
    for (IRType* type : types)
    {
        SLANG_ASSERT(as<IRMatrixType>(type));
        IRMatrixType* matType = static_cast<IRMatrixType*>(type);
        m_builder.getVectorType(matType->getElementType(), matType->getColumnCount());
    }
}

static bool _hasNominalOperand(IRInst* inst)
{
    const Index operandCount = Index(inst->getOperandCount());
    auto operands = inst->getOperands();

    for (Index i = 0; i < operandCount; ++i)
    {
        IRInst* operand = operands[i].get();
        if (isNominalOp(operand->op))
        {
            return true;
        }
    }

    return false;
}

void IRTypeSet::_addAllBuiltinTypesRec(IRInst* inst)
{
    for (IRInst* child = inst->getFirstDecorationOrChild(); child; child = child->getNextInst())
    {
        IRType* type = nullptr;

        if (auto vectorType = as<IRVectorType>(child))
        {
            type = vectorType;
        }
        else if (auto matrixType = as<IRMatrixType>(child))
        {
            type = matrixType;
        }
        if (type && !_hasNominalOperand(type))
        {
            add(type);
        }
        else
        {
            _addAllBuiltinTypesRec(child);
        }
    }
}

void IRTypeSet::addAllBuiltinTypes(IRModule* module)
{
    _addAllBuiltinTypesRec(module->getModuleInst());
}

}
