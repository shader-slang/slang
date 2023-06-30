// slang-ast-natural-layout.cpp
#include "slang-ast-natural-layout.h"

#include "slang-ast-builder.h"

// For BaseInfo
#include "slang-compiler.h"

namespace Slang
{

SlangResult ASTNaturalLayoutContext::_getInt(IntVal* intVal, Count& outValue)
{
    if (auto constIntVal = as<ConstantIntVal>(intVal))
    {
        outValue = Count(constIntVal->value);
        return SLANG_OK;
    }

    if (m_sink)
    {
        // Could output an error
    }

    return SLANG_FAIL;
}

SLANG_FORCE_INLINE static Count _calcAligned(Count size, Count alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

SlangResult ASTNaturalLayoutContext::calcLayout(Type* type, NaturalSize& outSize)
{
    if (!type)
    {
        return SLANG_FAIL;
    }

    if (VectorExpressionType* vecType = as<VectorExpressionType>(type))
    {
        // The number of elements
        Count elementCount;
        SLANG_RETURN_ON_FAIL(_getInt(vecType->elementCount, elementCount));
        SLANG_RETURN_ON_FAIL(calcLayout(vecType->elementType, outSize));

        outSize.stride *= elementCount;
        outSize.size = outSize.stride;
        return SLANG_OK;
    }
    else if (auto matType = as<MatrixExpressionType>(type))
    {
        Count colCount, rowCount;

        SLANG_RETURN_ON_FAIL(_getInt(matType->getRowCount(), rowCount));
        SLANG_RETURN_ON_FAIL(_getInt(matType->getColumnCount(), colCount));

        SLANG_RETURN_ON_FAIL(calcLayout(matType->getElementType(), outSize));

        outSize.stride *= colCount * rowCount;
        outSize.size = outSize.stride;
        return SLANG_OK;
    }
    else if (auto basicType = as<BasicExpressionType>(type))
    {
        auto info = BaseTypeInfo::getInfo(basicType->baseType);

        outSize.alignment = info.sizeInBytes;
        outSize.size = info.sizeInBytes;
        outSize.stride = info.sizeInBytes;
        return SLANG_OK;
    }
    else if (auto ptrType = as<PtrTypeBase>(type))
    {
        // We assume 64 bits/8 bytes across the board
        auto info = BaseTypeInfo::getInfo(BaseType::Int64);

        outSize.alignment = info.sizeInBytes;
        outSize.size = info.sizeInBytes;
        outSize.stride = info.sizeInBytes;
        return SLANG_OK;
    }
    else if (auto arrayType = as<ArrayExpressionType>(type))
    {
        // The number of elements
        Count elementCount;
        SLANG_RETURN_ON_FAIL(_getInt(arrayType->getElementCount(), elementCount));
        SLANG_RETURN_ON_FAIL(calcLayout(arrayType->getElementType(), outSize));

        if (elementCount > 0)
        {
            // We are going to be careful about the last element.

            const auto stride = outSize.stride;
            const Count strideSizeMinusOne = (elementCount - 1) * stride;
            outSize.size += strideSizeMinusOne;
            outSize.stride = strideSizeMinusOne + stride;
        }
        else
        {
            outSize.stride = 0;
            outSize.alignment = 1;
            outSize.size = 0;
        }
        return SLANG_OK;
    }
    else if (auto namedType = as<NamedExpressionType>(type))
    {
        return calcLayout(namedType->innerType, outSize);
    }
    else if( auto declRefType = as<DeclRefType>(type) )
    {
        if (const auto enumDeclRef = declRefType->declRef.as<EnumDecl>())
        {
            Type* tagType = getTagType(m_astBuilder, enumDeclRef);
            return calcLayout(tagType, outSize);
        }
        else if(const auto structDeclRef = declRefType->declRef.as<StructDecl>())
        {
            NaturalSize size;
            size.alignment = 1;
            size.stride = 0;
            size.size = 0;

            for (auto inherited : structDeclRef.getDecl()->getMembersOfType<InheritanceDecl>())
            {
                // Look for a struct type that it inherits from
                if (auto inheritedDeclRef = as<DeclRefType>(inherited->base.type))
                {
                    if (auto parentDecl = inheritedDeclRef->declRef.as<StructDecl>())
                    {
                        // We can only inherit from one thing
                        SLANG_RETURN_ON_FAIL(calcLayout(inherited->base.type, size));
                        break;
                    }
                }
            }

            Count maxAlignment = size.alignment;

            for (auto field : structDeclRef.getDecl()->getFields())
            {
                NaturalSize fieldSize;
                SLANG_RETURN_ON_FAIL(calcLayout(field->getType(), fieldSize));

                // Align and add the size
                size.size = _calcAligned(size.size, fieldSize.alignment) + fieldSize.size;

                // Work out the max alignment
                maxAlignment = Math::Max(fieldSize.alignment, maxAlignment);
            }

            // The strided size is equal to the size with alignment
            size.stride = _calcAligned(size.size, maxAlignment);
            size.alignment = maxAlignment;

            // Set the size
            outSize = size;
            return SLANG_OK;
        }
        else if (const auto typeDef = declRefType->declRef.as<TypeDefDecl>())
        {
            return calcLayout(typeDef.getDecl()->type, outSize);
        }
    }

    return SLANG_FAIL;
}

} // namespace Slang
