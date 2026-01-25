// slang-emit-tsl.h
#pragma once

#include "slang-emit-c-like.h"

namespace Slang
{

/// TSL (Three.js Shading Language) source emitter.
///
/// TSL is a JavaScript-based DSL used by Three.js for shader authoring.
/// Unlike traditional shading languages (GLSL, HLSL, WGSL), TSL uses:
/// - Function composition instead of operators: `add(a, b)` instead of `a + b`
/// - Method chaining for transformations: `value.mul(2).add(1)`
/// - JavaScript functions for shader functions: `Fn(() => { ... })`
/// - Control flow constructs: `If()`, `Loop()`, `Switch()`, `Break()`, `Continue()`
///
class TSLSourceEmitter : public CLikeSourceEmitter
{
public:
    explicit TSLSourceEmitter(const Desc& desc);

    // Override key emission methods for TSL-style output

    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
        SLANG_OVERRIDE;
    virtual void emitSimpleValueImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual bool tryEmitInstStmtImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitFuncHeaderImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitFrontMatterImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;
    virtual void emitVarKeywordImpl(IRType* type, IRInst* varDecl) SLANG_OVERRIDE;
    virtual void emitOperandImpl(IRInst* operand, EmitOpInfo const& outerPrec) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(
        IRFunc* irFunc,
        IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;
    virtual void emitParameterGroupImpl(
        IRGlobalParam* varDecl,
        IRUniformParameterGroupType* type) SLANG_OVERRIDE;

    // TSL-specific helper methods
    const char* getTSLBinaryOpName(IROp op);
    const char* getTSLUnaryOpName(IROp op);
    String getTSLTypeName(IRType* type);

    void emitTSLBinaryOp(IROp op, IRInst* left, IRInst* right, const EmitOpInfo& outerPrec);
    void emitTSLUnaryOp(IROp op, IRInst* operand, const EmitOpInfo& outerPrec);
    void emitTSLCall(IRCall* call, const EmitOpInfo& outerPrec);
    void emitTSLConstructor(IRInst* inst, const EmitOpInfo& outerPrec);
    void emitTSLSwizzle(IRSwizzle* swizzle, const EmitOpInfo& outerPrec);

    void emitTSLIfElse(IRIfElse* ifElse);
    void emitTSLLoop(IRLoop* loop);
    void emitTSLSwitch(IRSwitch* switchInst);

    void addTSLImport(const char* name);

private:
    HashSet<String> m_tslImports;
    bool m_inFunctionBody = false;
};

} // namespace Slang
