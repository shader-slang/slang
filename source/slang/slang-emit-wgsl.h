#pragma once

#include "slang-emit-c-like.h"

namespace Slang
{

class WGSLSourceEmitter : public CLikeSourceEmitter
{
public:
    WGSLSourceEmitter(const Desc& desc)
        : CLikeSourceEmitter(desc)
    {
    }

    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
        SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(
        IRFunc* irFunc,
        IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
        SLANG_OVERRIDE;
    virtual void emitFuncHeaderImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitSimpleValueImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual bool tryEmitInstStmtImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitSwitchCaseSelectorsImpl(const SwitchRegion::Case* currentCase, bool isDefault)
        SLANG_OVERRIDE;
    virtual void emitSimpleTypeAndDeclaratorImpl(IRType* type, DeclaratorInfo* declarator)
        SLANG_OVERRIDE;
    virtual void emitVarKeywordImpl(IRType* type, IRInst* varDecl) SLANG_OVERRIDE;
    virtual void emitDeclaratorImpl(DeclaratorInfo* declarator) SLANG_OVERRIDE;
    virtual void emitOperandImpl(IRInst* operand, EmitOpInfo const& outerPrec) SLANG_OVERRIDE;
    virtual void emitStructDeclarationSeparatorImpl() SLANG_OVERRIDE;
    virtual void emitLayoutQualifiersImpl(IRVarLayout* layout) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;
    virtual void emitParamTypeImpl(IRType* type, const String& name) SLANG_OVERRIDE;
    virtual void _emitType(IRType* type, DeclaratorInfo* declarator) SLANG_OVERRIDE;
    virtual void emitFrontMatterImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitSemanticsPrefixImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitStructFieldAttributes(IRStructType* structType, IRStructField* field)
        SLANG_OVERRIDE;
    virtual void emitCallArg(IRInst* inst) SLANG_OVERRIDE;

    virtual void emitIntrinsicCallExprImpl(
        IRCall* inst,
        UnownedStringSlice intrinsicDefinition,
        IRInst* intrinsicInst,
        EmitOpInfo const& inOuterPrec) SLANG_OVERRIDE;

    void emit(const AddressSpace addressSpace);

    virtual bool shouldFoldInstIntoUseSites(IRInst* inst) SLANG_OVERRIDE;
    Dictionary<const char*, IRStringLit*> m_builtinPreludes;

protected:
    void ensurePrelude(const char* preludeText);

private:
    bool maybeEmitSystemSemantic(IRInst* inst);

    // Emit the matrix type with 'rowCountWGSL' WGSL-rows and 'colCountWGSL' WGSL-columns
    void emitMatrixType(
        IRType* const elementType,
        const IRIntegerValue& rowCountWGSL,
        const IRIntegerValue& colCountWGSL);

    bool m_f16ExtensionEnabled = false;
};

} // namespace Slang
