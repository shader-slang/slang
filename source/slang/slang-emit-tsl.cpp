// slang-emit-tsl.cpp

#include "slang-emit-tsl.h"

#include "slang-ir-util.h"
#include "slang-legalize-types.h"

namespace Slang
{

TSLSourceEmitter::TSLSourceEmitter(const Desc& desc)
    : CLikeSourceEmitter(desc)
{
}

const char* TSLSourceEmitter::getTSLBinaryOpName(IROp op)
{
    switch (op)
    {
    case kIROp_Add: return "add";
    case kIROp_Sub: return "sub";
    case kIROp_Mul: return "mul";
    case kIROp_Div: return "div";
    case kIROp_IRem:
    case kIROp_FRem: return "remainder";
    case kIROp_Eql: return "equal";
    case kIROp_Neq: return "notEqual";
    case kIROp_Less: return "lessThan";
    case kIROp_Greater: return "greaterThan";
    case kIROp_Leq: return "lessThanEqual";
    case kIROp_Geq: return "greaterThanEqual";
    case kIROp_And: return "and";
    case kIROp_Or: return "or";
    case kIROp_BitXor: return "bitXor";
    case kIROp_BitAnd: return "bitAnd";
    case kIROp_BitOr: return "bitOr";
    case kIROp_Lsh: return "shiftLeft";
    case kIROp_Rsh: return "shiftRight";
    default: return nullptr;
    }
}

const char* TSLSourceEmitter::getTSLUnaryOpName(IROp op)
{
    switch (op)
    {
    case kIROp_Neg: return "negate";
    case kIROp_Not: return "not";
    case kIROp_BitNot: return "bitNot";
    default: return nullptr;
    }
}

void TSLSourceEmitter::addTSLImport(const char* name)
{
    m_tslImports.add(name);
}

String TSLSourceEmitter::getTSLTypeName(IRType* type)
{
    if (!type)
        return "void";

    switch (type->getOp())
    {
    case kIROp_VoidType:
        return "void";

    case kIROp_FloatType:
        return "float";

    case kIROp_IntType:
        return "int";

    case kIROp_UIntType:
        return "uint";

    case kIROp_BoolType:
        return "bool";

    case kIROp_HalfType:
        return "float"; // TSL doesn't have half, use float

    case kIROp_DoubleType:
        return "float"; // TSL doesn't have double, use float

    case kIROp_VectorType:
        {
            auto vecType = as<IRVectorType>(type);
            auto elemType = vecType->getElementType();
            auto elemCount = as<IRIntLit>(vecType->getElementCount());
            int count = elemCount ? int(elemCount->getValue()) : 4;

            StringBuilder sb;
            if (as<IRFloatType>(elemType) || as<IRHalfType>(elemType) || as<IRDoubleType>(elemType))
            {
                sb << "vec" << count;
            }
            else if (as<IRIntType>(elemType))
            {
                sb << "ivec" << count;
            }
            else if (as<IRUIntType>(elemType))
            {
                sb << "uvec" << count;
            }
            else if (as<IRBoolType>(elemType))
            {
                sb << "bvec" << count;
            }
            else
            {
                sb << "vec" << count;
            }
            return sb.produceString();
        }

    case kIROp_MatrixType:
        {
            auto matType = as<IRMatrixType>(type);
            auto rowCount = as<IRIntLit>(matType->getRowCount());
            auto colCount = as<IRIntLit>(matType->getColumnCount());
            int rows = rowCount ? int(rowCount->getValue()) : 4;
            int cols = colCount ? int(colCount->getValue()) : 4;

            StringBuilder sb;
            if (rows == cols)
            {
                sb << "mat" << rows;
            }
            else
            {
                sb << "mat" << cols << "x" << rows;
            }
            return sb.produceString();
        }

    case kIROp_StructType:
        return getName(type);

    case kIROp_PtrType:
    case kIROp_OutParamType:
    case kIROp_BorrowInOutParamType:
    case kIROp_RefParamType:
        {
            auto ptrType = as<IRPtrTypeBase>(type);
            return getTSLTypeName(ptrType->getValueType());
        }

    // Storage buffer types - TSL uses storage() function to access these
    case kIROp_HLSLRWStructuredBufferType:
    case kIROp_HLSLRasterizerOrderedStructuredBufferType:
        {
            // For RWStructuredBuffer<T>, we emit a storage buffer type
            // In TSL, these are accessed via storage(buffer, type, count)
            auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type);
            auto elementType = structuredBufferType->getElementType();
            StringBuilder sb;
            sb << "StorageBuffer<" << getTSLTypeName(elementType) << ">";
            return sb.produceString();
        }

    case kIROp_HLSLStructuredBufferType:
        {
            // For read-only StructuredBuffer<T>
            auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type);
            auto elementType = structuredBufferType->getElementType();
            StringBuilder sb;
            sb << "StorageBufferReadOnly<" << getTSLTypeName(elementType) << ">";
            return sb.produceString();
        }

