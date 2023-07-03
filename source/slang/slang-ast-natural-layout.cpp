// slang-ast-natural-layout.cpp
#include "slang-ast-natural-layout.h"

#include "slang-ast-builder.h"

// For BaseInfo
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTNaturalLayoutContext !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

ASTNaturalLayoutContext::ASTNaturalLayoutContext(ASTBuilder* astBuilder, DiagnosticSink* sink):
    m_astBuilder(astBuilder),
    m_sink(sink)
{
    // A null type always maps to invalid
    m_typeToSize.add(nullptr, NaturalSize::makeInvalid());
}

Count ASTNaturalLayoutContext::_getCount(IntVal* intVal)
{
    if (auto constIntVal = as<ConstantIntVal>(intVal))
    {
        if (constIntVal->value >= 0)
        {
            return Count(constIntVal->value);
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
    // in case there is an aggregate type that `poisoned` the cache entry, to stop infinite recursion.
    // 
    // A requirement is that when the agg type completes it must set the cache entry, and return the same result.
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
        const Count elementCount = _getCount(vecType->elementCount);
        return (elementCount > 0) ? 
            calcSize(vecType->elementType) * elementCount : 
            NaturalSize::makeInvalid();
    }
    else if (auto matType = as<MatrixExpressionType>(type))
    {
        const Count colCount = _getCount(matType->getColumnCount());
        const Count rowCount = _getCount(matType->getRowCount());
        return (colCount > 0 && rowCount > 0) ? 
            calcSize(matType->getElementType()) * (colCount * rowCount) : 
            NaturalSize::makeInvalid();
    }
    else if (auto basicType = as<BasicExpressionType>(type))
    {
        // Special case void
        if (basicType->baseType == BaseType::Void)
        {
            return NaturalSize::makeEmpty();
        }
        else
        {
            // In "natural" layout the alignment of a base type is always the same
            // as the size of the type itself
            auto info = BaseTypeInfo::getInfo(basicType->baseType);
            return NaturalSize::make(info.sizeInBytes, info.sizeInBytes);
        }
    }
    else if (as<PtrTypeBase>(type) || as<NullPtrType>(type))
    {
        // We assume 64 bits/8 bytes across the board
        auto info = BaseTypeInfo::getInfo(BaseType::Int64);
        return NaturalSize::make(info.sizeInBytes, info.sizeInBytes);
    }
    else if (auto arrayType = as<ArrayExpressionType>(type))
    {
        const Count elementCount = _getCount(arrayType->getElementCount());
        return (elementCount > 0) ? 
            calcSize(arrayType->getElementType()) * elementCount : 
            NaturalSize::makeInvalid();
    }
    else if (auto namedType = as<NamedExpressionType>(type))
    {
        return calcSize(namedType->innerType);
    }
    else if( auto declRefType = as<DeclRefType>(type) )
    {
        if (const auto enumDeclRef = declRefType->declRef.as<EnumDecl>())
        {
            Type* tagType = getTagType(m_astBuilder, enumDeclRef);
            return calcSize(tagType);
        }
        else if(const auto structDeclRef = declRefType->declRef.as<StructDecl>())
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
                    if (auto parentDecl = inheritedDeclRef->declRef.as<StructDecl>())
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
        else if (const auto typeDef = declRefType->declRef.as<TypeDefDecl>())
        {
            return calcSize(typeDef.getDecl()->type);
        }
    }

    return NaturalSize::makeInvalid();
}

} // namespace Slang
