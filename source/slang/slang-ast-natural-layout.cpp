// slang-ast-natural-layout.cpp
#include "slang-ast-natural-layout.h"

#include "slang-ast-builder.h"

// For BaseInfo
#include "slang-check-impl.h"
#include "slang-compiler.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!! NaturalSize !!!!!!!!!!!!!!!!!!!!!!!!!!!! */


NaturalSize NaturalSize::operator*(Count count) const
{
    // If the count is < 0 or the size is invalid, the result is invalid
    if (isInvalid() || count < 0)
    {
        return makeInvalid();
    }

    if (count <= 0)
    {
        // If the count is 0, in effect the result doesn't take up any space
        return makeEmpty();
    }
    else
    {
        // We don't want to produce an aligned size, as we allow the last element to not
        // take up a whole stride (only up to size)
        return make(size + (getStride() * (count - 1)), alignment);
    }
}

/* static */ NaturalSize NaturalSize::makeFromBaseType(BaseType baseType)
{
    // Special case void
    if (baseType == BaseType::Void)
    {
        return makeEmpty();
    }
    if (baseType == BaseType::IntPtr || baseType == BaseType::UIntPtr)
    {
        // We don't know the size of the pointer at the AST level.
        return makeInvalid();
    }
    else
    {
        // In "natural" layout the alignment of a base type is always the same
        // as the size of the type itself
        auto info = BaseTypeInfo::getInfo(baseType);
        return make(info.sizeInBytes, info.sizeInBytes);
    }
}

