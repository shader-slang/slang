// slang-emit-cpp.h
#ifndef SLANG_EMIT_CPP_H
#define SLANG_EMIT_CPP_H

#include "slang-emit-c-like.h"
#include "slang-ir-clone.h"

#include "slang-ir-type-set.h"
#include "slang-hlsl-intrinsic-set.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

class CPPSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    typedef uint32_t SemanticUsedFlags;
    struct SemanticUsedFlag
    {
        enum Enum : SemanticUsedFlags
        {
            DispatchThreadID    = 0x01,
            GroupThreadID       = 0x02,
            GroupID             = 0x04,
        };
    };

    struct TypeDimension
    {
        bool isScalar() const { return rowCount <= 1 && colCount <= 1; }

        BaseType elemType;
        int rowCount;
        int colCount;
    };

    virtual void useType(IRType* type);
    virtual void emitCall(const HLSLIntrinsic* specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec);
    virtual void emitTypeDefinition(IRType* type);
    virtual void emitSpecializedOperationDefinition(const HLSLIntrinsic* specOp);
    
    static UnownedStringSlice getBuiltinTypeName(IROp op);
    
    SourceWriter* getSourceWriter() const { return m_writer; }

    CPPSourceEmitter(const Desc& desc);

protected:

    // Implement CLikeSourceEmitter interface
    virtual bool doesTargetSupportPtrTypes() SLANG_OVERRIDE { return true; }
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void _emitType(IRType* type, DeclaratorInfo* declarator) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual void emitPreModuleImpl() SLANG_OVERRIDE;
    virtual void emitSimpleValueImpl(IRInst* value) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;
    virtual void emitModuleImpl(IRModule* module, DiagnosticSink* sink) SLANG_OVERRIDE;
    virtual void emitSimpleFuncImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitOperandImpl(IRInst* inst, EmitOpInfo const&  outerPrec) SLANG_OVERRIDE;
    virtual void emitParamTypeImpl(IRType* type, String const& name) SLANG_OVERRIDE;
    virtual void emitGlobalRTTISymbolPrefix();
    virtual void emitWitnessTable(IRWitnessTable* witnessTable) SLANG_OVERRIDE;
    virtual void emitInterface(IRInterfaceType* interfaceType) SLANG_OVERRIDE;
    void emitComInterface(IRInterfaceType* interfaceType);
    virtual void emitRTTIObject(IRRTTIObject* rttiObject) SLANG_OVERRIDE;
    virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) SLANG_OVERRIDE;
    virtual void emitIntrinsicCallExprImpl(IRCall* inst, IRTargetIntrinsicDecoration* targetIntrinsic, EmitOpInfo const& inOuterPrec) SLANG_OVERRIDE;
    virtual void emitLoopControlDecorationImpl(IRLoopControlDecoration* decl) SLANG_OVERRIDE;
    virtual void emitFuncDecorationsImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitVarDecorationsImpl(IRInst* var) SLANG_OVERRIDE;
    virtual void emitGlobalInstImpl(IRInst* inst) SLANG_OVERRIDE;

    virtual const UnownedStringSlice* getVectorElementNames(BaseType elemType, Index elemCount);
    
    // Replaceable for classes derived from CPPSourceEmitter
    virtual SlangResult calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out);
    virtual SlangResult calcFuncName(const HLSLIntrinsic* specOp, StringBuilder& out);
    virtual SlangResult calcScalarFuncName(HLSLIntrinsic::Op op, IRBasicType* type, StringBuilder& outBuilder);

    const UnownedStringSlice* getVectorElementNames(IRVectorType* vectorType);

    void _maybeEmitSpecializedOperationDefinition(const HLSLIntrinsic* specOp);

    void _emitForwardDeclarations(const List<EmitAction>& actions);

    void _emitAryDefinition(const HLSLIntrinsic* specOp);

    // Really we don't want any of these defined like they are here, they should be defined in slang stdlib 
    void _emitAnyAllDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp);
    void _emitConstructConvertDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp);
    void _emitConstructFromScalarDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp);
    void _emitGetAtDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp);
    void _emitInitDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp);

    void _emitSignature(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp);

    void _emitInOutParamType(IRType* type, String const& name, IRType* valueType);


    UnownedStringSlice _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op op, IRType*const* argTypes, Int argCount, IRType* retType);

    static TypeDimension _getTypeDimension(IRType* type, bool vecSwap);

    void _emitAccess(const UnownedStringSlice& name, const TypeDimension& dimension, int row, int col, SourceWriter* writer);

    UnownedStringSlice _getScalarFuncName(HLSLIntrinsic::Op operation, IRBasicType* scalarType);

    UnownedStringSlice _getFuncName(const HLSLIntrinsic* specOp);

    UnownedStringSlice _getTypeName(IRType* type);
    
    SlangResult _calcCPPTextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName);

    void _emitEntryPointDefinitionStart(IRFunc* func, const String& funcName, const UnownedStringSlice& varyingTypeName);
    void _emitEntryPointDefinitionEnd(IRFunc* func);
    void _emitEntryPointGroup(const Int sizeAlongAxis[kThreadGroupAxisCount], const String& funcName);
    void _emitEntryPointGroupRange(const Int sizeAlongAxis[kThreadGroupAxisCount], const String& funcName);

    void _emitInitAxisValues(const Int sizeAlongAxis[kThreadGroupAxisCount], const UnownedStringSlice& mulName, const UnownedStringSlice& addName);

    bool _tryEmitInstExprAsIntrinsic(IRInst* inst, const EmitOpInfo& inOuterPrec);

        // Emit the actual definition (including intializer list)
        // of all the witness table objects in `pendingWitnessTableDefinitions`.
    void _emitWitnessTableDefinitions();

        /// Maybe emits 'export' (such that visible outside binary/dll) and `extern "C"` naming
    void _maybeEmitExportLike(IRInst* inst);

    HLSLIntrinsic* _addIntrinsic(HLSLIntrinsic::Op op, IRType* returnType, IRType*const* argTypes, Index argTypeCount);

    static bool _isVariable(IROp op);

    Dictionary<IRType*, StringSlicePool::Handle> m_typeNameMap;
    Dictionary<const HLSLIntrinsic*, StringSlicePool::Handle> m_intrinsicNameMap;

    IRTypeSet m_typeSet;
    RefPtr<HLSLIntrinsicOpLookup> m_opLookup;
    HLSLIntrinsicSet m_intrinsicSet;

    HashSet<const HLSLIntrinsic*> m_intrinsicEmitted;

    HashSet<IRInterfaceType*> m_interfaceTypesEmitted;

    StringSlicePool m_slicePool;

    SemanticUsedFlags m_semanticUsedFlags;

    // Witness tables pending for emitting their definitions.
    // They must be emitted last, after the entire `Context` class so those member functions defined
    // in `Context` may be referenced.
    List<IRWitnessTable*> pendingWitnessTableDefinitions;

    bool m_hasString = false;
};

}
#endif