    case kIROp_ConstantBufferType:
        {
            // For constant buffers, we just emit the element type
            // TSL will use uniform() for each field
            auto cbType = as<IRConstantBufferType>(type);
            return getTSLTypeName((IRType*)cbType->getElementType());
        }

    case kIROp_ParameterBlockType:
        {
            auto pbType = as<IRParameterBlockType>(type);
            return getTSLTypeName((IRType*)pbType->getElementType());
        }

    default:
        return "unknown";
    }
}

void TSLSourceEmitter::emitFrontMatterImpl(TargetRequest* targetReq)
{
    SLANG_UNUSED(targetReq);

    m_writer->emit("// Generated by Slang - TSL (Three.js Shading Language) Target\n");
    m_writer->emit("// For use with Three.js WebGPU/WebGL renderer\n");
    m_writer->emit("\n");

    // Emit imports
    m_writer->emit("import {\n");
    m_writer->indent();

    // Common imports that are almost always needed
    m_writer->emit("Fn,\n");
    m_writer->emit("float, int, uint, bool,\n");
    m_writer->emit("vec2, vec3, vec4,\n");
    m_writer->emit("ivec2, ivec3, ivec4,\n");
    m_writer->emit("uvec2, uvec3, uvec4,\n");
    m_writer->emit("mat2, mat3, mat4,\n");
    m_writer->emit("add, sub, mul, div,\n");
    m_writer->emit("equal, notEqual, lessThan, greaterThan, lessThanEqual, greaterThanEqual,\n");
    m_writer->emit("and, or, not,\n");
    m_writer->emit("If, Loop, Switch, Break, Continue, Discard, Return,\n");
    m_writer->emit("// Math functions\n");
    m_writer->emit("abs, acos, asin, atan, ceil, clamp, cos, cross, degrees,\n");
    m_writer->emit("distance, dot, exp, exp2, floor, fract, inverseSqrt, length,\n");
    m_writer->emit("log, log2, max, min, mix, mod, normalize, pow, radians,\n");
    m_writer->emit("reflect, refract, round, sign, sin, smoothstep, sqrt, step, tan, trunc,\n");
    m_writer->emit("// Compute shader builtins\n");
    m_writer->emit("instanceIndex, localIndex, workgroupId,\n");
    m_writer->emit("// Uniform and storage\n");
    m_writer->emit("uniform, storage, instancedArray,\n");
    m_writer->emit("// Atomic operations\n");
    m_writer->emit("atomicStore, atomicLoad, atomicAdd, atomicSub, atomicMax, atomicMin, atomicAnd, atomicOr, atomicXor\n");

    m_writer->dedent();
    m_writer->emit("} from 'three/tsl';\n\n");
}

void TSLSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    String typeName = getTSLTypeName(type);
    m_writer->emit(typeName);
}

void TSLSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    StringBuilder sb;

    if (as<IRFloatType>(elementType) || as<IRHalfType>(elementType) || as<IRDoubleType>(elementType))
    {
        sb << "vec" << elementCount;
    }
    else if (as<IRIntType>(elementType))
    {
        sb << "ivec" << elementCount;
    }
    else if (as<IRUIntType>(elementType))
    {
        sb << "uvec" << elementCount;
    }
    else if (as<IRBoolType>(elementType))
    {
        sb << "bvec" << elementCount;
    }
    else
    {
        sb << "vec" << elementCount;
    }

    m_writer->emit(sb.getUnownedSlice());
}