/* static */ NaturalSize NaturalSize::calcUnion(NaturalSize a, NaturalSize b)
{
    const auto alignment = maxAlignment(a.alignment, b.alignment);
    Count size = (alignment == kInvalidAlignment) ? 0 : Math::Max(a.size, b.size);
    return make(size, alignment);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTNaturalLayoutContext !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

ASTNaturalLayoutContext::ASTNaturalLayoutContext(ASTBuilder* astBuilder, DiagnosticSink* sink)
    : m_astBuilder(astBuilder), m_sink(sink)
{
    // A null type always maps to invalid
    m_typeToSize.add(nullptr, NaturalSize::makeInvalid());
}

Count ASTNaturalLayoutContext::_getCount(IntVal* intVal)
{
    if (auto constIntVal = as<ConstantIntVal>(intVal))
    {
        if (constIntVal->getValue() >= 0)
        {
            return Count(constIntVal->getValue());
        }
    }

    if (m_sink)
    {
        // Could output an error
    }

    return -1;
}

NaturalSize ASTNaturalLayoutContext::calcSize(Type* type)
{
    if (auto sizePtr = m_typeToSize.tryGetValue(type))
    {
        return *sizePtr;
    }

    // Calc the size
    const NaturalSize size = _calcSizeImpl(type);

    // We want to add to the cache, but we need to special case
    // in case there is an aggregate type that `poisoned` the cache entry, to stop infinite
    // recursion.
    //
    // A requirement is that when the agg type completes it must set the cache entry, and return the
    // same result.
    if (auto foundSize = m_typeToSize.tryGetValueOrAdd(type, size))
    {
        // If there is a found size, it must match. If not we update the state as invalid.
        if (*foundSize != size)
        {
            *foundSize = NaturalSize::makeInvalid();
            return *foundSize;
        }
    }

    return size;
}

NaturalSize ASTNaturalLayoutContext::_calcSizeImpl(Type* type)
{
    if (VectorExpressionType* vecType = as<VectorExpressionType>(type))
    {
        const Count elementCount = _getCount(vecType->getElementCount());
        return (elementCount > 0) ? calcSize(vecType->getElementType()) * elementCount
                                  : NaturalSize::makeInvalid();
    }
    else if (auto matType = as<MatrixExpressionType>(type))
    {
        const Count colCount = _getCount(matType->getColumnCount());
        const Count rowCount = _getCount(matType->getRowCount());
        return (colCount > 0 && rowCount > 0)
                   ? calcSize(matType->getElementType()) * (colCount * rowCount)
                   : NaturalSize::makeInvalid();
    }
    else if (auto basicType = as<BasicExpressionType>(type))
    {
        return NaturalSize::makeFromBaseType(basicType->getBaseType());
    }
    else if (as<PtrTypeBase>(type) || as<NullPtrType>(type))
    {
        // We can't know the size of pointer types at the AST level, as it is
        // determined by the target that is not yet known.
        return NaturalSize::makeInvalid();
    }
    else if (auto arrayType = as<ArrayExpressionType>(type))
    {
        const Count elementCount = _getCount(arrayType->getElementCount());
        return (elementCount > 0) ? calcSize(arrayType->getElementType()) * elementCount
                                  : NaturalSize::makeInvalid();
    }
    else if (auto namedType = as<NamedExpressionType>(type))
    {
        return calcSize(namedType->getCanonicalType());
    }
    else if (const auto tupleType = as<TupleType>(type))
    {
        // Initialize empty
        NaturalSize size = NaturalSize::makeEmpty();

        // We can't compute the size of an abstract type pack yet.
        if (isAbstractTypePack(tupleType->getTypePack()))
        {
            return NaturalSize::makeInvalid();
        }

        // Accumulate over all the member types
        for (auto cur = 0; cur < tupleType->getMemberCount(); cur++)
        {
            const auto curSize = calcSize(tupleType->getMember(cur));
            if (!curSize)
            {
                return NaturalSize::makeInvalid();
            }
            size.append(curSize);
        }

        return size;
    }
    else if (auto optionalType = as<OptionalType>(type))
    {
        // Sometimes a type `T` has an unused bit pattern that
        // can be used to represent the null/absent optional value,
        // and for such types the size of an `Optional<T>` can be
        // the same as a `T`, by making use of that unused pattern.
        //
        if (doesTypeHaveAnUnusedBitPatternThatCanBeUsedForOptionalRepresentation(
                optionalType->getValueType()))
            return calcSize(optionalType->getValueType());

        // For all other types, an `Optional<T>` is laid out more-or-less
        // as a tuple of a `bool` and a `T`.
        //
        // TODO(tfoley): This appears to be the exact *opposite* of how
        // we should be laying out optionals if we want to be at all
        // efficient about space. For various targets and layout modes
        // (with natural layout currently being one of them), a type
        // can have "tail padding," when its size is not a multiple of
        // its alignment. In such cases laying things out as `(T, bool)`
        // can both end up takign advantage of the tail padding of `T`
        // when present *or* for types `T` that don't include tail
        // padding in their layout, but have an alignment N > 1
        // the `(T, bool)` order will then *create* N-1 bytes of tail
        // padding (that can possibly be exploited elsewhere).
        //
        NaturalSize size = NaturalSize::makeEmpty();
        size.append(calcSize(m_astBuilder->getBoolType()));
        size.append(calcSize(optionalType->getValueType()));
        return size;
    }
    else if (auto declRefType = as<DeclRefType>(type))
    {
        if (const auto enumDeclRef = declRefType->getDeclRef().as<EnumDecl>())
        {
            Type* tagType = getTagType(m_astBuilder, enumDeclRef);
            return calcSize(tagType);
        }
        else if (const auto structDeclRef = declRefType->getDeclRef().as<StructDecl>())
        {
            // Poison the cache whilst we construct
            m_typeToSize.add(type, NaturalSize::makeInvalid());

            // Initialize empty
            NaturalSize size = NaturalSize::makeEmpty();

            for (auto inherited : structDeclRef.getDecl()->getMembersOfType<InheritanceDecl>())
            {
                // Look for a struct type that it inherits from
                if (auto inheritedDeclRef = as<DeclRefType>(inherited->base.type))
                {
                    if (auto parentDecl = inheritedDeclRef->getDeclRef().as<StructDecl>())
                    {
                        // We can only inherit from one thing
                        size = calcSize(inherited->base.type);
                        if (!size)
                        {
                            return size;
                        }
                        break;
                    }
                }
            }

            // Accumulate over all of the fields
            for (auto field : structDeclRef.getDecl()->getFields())
            {
                const auto fieldSize = calcSize(field->getType());
                if (!fieldSize)
                {
                    return NaturalSize::makeInvalid();
                }
                size.append(fieldSize);
            }

            // Set the cached result to the size.
            m_typeToSize.set(type, size);

            return size;
        }
        else if (const auto typeDef = declRefType->getDeclRef().as<TypeDefDecl>())
        {
            return calcSize(typeDef.getDecl()->type);
        }
    }

    return NaturalSize::makeInvalid();
}

} // namespace Slang
