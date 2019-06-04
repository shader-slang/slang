// slang-emit-cpp.h
#ifndef SLANG_EMIT_CPP_H
#define SLANG_EMIT_CPP_H

#include "slang-emit-c-like.h"

namespace Slang
{

class CPPSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;


    CPPSourceEmitter(const Desc& desc) :
        Super(desc)
    {}

protected:

    void _emitCVecType(IROp op, Int size);
    void _emitCMatType(IROp op, IRIntegerValue rowCount, IRIntegerValue colCount);

    virtual void emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitMatrixTypeImpl(IRMatrixType* matType) SLANG_OVERRIDE;
};

}
#endif