void TSLSourceEmitter::emitSimpleValueImpl(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_IntLit:
        {
            auto litInst = as<IRIntLit>(inst);
            auto type = litInst->getDataType();
            IRIntegerValue value = litInst->getValue();

            if (as<IRIntType>(type))
            {
                m_writer->emit("int(");
                m_writer->emit(value);
                m_writer->emit(")");
            }
            else if (as<IRUIntType>(type))
            {
                m_writer->emit("uint(");
                m_writer->emit(UInt(value));
                m_writer->emit(")");
            }
            else
            {
                // Default to int for TSL
                m_writer->emit("int(");
                m_writer->emit(value);
                m_writer->emit(")");
            }
        }
        break;

    case kIROp_FloatLit:
        {
            auto litInst = as<IRFloatLit>(inst);
            IRFloatingPointValue value = litInst->getValue();

            m_writer->emit("float(");

            // Format the float value
            StringBuilder sb;
            sb << value;
            String valStr = sb.produceString();

            // Ensure decimal point for float literals
            if (valStr.indexOf('.') == Index(-1) && valStr.indexOf('e') == Index(-1) &&
                valStr.indexOf('E') == Index(-1))
            {
                valStr = valStr + ".0";
            }

            m_writer->emit(valStr);
            m_writer->emit(")");
        }
        break;

    case kIROp_BoolLit:
        {
            auto litInst = as<IRBoolLit>(inst);
            m_writer->emit("bool(");
            m_writer->emit(litInst->getValue() ? "true" : "false");
            m_writer->emit(")");
        }
        break;

    default:
        // Fall back to base implementation
        CLikeSourceEmitter::emitSimpleValueImpl(inst);
        break;
    }
}

