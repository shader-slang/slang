// slang-emit-cpp.h
#ifndef SLANG_EMIT_CPP_H
#define SLANG_EMIT_CPP_H

#include "slang-emit-c-like.h"
#include "slang-ir-clone.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

class CPPSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    enum class BuiltInCOp
    {
        Splat,                  //< Splat a single value to all values of a vector or matrix type
        Init,                   //< Initialize with parameters (must match the type)
    };

    CPPSourceEmitter(const Desc& desc);

protected:

    void _emitCFunc(BuiltInCOp cop, IRType* type);

    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl();

    UnownedStringSlice _getTypeName(IRType* type);
    StringSlicePool::Handle _calcTypeName(IRType* type);
    IRType* _getVectorType(IRType* elemType, int count);
    
    Dictionary<IRType*, StringSlicePool::Handle> m_typeNameMap;

    StringSlicePool m_slicePool;

    //RefPtr<IRModule> m_module;
    //SharedIRBuilder m_sharedBuilder;
    //IRBuilder m_builder;
};

}
#endif
