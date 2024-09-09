#pragma once

#include "slang-emit-c-like.h"

namespace Slang
{

class WGSLSourceEmitter : public CLikeSourceEmitter
{
public:

    WGSLSourceEmitter(const Desc& desc)
        : CLikeSourceEmitter(desc)
    {}

    virtual void emitParameterGroupImpl(
        IRGlobalParam* varDecl, IRUniformParameterGroupType* type
    ) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(
        IRFunc* irFunc, IREntryPointDecoration* entryPointDecor
    ) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(
        IRType* elementType, IRIntegerValue elementCount
    ) SLANG_OVERRIDE;
    virtual void emitFuncHeaderImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitSimpleValueImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(
        IRInst* inst, const EmitOpInfo& inOuterPrec
    ) SLANG_OVERRIDE;
    virtual void emitSwitchCaseSelectorsImpl(
        IRBasicType *const switchCondition,
        const SwitchRegion::Case *const currentCase,
        const bool isDefault
    ) SLANG_OVERRIDE;
    virtual void emitSimpleTypeAndDeclaratorImpl(
        IRType* type, DeclaratorInfo* declarator
    ) SLANG_OVERRIDE;
    virtual void emitVarKeywordImpl(IRType * type, const bool isConstant) SLANG_OVERRIDE;
    virtual void emitDeclaratorImpl(DeclaratorInfo* declarator) SLANG_OVERRIDE;
    virtual void emitStructDeclarationSeparatorImpl() SLANG_OVERRIDE;
    virtual void emitLayoutQualifiersImpl(IRVarLayout* layout) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;
    virtual void emitParamTypeImpl(IRType* type, const String& name) SLANG_OVERRIDE;
    virtual bool isPointerSyntaxRequiredImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void _emitType(IRType* type, DeclaratorInfo* declarator) SLANG_OVERRIDE;
    virtual void emitFrontMatterImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitStructFieldAttributes(
        IRStructType * structType, IRStructField * field
    ) SLANG_OVERRIDE;
    virtual void emitGlobalParamType(IRType* type, const String& name) SLANG_OVERRIDE;
    virtual void emitOperandImpl(
        IRInst* inst, const EmitOpInfo& outerPrec
    ) SLANG_OVERRIDE;

    void emit(const AddressSpace addressSpace);

private:

    // Emit the matrix type with 'rowCountWGSL' WGSL-rows and 'colCountWGSL' WGSL-columns
    void emitMatrixType(
        IRType *const elementType,
        const IRIntegerValue& rowCountWGSL,
        const IRIntegerValue& colCountWGSL
    );

    bool m_f16ExtensionEnabled {false};

};

} // namespace Slang