void TSLSourceEmitter::emitTSLBinaryOp(
    IROp op,
    IRInst* left,
    IRInst* right,
    const EmitOpInfo& outerPrec)
{
    SLANG_UNUSED(outerPrec);

    const char* opName = getTSLBinaryOpName(op);
    if (opName)
    {
        m_writer->emit(opName);
        m_writer->emit("(");
        emitOperand(left, getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(right, getInfo(EmitOp::General));
        m_writer->emit(")");
    }
    else
    {
        // Fallback: emit the operands with method chaining
        emitOperand(left, getInfo(EmitOp::General));
        m_writer->emit(".op(");
        emitOperand(right, getInfo(EmitOp::General));
        m_writer->emit(")");
    }
}

void TSLSourceEmitter::emitTSLUnaryOp(IROp op, IRInst* operand, const EmitOpInfo& outerPrec)
{
    SLANG_UNUSED(outerPrec);

    const char* opName = getTSLUnaryOpName(op);
    if (opName)
    {
        m_writer->emit(opName);
        m_writer->emit("(");
        emitOperand(operand, getInfo(EmitOp::General));
        m_writer->emit(")");
    }
    else
    {
        emitOperand(operand, getInfo(EmitOp::General));
    }
}

void TSLSourceEmitter::emitTSLCall(IRCall* call, const EmitOpInfo& outerPrec)
{
    SLANG_UNUSED(outerPrec);

    auto callee = call->getCallee();

    // Emit the function name
    emitOperand(callee, getInfo(EmitOp::General));

    m_writer->emit("(");

    bool first = true;
    UInt argCount = call->getArgCount();
    for (UInt i = 0; i < argCount; i++)
    {
        if (!first)
            m_writer->emit(", ");
        first = false;

        emitOperand(call->getArg(i), getInfo(EmitOp::General));
    }

    m_writer->emit(")");
}

void TSLSourceEmitter::emitTSLConstructor(IRInst* inst, const EmitOpInfo& outerPrec)
{
    SLANG_UNUSED(outerPrec);

    String typeName = getTSLTypeName(inst->getDataType());
    m_writer->emit(typeName);
    m_writer->emit("(");

    bool first = true;
    UInt operandCount = inst->getOperandCount();
    for (UInt i = 0; i < operandCount; i++)
    {
        if (!first)
            m_writer->emit(", ");
        first = false;

        emitOperand(inst->getOperand(i), getInfo(EmitOp::General));
    }

    m_writer->emit(")");
}

void TSLSourceEmitter::emitTSLSwizzle(IRSwizzle* swizzle, const EmitOpInfo& outerPrec)
{
    SLANG_UNUSED(outerPrec);

    emitOperand(swizzle->getBase(), getInfo(EmitOp::General));
    m_writer->emit(".");

    static const char kComponentNames[] = "xyzw";
    UInt elementCount = swizzle->getElementCount();
    for (UInt i = 0; i < elementCount; i++)
    {
        auto indexInst = swizzle->getElementIndex(i);
        if (auto intLit = as<IRIntLit>(indexInst))
        {
            int index = int(intLit->getValue());
            if (index >= 0 && index < 4)
            {
                m_writer->emitChar(kComponentNames[index]);
            }
        }
    }
}

bool TSLSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
    // Binary operations - emit as TSL function calls
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_IRem:
    case kIROp_FRem:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Less:
    case kIROp_Greater:
    case kIROp_Leq:
    case kIROp_Geq:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_BitXor:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_Lsh:
    case kIROp_Rsh:
        emitTSLBinaryOp(inst->getOp(), inst->getOperand(0), inst->getOperand(1), inOuterPrec);
        return true;

    // Unary operations
    case kIROp_Neg:
    case kIROp_Not:
    case kIROp_BitNot:
        emitTSLUnaryOp(inst->getOp(), inst->getOperand(0), inOuterPrec);
        return true;

    // Function calls
    case kIROp_Call:
        emitTSLCall(as<IRCall>(inst), inOuterPrec);
        return true;

    // Constructors
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeStruct:
        emitTSLConstructor(inst, inOuterPrec);
        return true;

    // Swizzle
    case kIROp_Swizzle:
        emitTSLSwizzle(as<IRSwizzle>(inst), inOuterPrec);
        return true;

    // For other cases, fall back to base implementation
    default:
        return CLikeSourceEmitter::tryEmitInstExprImpl(inst, inOuterPrec);
    }
}

void TSLSourceEmitter::emitTSLIfElse(IRIfElse* ifElse)
{
    m_writer->emit("If(");
    emitOperand(ifElse->getCondition(), getInfo(EmitOp::General));
    m_writer->emit(", () => {\n");
    m_writer->indent();

    // Emit true branch - we need to visit the true block
    if (auto trueBlock = ifElse->getTrueBlock())
    {
        // Emit the body of the true block
        for (auto inst : trueBlock->getOrdinaryInsts())
        {
            if (as<IRTerminatorInst>(inst))
                continue; // Skip terminators
            emitInst(inst);
        }
    }

    m_writer->dedent();
    m_writer->emit("})");

    // Check for else branch
    if (auto falseBlock = ifElse->getFalseBlock())
    {
        m_writer->emit(".Else(() => {\n");
        m_writer->indent();

        for (auto inst : falseBlock->getOrdinaryInsts())
        {
            if (as<IRTerminatorInst>(inst))
                continue;
            emitInst(inst);
        }

        m_writer->dedent();
        m_writer->emit("})");
    }

    m_writer->emit(";\n");
}

void TSLSourceEmitter::emitTSLLoop(IRLoop* loop)
{
    m_writer->emit("Loop(() => {\n");
    m_writer->indent();

    // Emit loop body
    if (auto targetBlock = loop->getTargetBlock())
    {
        for (auto inst : targetBlock->getOrdinaryInsts())
        {
            if (as<IRTerminatorInst>(inst))
                continue;
            emitInst(inst);
        }
    }

    m_writer->dedent();
    m_writer->emit("});\n");
}

