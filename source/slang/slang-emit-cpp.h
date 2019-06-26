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

    struct HLSLType
    {
        typedef HLSLType ThisType;

        static ThisType makeVec(IROp inElementType, int inCount)
        {
            return ThisType{ uint8_t(kIROp_VectorType), uint8_t(inElementType), uint8_t(inCount), 0 };
        }
        static ThisType makeMatrix(IROp inElementType, int inRowsCount, int inColsCount)
        {
            return ThisType{ uint8_t(kIROp_MatrixType), uint8_t(inElementType), uint8_t(inColsCount), uint8_t(inRowsCount) };
        }
        static ThisType makeBasic(IROp inType)
        {
            return ThisType{ uint8_t(inType), 0, 0, 0 };
        }

        uint32_t getOrder() const
        {
            switch (op)
            {
                case kIROp_MatrixType:      return uint32_t(0x02000000) | (uint32_t(elementType) << 16) | (uint32_t(sizeOrColCount) << 8) | rowCount;
                case kIROp_VectorType:      return uint32_t(0x01000000) | (uint32_t(elementType) << 16) | (uint32_t(sizeOrColCount) << 8);
                default:                    return                        (uint32_t(op) << 16);
            }
        }
            /// Just fit into a uint32_t
        uint32_t getCompressed() const { return (uint32_t(op) << 24) | (uint32_t(elementType) << 16) | (uint32_t(sizeOrColCount) << 8) | uint32_t(rowCount); }

            /// It's better than a hash in that one to one mapping between 'type' and hash
        UInt GetHashCode() const { return getCompressed(); }

        bool operator==(const ThisType& rhs) const { return getCompressed() == rhs.getCompressed(); }
        bool operator!=(const ThisType& rhs) const { return getCompressed() != rhs.getCompressed(); }

        uint8_t op;
        uint8_t elementType;
        uint8_t sizeOrColCount;
        uint8_t rowCount;
    };

    enum class BuiltInCOp
    {
        Splat,                  //< Splat a single value to all values of a vector or matrix type
        Init,                   //< Initialize with parameters (must match the type)
    };

    CPPSourceEmitter(const Desc& desc);

    static UnownedStringSlice getBuiltinTypeName(IROp op);

protected:

    void _emitCFunc(BuiltInCOp cop, IRType* type);

    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl();

    UnownedStringSlice _getTypeName(const HLSLType& type);
    StringSlicePool::Handle _calcTypeName(const HLSLType& type);

    Dictionary<HLSLType, StringSlicePool::Handle> m_typeNameMap;

    StringSlicePool m_slicePool;
};

}
#endif
