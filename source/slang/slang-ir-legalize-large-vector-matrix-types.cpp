#include "slang-ir-legalize-large-vector-matrix-types.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

// No target language has a vector type with more than 4 elements, or a matrix type with more than
// 4 rows or columns. GPUs have no per-thread vector or matrix hardware at those sizes either, so
// such a type is just a fixed-size collection of scalars, and every target can express it as an
// array. This pass rewrites the large vector and matrix types to arrays, and rewrites the
// instructions that treat a vector or matrix as a whole (element-wise arithmetic, construction,
// swizzles, ...) to work on the elements.
//
// The pass works like `legalizeVectorTypes`: each instruction that needs a new form gets a
// replacement, computed while the original types are still in place, and all replacements
// (including the type instructions themselves) are applied at the end. Instructions that do not
// look inside a vector or matrix value (loads, stores, calls, element access, ...) need no
// replacement; they pick up the array types when the type instructions are replaced.

static bool isLargeCount(IRIntegerValue count)
{
    return count > kMaxNativeVectorElementCount && count <= kMaxVectorElementCount;
}

static bool tryGetCount(IRInst* countInst, IRIntegerValue& outCount)
{
    if (auto lit = as<IRIntLit>(countInst))
    {
        outCount = lit->getValue();
        return true;
    }
    return false;
}

/// Return true if `type` is a vector or matrix type that this pass lowers.
static bool isLargeVectorOrMatrixType(IRInst* type)
{
    if (auto vectorType = as<IRVectorType>(type))
    {
        IRIntegerValue count;
        return tryGetCount(vectorType->getElementCount(), count) && isLargeCount(count);
    }
    if (auto matrixType = as<IRMatrixType>(type))
    {
        IRIntegerValue rowCount, columnCount;
        if (!tryGetCount(matrixType->getRowCount(), rowCount) ||
            !tryGetCount(matrixType->getColumnCount(), columnCount))
            return false;
        if (rowCount > kMaxVectorElementCount || columnCount > kMaxVectorElementCount)
            return false;
        return rowCount > kMaxNativeVectorElementCount ||
               columnCount > kMaxNativeVectorElementCount;
    }
    return false;
}

/// Return true if `type` is, or points to, or is an array of, a type this pass lowers.
static bool isOrWrapsLargeVectorOrMatrixType(IRType* type)
{
    for (;;)
    {
        if (isLargeVectorOrMatrixType(type))
            return true;
        if (auto ptrType = as<IRPtrTypeBase>(type))
        {
            type = ptrType->getValueType();
            continue;
        }
        if (auto arrayType = as<IRArrayTypeBase>(type))
        {
            type = arrayType->getElementType();
            continue;
        }
        return false;
    }
}

/// Return true if `type` contains a type this pass lowers, looking through pointers, arrays and
/// struct fields.
static bool containsLargeVectorOrMatrixType(IRType* type, HashSet<IRType*>& visited)
{
    if (!type || !visited.add(type))
        return false;
    if (isOrWrapsLargeVectorOrMatrixType(type))
        return true;
    if (auto ptrType = as<IRPtrTypeBase>(type))
        return containsLargeVectorOrMatrixType(ptrType->getValueType(), visited);
    if (auto arrayType = as<IRArrayTypeBase>(type))
        return containsLargeVectorOrMatrixType(arrayType->getElementType(), visited);
    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (containsLargeVectorOrMatrixType(field->getFieldType(), visited))
                return true;
        }
    }
    return false;
}

static bool containsLargeVectorOrMatrixType(IRType* type)
{
    HashSet<IRType*> visited;
    return containsLargeVectorOrMatrixType(type, visited);
}

/// Return true if a `spirv_asm` block, one of its instructions, or one of their operands (which
/// wrap the referenced values) has a large vector or matrix type.
static bool spirvAsmUsesLargeType(IRInst* inst)
{
    if (isLargeVectorOrMatrixType(inst->getDataType()))
        return true;
    for (UInt i = 0; i < inst->getOperandCount(); i++)
    {
        auto operand = inst->getOperand(i);
        if (!operand)
            continue;
        if (isLargeVectorOrMatrixType(operand->getDataType()) || isLargeVectorOrMatrixType(operand))
            return true;
        if (as<IRSPIRVAsmOperand>(operand))
        {
            for (UInt j = 0; j < operand->getOperandCount(); j++)
            {
                auto value = operand->getOperand(j);
                if (value && (isLargeVectorOrMatrixType(value->getDataType()) ||
                              isLargeVectorOrMatrixType(value)))
                    return true;
            }
        }
    }
    for (auto child : inst->getChildren())
    {
        if (spirvAsmUsesLargeType(child))
            return true;
    }
    return false;
}

