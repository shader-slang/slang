#ifndef SLANG_IR_TYPES_H
#define SLANG_IR_TYPES_H

#include "ir.h"
#include "type-system-shared.h"

namespace Slang
{
    struct IRBasicType : public IRType {};

    struct IRBuiltinGenericType : public IRType
    {
        IRUse elementType;
    };
    struct IRVectorType : public IRBuiltinGenericType
    {
        IRUse vectorSize;
    };
    struct IRMatrixType : public IRBuiltinGenericType
    {
        IRUse rows, cols;
    };
    struct IRArrayType : public IRBuiltinGenericType
    {
        IRUse size;
    };
    struct IRFuncType : public IRType
    {
        IRUse returnType;
        IRType* getParameterType(uint32_t i);
        int getParameterCount();
    };
    struct IRStructType : public IRType
    {
    };
    struct IRRateQualifiedType : public IRType
    {
        IRUse rate;
        IRUse valueType;
    };
    struct IRTextureBaseType : public IRBuiltinGenericType
    {
    public:
        TextureFlavor flavor;
    };
    struct IRTextureType : public IRTextureBaseType
    {};
    struct IRTextureSamplerType : public IRBuiltinGenericType
    {};
    struct IRSamplerStateType : public IRType
    {};
    struct IRGLSLImageType : public IRTextureBaseType
    {};
    struct IRPointerType : public IRBuiltinGenericType
    {};
    struct IROutBaseType : public IRPointerType
    {};
    struct IROutType : public IROutBaseType
    {};
    struct IRInOutType : public IROutBaseType
    {};
    struct IRHLSLStructuredBufferBaseType : public IRBuiltinGenericType
    {};
    struct IRHLSLStructuredBufferType : public IRHLSLStructuredBufferBaseType
    {};
    struct IRHLSLRWStructuredBufferType : public IRHLSLStructuredBufferBaseType
    {};
    struct IRHLSLAppendStructuredBufferType : public IRHLSLStructuredBufferBaseType
    {};
    struct IRHLSLConsumeStructuredBufferType : public IRHLSLStructuredBufferBaseType
    {};
    struct IRUntypedBufferResourceType : public IRType
    {};
    struct IRHLSLByteAddressBufferType : public IRUntypedBufferResourceType
    {};
    struct IRHLSLRWByteAddressBufferType : public IRUntypedBufferResourceType
    {};
    struct IRHLSLPatchType : public IRBuiltinGenericType
    {};
    struct IRHLSLInputPatchType : public IRHLSLPatchType
    {};
    struct IRHLSLOutputPatchType : public IRHLSLPatchType
    {};
    struct IRHLSLStreamOutputType : public IRBuiltinGenericType
    {};
    struct IRHLSLPointStreamType : public IRHLSLStreamOutputType
    {};
    struct IRHLSLLineStreamType : public IRHLSLStreamOutputType
    {};
    struct IRHLSLTriangleStreamType : public IRHLSLStreamOutputType
    {};
    struct IRGLSLInputAttachmentType : public IRType
    {};
    struct IRParameterGroupType : public IRPointerType
    {};
    struct IRUniformParameterGroupType : public IRParameterGroupType
    {};
    struct IRVaryingParameterGroupType : public IRParameterGroupType
    {};
    struct IRConstantBufferType : public IRParameterGroupType
    {};
    struct IRTextureBufferType : public IRParameterGroupType
    {};
    struct IRGLSLInputParameterGroupType : public IRVaryingParameterGroupType
    {};
    struct IRGLSLOutputParameterGroupType : public IRVaryingParameterGroupType
    {};
    struct IRGLSLShaderStorageBufferType : public IRUniformParameterGroupType
    {};
    struct IRParameterBlockType : public IRUniformParameterGroupType
    {};
};

#endif