void TSLSourceEmitter::emitTSLSwitch(IRSwitch* switchInst)
{
    m_writer->emit("Switch(");
    emitOperand(switchInst->getCondition(), getInfo(EmitOp::General));
    m_writer->emit(")\n");
    m_writer->indent();

    // Emit cases
    UInt caseCount = switchInst->getCaseCount();
    for (UInt i = 0; i < caseCount; i++)
    {
        auto caseVal = switchInst->getCaseValue(i);
        auto caseBlock = switchInst->getCaseLabel(i);

        m_writer->emit(".Case(");
        emitOperand(caseVal, getInfo(EmitOp::General));
        m_writer->emit(", () => {\n");
        m_writer->indent();

        for (auto inst : caseBlock->getOrdinaryInsts())
        {
            if (as<IRTerminatorInst>(inst))
                continue;
            emitInst(inst);
        }

        m_writer->dedent();
        m_writer->emit("})\n");
    }

    // Default case
    if (auto defaultBlock = switchInst->getDefaultLabel())
    {
        m_writer->emit(".Default(() => {\n");
        m_writer->indent();

        for (auto inst : defaultBlock->getOrdinaryInsts())
        {
            if (as<IRTerminatorInst>(inst))
                continue;
            emitInst(inst);
        }

        m_writer->dedent();
        m_writer->emit("})");
    }

    m_writer->dedent();
    m_writer->emit(";\n");
}

bool TSLSourceEmitter::tryEmitInstStmtImpl(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_IfElse:
        emitTSLIfElse(as<IRIfElse>(inst));
        return true;

    case kIROp_Loop:
        emitTSLLoop(as<IRLoop>(inst));
        return true;

    case kIROp_Switch:
        emitTSLSwitch(as<IRSwitch>(inst));
        return true;

    case kIROp_Discard:
        m_writer->emit("Discard();\n");
        return true;

    case kIROp_Return:
        {
            auto returnInst = as<IRReturn>(inst);
            if (auto val = returnInst->getVal())
            {
                m_writer->emit("return ");
                emitOperand(val, getInfo(EmitOp::General));
                m_writer->emit(";\n");
            }
            else
            {
                m_writer->emit("return;\n");
            }
            return true;
        }

    default:
        return CLikeSourceEmitter::tryEmitInstStmtImpl(inst);
    }
}

void TSLSourceEmitter::emitFuncHeaderImpl(IRFunc* func)
{
    SLANG_UNUSED(func->getResultType()); // TSL functions don't use return types in header

    // Check if this is an entry point
    auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();

    // Emit comment for entry points
    if (entryPointDecor)
    {
        auto stage = entryPointDecor->getProfile().getStage();
        switch (stage)
        {
        case Stage::Vertex:
            m_writer->emit("// Vertex Shader Entry Point\n");
            break;
        case Stage::Fragment:
            m_writer->emit("// Fragment Shader Entry Point\n");
            break;
        case Stage::Compute:
            m_writer->emit("// Compute Shader Entry Point\n");
            break;
        default:
            m_writer->emit("// Shader Entry Point\n");
            break;
        }
    }

    // Emit TSL function definition
    m_writer->emit("export const ");
    m_writer->emit(getName(func));
    m_writer->emit(" = /*@__PURE__*/ Fn((");

    // Emit parameters
    bool first = true;
    for (auto param : func->getParams())
    {
        if (!first)
            m_writer->emit(", ");
        first = false;

        m_writer->emit(getName(param));
    }

    m_writer->emit(") => ");
}

void TSLSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    // For TSL, we just emit the parameter name
    m_writer->emit(getName(param));
}

void TSLSourceEmitter::emitVarKeywordImpl(IRType* type, IRInst* varDecl)
{
    SLANG_UNUSED(type);
    SLANG_UNUSED(varDecl);

    // TSL uses const for variable declarations
    m_writer->emit("const ");
}

void TSLSourceEmitter::emitOperandImpl(IRInst* operand, EmitOpInfo const& outerPrec)
{
    // Handle loads specially - emit the variable name directly
    if (auto load = as<IRLoad>(operand))
    {
        emitOperand(load->getPtr(), outerPrec);
        return;
    }

    // For everything else, use base implementation
    CLikeSourceEmitter::emitOperandImpl(operand, outerPrec);
}