/// Return true if `func` is implemented by target code (`__intrinsic_asm`, `spirv_asm`, or a
/// target intrinsic decoration) rather than by Slang code that this pass can rewrite.
///
/// Core-module intrinsics take an element-wise path for large sizes before their
/// `__target_switch`, as `if (N > 4) { ... }`. Once specialized, that condition is a constant, but
/// the target code in the other branch may not have been removed yet, so only blocks reachable
/// through constant conditions are considered.
static bool isTargetIntrinsicFunc(IRFunc* func)
{
    if (findAnyTargetIntrinsicDecoration(func))
        return true;
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    List<IRBlock*> workList;
    HashSet<IRBlock*> visited;
    workList.add(firstBlock);
    visited.add(firstBlock);
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto block = workList[i];
        auto terminator = block->getTerminator();
        if (as<IRGenericAsm>(terminator))
            return true;
        for (auto inst : block->getChildren())
        {
            // A `spirv_asm` block on scalars (e.g. from an inlined scalar intrinsic in the
            // element-wise path) is fine; only one that handles a large value is not.
            if (as<IRSPIRVAsm>(inst) && spirvAsmUsesLargeType(inst))
                return true;
        }

        auto addSuccessor = [&](IRBlock* successor)
        {
            if (successor && visited.add(successor))
                workList.add(successor);
        };
        if (auto ifElse = as<IRIfElse>(terminator))
        {
            if (auto condition = as<IRBoolLit>(ifElse->getCondition()))
            {
                addSuccessor(
                    condition->getValue() ? ifElse->getTrueBlock() : ifElse->getFalseBlock());
                continue;
            }
        }
        for (auto successor : block->getSuccessors())
            addSuccessor(successor);
    }
    return false;
}

static String getNameHint(IRInst* inst)
{
    if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
        return nameHint->getName();
    return "<unnamed>";
}