void TSLSourceEmitter::emitEntryPointAttributesImpl(
    IRFunc* irFunc,
    IREntryPointDecoration* entryPointDecor)
{
    SLANG_UNUSED(irFunc);
    SLANG_UNUSED(entryPointDecor);
    // TSL doesn't use entry point attributes in the same way as other targets
}

void TSLSourceEmitter::emitTSLDefaultValue(IRType* type)
{
    // Emit a default/zero value for the given type
    if (!type)
    {
        m_writer->emit("0");
        return;
    }

    switch (type->getOp())
    {
    case kIROp_FloatType:
    case kIROp_HalfType:
    case kIROp_DoubleType:
        m_writer->emit("0.0");
        break;

    case kIROp_IntType:
        m_writer->emit("0");
        break;

    case kIROp_UIntType:
        m_writer->emit("0");
        break;

    case kIROp_BoolType:
        m_writer->emit("false");
        break;

    case kIROp_VectorType:
        {
            auto vecType = as<IRVectorType>(type);
            auto elemType = vecType->getElementType();
            auto elemCount = as<IRIntLit>(vecType->getElementCount());
            int count = elemCount ? int(elemCount->getValue()) : 4;

            String typeName = getTSLTypeName(type);
            m_writer->emit(typeName);
            m_writer->emit("(");
            for (int i = 0; i < count; i++)
            {
                if (i > 0)
                    m_writer->emit(", ");
                emitTSLDefaultValue(elemType);
            }
            m_writer->emit(")");
        }
        break;

    case kIROp_MatrixType:
        {
            // For matrices, emit identity-like default
            m_writer->emit(getTSLTypeName(type));
            m_writer->emit("()");
        }
        break;

    default:
        m_writer->emit("0");
        break;
    }
}

void TSLSourceEmitter::emitParameterGroupImpl(
    IRGlobalParam* varDecl,
    IRUniformParameterGroupType* type)
{
    // TSL handles uniform parameter groups using the uniform() function
    // For constant buffers, we emit each field as a uniform
    auto elementType = type->getElementType();

    m_writer->emit("// Uniform: ");
    m_writer->emit(getName(varDecl));
    m_writer->emit("\n");

    // Check if the element type is a struct
    if (auto structType = as<IRStructType>(elementType))
    {
        // Emit struct fields as individual uniforms
        m_writer->emit("const ");
        m_writer->emit(getName(varDecl));
        m_writer->emit(" = {\n");
        m_writer->indent();

        bool first = true;
        for (auto field : structType->getFields())
        {
            if (!first)
                m_writer->emit(",\n");
            first = false;

            m_writer->emit(getName(field->getKey()));
            m_writer->emit(": uniform(");
            emitTSLDefaultValue(field->getFieldType());
            m_writer->emit(")");
        }

        m_writer->emit("\n");
        m_writer->dedent();
        m_writer->emit("};\n");
    }
    else
    {
        // For simple types, just emit as a single uniform
        m_writer->emit("const ");
        m_writer->emit(getName(varDecl));
        m_writer->emit(" = uniform(");
        emitTSLDefaultValue(elementType);
        m_writer->emit(");\n");
    }
}

void TSLSourceEmitter::emitSimpleFuncImpl(IRFunc* func)
{
    // Deal with decorations that need to be emitted as attributes
    IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>();
    if (entryPointDecor)
    {
        emitEntryPointAttributes(func, entryPointDecor);
    }

    emitFunctionPreambleImpl(func);
    emitFuncDecorations(func);
    emitFuncHeader(func);
    emitSemantics(func);

    // TODO: encode declaration vs. definition
    if (isDefinition(func))
    {
        // TSL uses arrow function syntax: Fn((params) => { ... })
        // The header already emitted "export const name = /*@__PURE__*/ Fn((params) => "
        m_writer->emit("\n{\n");
        m_writer->indent();

        // Need to emit the operations in the blocks of the function
        emitFunctionBody(func);

        m_writer->dedent();
        // TSL function closing: }); instead of just }
        m_writer->emit("});\n\n");
    }
    else
    {
        m_writer->emit(";\n\n");
    }
}

} // namespace Slang