struct LargeVectorMatrixLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    Dictionary<IRInst*, IRInst*> replacements;

    // Instructions that were rewritten into other instructions and have no value of their own
    // (e.g. a swizzled store), removed after all replacements are applied.
    List<IRInst*> instsToRemove;

    LargeVectorMatrixLoweringContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink), workList(module), workListSet(module)
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

    // Types are hoisted to the module scope, so a scan of the global instructions finds every
    // large vector or matrix type used by non-generic code.
    bool hasAnyLargeVectorOrMatrixType()
    {
        for (auto inst : module->getGlobalInsts())
        {
            if (isLargeVectorOrMatrixType(inst))
                return true;
        }
        return false;
    }

    IRType* getRowType(IRBuilder& builder, IRMatrixType* matrixType)
    {
        return builder.getVectorType(matrixType->getElementType(), matrixType->getColumnCount());
    }

    /// The type that values of `type` have after lowering.
    IRType* getLoweredType(IRBuilder& builder, IRType* type)
    {
        if (!isLargeVectorOrMatrixType(type))
            return type;
        if (auto vectorType = as<IRVectorType>(type))
        {
            return builder.getArrayType(
                vectorType->getElementType(),
                vectorType->getElementCount());
        }
        auto matrixType = as<IRMatrixType>(type);
        SLANG_ASSERT(matrixType);
        return builder.getArrayType(
            getLoweredType(builder, getRowType(builder, matrixType)),
            matrixType->getRowCount());
    }

    /// The number of elements (for a vector) or rows (for a matrix) of `type`, and the type of
    /// each before lowering.
    IRIntegerValue getOuterCount(IRBuilder& builder, IRType* type, IRType*& outPartType)
    {
        if (auto vectorType = as<IRVectorType>(type))
        {
            outPartType = vectorType->getElementType();
            return getIntVal(vectorType->getElementCount());
        }
        if (auto matrixType = as<IRMatrixType>(type))
        {
            outPartType = getRowType(builder, matrixType);
            return getIntVal(matrixType->getRowCount());
        }
        SLANG_UNEXPECTED("expected a vector or matrix type");
    }

    IRInst* emitExtract(IRBuilder& builder, IRInst* value, IRIntegerValue index)
    {
        return builder.emitElementExtract(value, builder.getIntValue(builder.getIntType(), index));
    }

    /// Build a value of (pre-lowering) vector or matrix type `type` from its elements or rows.
    IRInst* emitMakeValue(IRBuilder& builder, IRType* type, List<IRInst*> const& parts)
    {
        if (isLargeVectorOrMatrixType(type))
            return builder.emitMakeArray(
                getLoweredType(builder, type),
                parts.getCount(),
                parts.getBuffer());
        if (as<IRVectorType>(type))
            return builder.emitMakeVector(type, parts);
        if (as<IRMatrixType>(type))
            return builder.emitMakeMatrix(type, parts.getCount(), parts.getBuffer());
        SLANG_ASSERT(parts.getCount() == 1);
        return parts[0];
    }

    /// Collect the scalar elements of `value`, whose type before lowering is `type` (a scalar or
    /// a vector).
    void collectScalarElements(
        IRBuilder& builder,
        IRInst* value,
        IRType* type,
        List<IRInst*>& outElements)
    {
        if (auto vectorType = as<IRVectorType>(type))
        {
            auto count = getIntVal(vectorType->getElementCount());
            for (IRIntegerValue i = 0; i < count; i++)
                outElements.add(emitExtract(builder, value, i));
            return;
        }
        outElements.add(value);
    }

    // A value together with its type before lowering, which says how the value is split.
    struct TypedValue
    {
        IRInst* value;
        IRType* type;
    };

    /// The part of `arg` that goes with element or row `index` of a value of type `resultType`:
    /// the element (vector) or row (matrix) of `arg` if `arg` has the same shape, or `arg` itself
    /// if it is a scalar (or, for a matrix result, a vector) that is broadcast.
    TypedValue getPart(IRBuilder& builder, TypedValue arg, IRType* resultType, IRIntegerValue index)
    {
        if (as<IRMatrixType>(resultType))
        {
            if (auto argMatrixType = as<IRMatrixType>(arg.type))
                return {emitExtract(builder, arg.value, index), getRowType(builder, argMatrixType)};
            return arg;
        }
        if (as<IRVectorType>(resultType))
        {
            if (auto argVectorType = as<IRVectorType>(arg.type))
                return {emitExtract(builder, arg.value, index), argVectorType->getElementType()};
            return arg;
        }
        return arg;
    }

    /// Emit `op` applied element-wise to `args`, giving a value of (pre-lowering) type
    /// `resultType`. Large vectors are split into elements and large matrices into rows until
    /// every operand has a native type.
    IRInst* emitElementwise(
        IRBuilder& builder,
        IROp op,
        IRType* resultType,
        List<TypedValue> const& args)
    {
        bool anyLarge = isLargeVectorOrMatrixType(resultType);
        for (auto arg : args)
            anyLarge = anyLarge || isLargeVectorOrMatrixType(arg.type);

        if (!anyLarge)
        {
            List<IRInst*> operands;
            auto resultVectorType = as<IRVectorType>(resultType);
            for (Index i = 0; i < args.getCount(); i++)
            {
                auto arg = args[i];
                // A scalar broadcast to a vector result is turned into a vector, so the emitted
                // instruction has operands of one shape. The condition of a `select` is the
                // exception: a scalar condition picks a whole operand.
                bool isSelectCondition = op == kIROp_Select && i == 0;
                if (resultVectorType && as<IRBasicType>(arg.type) && !isSelectCondition)
                {
                    operands.add(builder.emitMakeVectorFromScalar(
                        builder.getVectorType(arg.type, resultVectorType->getElementCount()),
                        arg.value));
                }
                else
                {
                    operands.add(arg.value);
                }
            }
            return builder
                .emitIntrinsicInst(resultType, op, operands.getCount(), operands.getBuffer());
        }

        IRType* partType = nullptr;
        auto count = getOuterCount(builder, resultType, partType);
        List<IRInst*> parts;
        for (IRIntegerValue i = 0; i < count; i++)
        {
            List<TypedValue> partArgs;
            for (auto arg : args)
                partArgs.add(getPart(builder, arg, resultType, i));
            parts.add(emitElementwise(builder, op, partType, partArgs));
        }
        return emitMakeValue(builder, resultType, parts);
    }

    IRInst* legalizeElementwise(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        List<TypedValue> args;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            args.add({getReplacement(operand), operand->getDataType()});
        }
        return emitElementwise(builder, inst->getOp(), inst->getDataType(), args);
    }

    IRInst* legalizeMakeVector(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        List<IRInst*> elements;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            collectScalarElements(
                builder,
                getReplacement(operand),
                operand->getDataType(),
                elements);
        }
        return emitMakeValue(builder, inst->getDataType(), elements);
    }

    IRInst* legalizeMakeVectorFromScalar(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto vectorType = as<IRVectorType>(inst->getDataType());
        auto scalar = getReplacement(inst->getOperand(0));
        List<IRInst*> elements;
        auto count = getIntVal(vectorType->getElementCount());
        for (IRIntegerValue i = 0; i < count; i++)
            elements.add(scalar);
        return emitMakeValue(builder, vectorType, elements);
    }

    IRInst* legalizeMakeMatrix(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto matrixType = as<IRMatrixType>(inst->getDataType());
        auto rowType = getRowType(builder, matrixType);
        auto rowCount = getIntVal(matrixType->getRowCount());
        auto columnCount = getIntVal(matrixType->getColumnCount());

        // The operands are either the rows, or all the elements in row-major order.
        List<IRInst*> rows;
        if (inst->getOperandCount() == UInt(rowCount) &&
            as<IRVectorType>(inst->getOperand(0)->getDataType()))
        {
            for (UInt i = 0; i < inst->getOperandCount(); i++)
                rows.add(getReplacement(inst->getOperand(i)));
        }
        else
        {
            List<IRInst*> elements;
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto operand = inst->getOperand(i);
                collectScalarElements(
                    builder,
                    getReplacement(operand),
                    operand->getDataType(),
                    elements);
            }
            SLANG_ASSERT(elements.getCount() == rowCount * columnCount);
            for (IRIntegerValue r = 0; r < rowCount; r++)
            {
                List<IRInst*> rowElements;
                for (IRIntegerValue c = 0; c < columnCount; c++)
                    rowElements.add(elements[Index(r * columnCount + c)]);
                rows.add(emitMakeValue(builder, rowType, rowElements));
            }
        }
        return emitMakeValue(builder, matrixType, rows);
    }

    IRInst* legalizeMakeMatrixFromScalar(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto matrixType = as<IRMatrixType>(inst->getDataType());
        auto rowType = getRowType(builder, matrixType);
        auto rowCount = getIntVal(matrixType->getRowCount());
        auto columnCount = getIntVal(matrixType->getColumnCount());
        auto scalar = getReplacement(inst->getOperand(0));

        List<IRInst*> rowElements;
        for (IRIntegerValue c = 0; c < columnCount; c++)
            rowElements.add(scalar);
        auto row = emitMakeValue(builder, rowType, rowElements);

        List<IRInst*> rows;
        for (IRIntegerValue r = 0; r < rowCount; r++)
            rows.add(row);
        return emitMakeValue(builder, matrixType, rows);
    }

    IRInst* legalizeSwizzle(IRSwizzle* swizzle)
    {
        IRBuilder builder(swizzle);
        builder.setInsertBefore(swizzle);

        auto base = getReplacement(swizzle->getBase());
        List<IRInst*> elements;
        for (UInt i = 0; i < swizzle->getElementCount(); i++)
        {
            auto index = getIntVal(swizzle->getElementIndex(i));
            elements.add(emitExtract(builder, base, index));
        }
        return emitMakeValue(builder, swizzle->getDataType(), elements);
    }

    IRInst* legalizeSwizzleSet(IRSwizzleSet* swizzleSet)
    {
        IRBuilder builder(swizzleSet);
        builder.setInsertBefore(swizzleSet);

        auto vectorType = as<IRVectorType>(swizzleSet->getDataType());
        auto base = getReplacement(swizzleSet->getBase());
        auto source = getReplacement(swizzleSet->getSource());
        bool sourceIsVector = as<IRVectorType>(swizzleSet->getSource()->getDataType()) != nullptr;

        List<IRInst*> elements;
        auto count = getIntVal(vectorType->getElementCount());
        for (IRIntegerValue i = 0; i < count; i++)
            elements.add(emitExtract(builder, base, i));
        for (UInt i = 0; i < swizzleSet->getElementCount(); i++)
        {
            auto index = getIntVal(swizzleSet->getElementIndex(i));
            elements[Index(index)] = sourceIsVector ? emitExtract(builder, source, i) : source;
        }
        return emitMakeValue(builder, vectorType, elements);
    }

    void legalizeSwizzledStore(IRSwizzledStore* store)
    {
        IRBuilder builder(store);
        builder.setInsertBefore(store);

        auto dest = getReplacement(store->getDest());
        auto source = getReplacement(store->getSource());
        bool sourceIsVector = as<IRVectorType>(store->getSource()->getDataType()) != nullptr;
        for (UInt i = 0; i < store->getElementCount(); i++)
        {
            auto index = getIntVal(store->getElementIndex(i));
            auto elementPtr =
                builder.emitElementAddress(dest, builder.getIntValue(builder.getIntType(), index));
            builder.emitStore(
                elementPtr,
                sourceIsVector ? emitExtract(builder, source, i) : source);
        }
        instsToRemove.add(store);
    }

    /// Element `index` of `value` (of pre-lowering vector type `type`), or zero past its end.
    IRInst* getElementOrZero(IRBuilder& builder, IRInst* value, IRType* type, IRIntegerValue index)
    {
        auto vectorType = as<IRVectorType>(type);
        if (index < getIntVal(vectorType->getElementCount()))
            return emitExtract(builder, value, index);
        return builder.emitDefaultConstruct(vectorType->getElementType());
    }

    IRInst* legalizeVectorReshape(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto operand = inst->getOperand(0);
        auto value = getReplacement(operand);
        auto resultType = as<IRVectorType>(inst->getDataType());
        List<IRInst*> elements;
        auto count = getIntVal(resultType->getElementCount());
        for (IRIntegerValue i = 0; i < count; i++)
            elements.add(getElementOrZero(builder, value, operand->getDataType(), i));
        return emitMakeValue(builder, resultType, elements);
    }

    IRInst* legalizeMatrixReshape(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto operand = inst->getOperand(0);
        auto value = getReplacement(operand);
        auto operandType = as<IRMatrixType>(operand->getDataType());
        auto operandRowType = getRowType(builder, operandType);
        auto operandRowCount = getIntVal(operandType->getRowCount());

        auto resultType = as<IRMatrixType>(inst->getDataType());
        auto resultRowType = getRowType(builder, resultType);
        auto rowCount = getIntVal(resultType->getRowCount());
        auto columnCount = getIntVal(resultType->getColumnCount());

        List<IRInst*> rows;
        for (IRIntegerValue r = 0; r < rowCount; r++)
        {
            List<IRInst*> elements;
            IRInst* operandRow = r < operandRowCount ? emitExtract(builder, value, r) : nullptr;
            for (IRIntegerValue c = 0; c < columnCount; c++)
            {
                elements.add(
                    operandRow ? getElementOrZero(builder, operandRow, operandRowType, c)
                               : builder.emitDefaultConstruct(resultType->getElementType()));
            }
            rows.add(emitMakeValue(builder, resultRowType, elements));
        }
        return emitMakeValue(builder, resultType, rows);
    }

    bool usesLargeType(IRInst* inst)
    {
        if (isLargeVectorOrMatrixType(inst->getDataType()))
            return true;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            if (auto operand = inst->getOperand(i))
            {
                if (isLargeVectorOrMatrixType(operand->getDataType()))
                    return true;
            }
        }
        return false;
    }

    IRInst* legalizeInst(IRInst* inst)
    {
        if (isLargeVectorOrMatrixType(inst))
        {
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            return getLoweredType(builder, (IRType*)inst);
        }

        switch (inst->getOp())
        {
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_IRem:
        case kIROp_FRem:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
        case kIROp_Not:
        case kIROp_BitNot:
        case kIROp_Neg:
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
        case kIROp_Select:
            if (usesLargeType(inst))
                return legalizeElementwise(inst);
            break;
        case kIROp_MakeVector:
            if (isLargeVectorOrMatrixType(inst->getDataType()))
                return legalizeMakeVector(inst);
            break;
        case kIROp_MakeVectorFromScalar:
            if (isLargeVectorOrMatrixType(inst->getDataType()))
                return legalizeMakeVectorFromScalar(inst);
            break;
        case kIROp_MakeMatrix:
            if (isLargeVectorOrMatrixType(inst->getDataType()))
                return legalizeMakeMatrix(inst);
            break;
        case kIROp_MakeMatrixFromScalar:
            if (isLargeVectorOrMatrixType(inst->getDataType()))
                return legalizeMakeMatrixFromScalar(inst);
            break;
        case kIROp_Swizzle:
            if (usesLargeType(inst))
                return legalizeSwizzle(as<IRSwizzle>(inst));
            break;
        case kIROp_SwizzleSet:
            if (usesLargeType(inst))
                return legalizeSwizzleSet(as<IRSwizzleSet>(inst));
            break;
        case kIROp_SwizzledStore:
            {
                auto store = as<IRSwizzledStore>(inst);
                auto ptrType = as<IRPtrTypeBase>(store->getDest()->getDataType());
                if (ptrType && isLargeVectorOrMatrixType(ptrType->getValueType()))
                    legalizeSwizzledStore(store);
                break;
            }
        case kIROp_VectorReshape:
            if (usesLargeType(inst))
                return legalizeVectorReshape(inst);
            break;
        case kIROp_MatrixReshape:
            if (usesLargeType(inst))
                return legalizeMatrixReshape(inst);
            break;
        case kIROp_Call:
            {
                auto call = as<IRCall>(inst);
                auto callee = as<IRFunc>(call->getCallee());
                if (callee && isTargetIntrinsicFunc(callee))
                {
                    bool usesLarge = isOrWrapsLargeVectorOrMatrixType(callee->getResultType());
                    for (UInt i = 0; i < callee->getParamCount(); i++)
                        usesLarge =
                            usesLarge || isOrWrapsLargeVectorOrMatrixType(callee->getParamType(i));
                    if (usesLarge)
                    {
                        sink->diagnose(Diagnostics::LargeVectorOrMatrixInTargetIntrinsic{
                            .name = getNameHint(callee),
                            .location = call->sourceLoc});
                    }
                }
                break;
            }
        default:
            break;
        }
        return inst;
    }

    IRInst* getReplacement(IRInst* inst)
    {
        if (auto replacement = replacements.tryGetValue(inst))
            return *replacement;

        auto newInst = legalizeInst(inst);
        replacements[inst] = newInst;
        return newInst;
    }

    void diagnoseVaryings()
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func || !func->findDecoration<IREntryPointDecoration>())
                continue;
            for (auto param : func->getParams())
            {
                auto layout = findVarLayout(param);
                if (!layout)
                    continue;
                if (!layout->usesResourceKind(LayoutResourceKind::VaryingInput) &&
                    !layout->usesResourceKind(LayoutResourceKind::VaryingOutput))
                    continue;
                if (containsLargeVectorOrMatrixType(param->getDataType()))
                {
                    sink->diagnose(Diagnostics::LargeVectorOrMatrixVarying{
                        .what = String("parameter '") + getNameHint(param) + "'",
                        .location = param->sourceLoc});
                }
            }
            auto layoutDecoration = func->findDecoration<IRLayoutDecoration>();
            auto entryPointLayout =
                layoutDecoration ? as<IREntryPointLayout>(layoutDecoration->getLayout()) : nullptr;
            if (entryPointLayout)
            {
                auto resultLayout = entryPointLayout->getResultLayout();
                if (resultLayout &&
                    (resultLayout->usesResourceKind(LayoutResourceKind::VaryingOutput) ||
                     resultLayout->usesResourceKind(LayoutResourceKind::VaryingInput)) &&
                    containsLargeVectorOrMatrixType(func->getResultType()))
                {
                    sink->diagnose(Diagnostics::LargeVectorOrMatrixVarying{
                        .what = String("the result of '") + getNameHint(func) + "'",
                        .location = func->sourceLoc});
                }
            }
        }
    }

    void processModule()
    {
        diagnoseVaryings();

        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            getReplacement(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Splitting a matrix into rows can create a large vector type (the row type) that the
        // module did not use before, so lower any large type that is not lowered yet.
        List<IRInst*> newLargeTypes;
        for (auto inst : module->getGlobalInsts())
        {
            if (isLargeVectorOrMatrixType(inst) && !replacements.containsKey(inst))
                newLargeTypes.add(inst);
        }
        for (auto type : newLargeTypes)
            getReplacement(type);

        for (const auto& [old, replacement] : replacements)
        {
            if (old != replacement)
            {
                old->replaceUsesWith(replacement);
                old->removeAndDeallocate();
            }
        }

        for (auto inst : instsToRemove)
            inst->removeAndDeallocate();
    }
};

void legalizeLargeVectorMatrixTypes(IRModule* module, DiagnosticSink* sink)
{
    LargeVectorMatrixLoweringContext context(module, sink);
    if (!context.hasAnyLargeVectorOrMatrixType())
        return;
    context.processModule();
}

} // namespace Slang
