// Slang HLSL compatibility library

typedef uint UINT;

// Access float2/float3 as a float3
[ForceInline] float3 __asFloat3(float2 v) { return float3(v, 0); }
[ForceInline] float3 __asFloat3(float3 v) { return v; }

__generic<T>
__intrinsic_op($(kIROp_StructuredBufferGetDimensions))
uint2 __structuredBufferGetDimensions(AppendStructuredBuffer<T> buffer);

__generic<T>
__intrinsic_op($(kIROp_StructuredBufferGetDimensions))
uint2 __structuredBufferGetDimensions(ConsumeStructuredBuffer<T> buffer);

__intrinsic_op($(kIROp_StructuredBufferGetDimensions))
uint2 __structuredBufferGetDimensions<T>(StructuredBuffer<T> buffer);

__intrinsic_op($(kIROp_StructuredBufferGetDimensions))
uint2 __structuredBufferGetDimensions<T>(RWStructuredBuffer<T> buffer);

__intrinsic_op($(kIROp_StructuredBufferGetDimensions))
uint2 __structuredBufferGetDimensions<T>(RasterizerOrderedStructuredBuffer<T> buffer);

__generic<T>
__magic_type(HLSLAppendStructuredBufferType)
__intrinsic_type($(kIROp_HLSLAppendStructuredBufferType))
struct AppendStructuredBuffer
{
    __intrinsic_op($(kIROp_StructuredBufferAppend))
    void Append(T value);

    [ForceInline]
    void GetDimensions(
        out uint numStructs,
        out uint stride)
    {
        let result = __structuredBufferGetDimensions(this);
        numStructs = result.x;
        stride = result.y;
    }
};

__magic_type(HLSLByteAddressBufferType)
__intrinsic_type($(kIROp_HLSLByteAddressBufferType))
struct ByteAddressBuffer
{
    [__readNone]
    __target_intrinsic(hlsl)
    __target_intrinsic(cpp)
    __target_intrinsic(cuda)
    [__unsafeForceInlineEarly]
    void GetDimensions(out uint dim);

    [__unsafeForceInlineEarly]
    __specialized_for_target(spirv)
    __specialized_for_target(glsl)
    void GetDimensions(out uint dim)
    {
        dim = __structuredBufferGetDimensions(__getEquivalentStructuredBuffer<uint>(this)).x*4;
    }

    [__readNone]
    [ForceInline]
    __target_intrinsic(hlsl)
    uint Load(int location)
    {
        return __byteAddressBufferLoad<uint>(this, location);
    }

    [__readNone]
    uint Load(int location, out uint status);

    [__readNone]
    [ForceInline]
    __target_intrinsic(hlsl)
    uint2 Load2(int location)
    {
        return __byteAddressBufferLoad<uint2>(this, location);
    }

    [__readNone]
    uint2 Load2(int location, out uint status);

    [__readNone]
    [ForceInline]
    __target_intrinsic(hlsl)
    uint3 Load3(int location)
    {
        return __byteAddressBufferLoad<uint3>(this, location);
    }

    [__readNone]
    uint3 Load3(int location, out uint status);

    [__readNone]
    [ForceInline]
    __target_intrinsic(hlsl)
    uint4 Load4(int location)
    {
        return __byteAddressBufferLoad<uint4>(this, location);
    }

    [__readNone]
    uint4 Load4(int location, out uint status);

    [__readNone]
    T Load<T>(int location)
    {
        return __byteAddressBufferLoad<T>(this, location);
    }
};

// Texture
[sealed]
[builtin]
interface __ITextureShape
{
    static const int flavor;
    static const int dimensions;
    static const int planeDimensions;
}
__magic_type(TextureShape1DType)
__intrinsic_type($(kIROp_TextureShape1DType))
struct __Shape1D : __ITextureShape
{
    static const int flavor = $(SLANG_TEXTURE_1D);
    static const int dimensions = 1;
    static const int planeDimensions = 1;
}
__magic_type(TextureShape2DType)
__intrinsic_type($(kIROp_TextureShape2DType))
struct __Shape2D : __ITextureShape
{
    static const int flavor = $(SLANG_TEXTURE_2D);
    static const int dimensions = 2;
    static const int planeDimensions = 2;
}
__magic_type(TextureShape3DType)
__intrinsic_type($(kIROp_TextureShape3DType))
struct __Shape3D : __ITextureShape
{
    static const int flavor = $(SLANG_TEXTURE_3D);
    static const int dimensions = 3;
    static const int planeDimensions = 3;
}
__magic_type(TextureShapeCubeType)
__intrinsic_type($(kIROp_TextureShapeCubeType))
struct __ShapeCube : __ITextureShape
{
    static const int flavor = $(SLANG_TEXTURE_CUBE);
    static const int dimensions = 3;
    static const int planeDimensions = 2;
}
__magic_type(TextureShapeBufferType)
__intrinsic_type($(kIROp_TextureShapeBufferType))
struct __ShapeBuffer : __ITextureShape
{
    static const int flavor = $(SLANG_TEXTURE_BUFFER);
    static const int dimensions = 1;
    static const int planeDimensions = 1;
}
__intrinsic_op(vectorReshape)
vector<T,N> __vectorReshape<let N : int, T, let M : int>(vector<T,M> vin);

__intrinsic_op(makeVector)
__generic<T, let N:int>
vector<T,N+1> __makeVector(vector<T,N> vec, T scalar);


__magic_type(TextureType)
__intrinsic_type($(kIROp_TextureType))
struct __TextureImpl<T, Shape: __ITextureShape, let isArray:int, let isMS:int, let sampleCount:int, let access:int, let isShadow:int, let isCombined:int, let format:int>
{
}

// Combined texture sampler specific functions
__generic<T, Shape: __ITextureShape, let isArray:int, let isMS:int, let sampleCount:int, let isShadow:int, let format:int>
extension __TextureImpl<T,Shape,isArray,isMS,sampleCount,0,isShadow,1,format>
{
    static const int access = 0;

    typealias TextureCoord = vector<float, Shape.dimensions>;

    __target_intrinsic(glsl, "textureQueryLod($0, $1).x")
    float CalculateLevelOfDetail(TextureCoord location);

    __target_intrinsic(glsl, "textureQueryLod($0, $1).y")
    float CalculateLevelOfDetailUnclamped(TextureCoord location);

    [__readNone]
    T Sample(vector<float, Shape.dimensions+isArray> location)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Sample";
            case glsl:
                __intrinsic_asm "$ctexture($0, $1)$z";
            case cuda:
                if (isArray != 0)
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1DLayered<$T0>($0, ($1).x, int(($1).y))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2DLayered<$T0>($0, ($1).x, ($1).y, int(($1).z))";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemapLayered<$T0>($0, ($1).x, ($1).y, ($1).z, int(($1).w))";
                    }
                }
                else
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1D<$T0>($0, ($1))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2D<$T0>($0, ($1).x, ($1).y)";
                    case $(SLANG_TEXTURE_3D):
                        __intrinsic_asm "tex3D<$T0>($0, ($1).x, ($1).y, ($1).z)";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemap<$T0>($0, ($1).x, ($1).y, ($1).z)";
                    }
                }
            case spirv:
                return spirv_asm
                {
                    %sampled : __sampledType(T) = OpImageSampleImplicitLod $this $location None;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    __glsl_extension(GL_ARB_sparse_texture_clamp)
    T Sample(vector<float, Shape.dimensions+isArray> location, vector<int, Shape.planeDimensions> offset, float clamp)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Sample";
            case glsl:
                __intrinsic_asm "$ctextureOffsetClampARB($0, $1, $2, $3)$z";
            case spirv:
                return spirv_asm
                {
                    OpCapability MinLod;
                    %sampled : __sampledType(T) = OpImageSampleImplicitLod $this $location None|ConstOffset|MinLod $offset $clamp;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    __target_intrinsic(hlsl)
    T Sample(vector<float, Shape.dimensions+isArray> location, vector<int, Shape.planeDimensions> offset, float clamp, out uint status)
    {
        status = 0;
        return Sample(location, offset, clamp);
    }

    [__readNone]
    T SampleBias(vector<float, Shape.dimensions+isArray> location, float bias)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".SampleBias";
            case glsl:
                __intrinsic_asm "$ctexture($0, $1, $2)$z";
            case spirv:
                return spirv_asm
                {
                    %sampled : __sampledType(T) = OpImageSampleImplicitLod $this $location None|Bias $bias;
                    __truncate $$T result __sampledType(T) %sampled;

                };
        }
    }

    [__readNone]
    T SampleBias(vector<float, Shape.dimensions+isArray> location, float bias, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleBias";
            case glsl:
            __intrinsic_asm "$ctextureOffset($0, $1, $3, $2)$z";
            case spirv:
            return spirv_asm
            {
                %sampled : __sampledType(T) = OpImageSampleImplicitLod $this $location None|Bias|ConstOffset $bias $offset;
                __truncate $$T result __sampledType(T) %sampled;
            };
        }
    }

    __target_intrinsic(glsl, "texture($0, $1)")
    float __glsl_texture(vector<float, Shape.dimensions+isArray+1> value);

    __glsl_extension(GL_EXT_texture_shadow_lod)
    __target_intrinsic(glsl, "textureOffset($0, $1, $2)")
    float __glsl_texture_offset(vector<float, Shape.dimensions+isArray+1> value, constexpr vector<int, Shape.planeDimensions> offset);

    __glsl_extension(GL_EXT_texture_shadow_lod)
    __target_intrinsic(glsl, "textureLod($0, $1, 0)")
    float __glsl_texture_level_zero(vector<float, Shape.dimensions+isArray+1> value);

    __glsl_extension(GL_EXT_texture_shadow_lod)
    __target_intrinsic(glsl, "textureLodOffset($0, $1, 0, $2)")
    float __glsl_texture_offset_level_zero(vector<float, Shape.dimensions+isArray+1> value, constexpr vector<int, Shape.planeDimensions> offset);

    [__readNone]
    [ForceInline]
    float SampleCmp(vector<float, Shape.dimensions+isArray> location, float compareValue)
    {
        __target_switch
        {
        case glsl:
            __glsl_texture(__makeVector(location, compareValue));
        case hlsl:
            __intrinsic_asm ".SampleCmp";
        }
    }

    [__readNone]
    [ForceInline]
    float SampleCmpLevelZero(vector<float, Shape.dimensions+isArray> location, float compareValue)
    {
        __target_switch
        {
        case glsl:
            __glsl_texture_level_zero(__makeVector(location, compareValue));
        case hlsl:
            __intrinsic_asm ".SampleCmpLevelZero";
        }
    }

    [__readNone] 
    [ForceInline]
    float SampleCmp(vector<float, Shape.dimensions+isArray> location, float compareValue, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
        case glsl:
            __glsl_texture_offset(__makeVector(location, compareValue), offset);
        case hlsl:
            __intrinsic_asm ".SampleCmp";
        }
    }

    [__readNone]
    [ForceInline]
    float SampleCmpLevelZero(vector<float, Shape.dimensions+isArray> location, float compareValue, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
        case glsl:
            __glsl_texture_offset_level_zero(__makeVector(location, compareValue), offset);
        case hlsl:
            __intrinsic_asm ".SampleCmpLevelZero";
        }
    }

    [__readNone]
    T SampleGrad(vector<float, Shape.dimensions+isArray> location, vector<float, Shape.planeDimensions> gradX, vector<float, Shape.planeDimensions> gradY)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleGrad";
            case glsl:
            __intrinsic_asm "$ctextureGrad($0, $1, $2, $3)$z";
            case spirv:
                return spirv_asm
                {
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod $this $location None|Grad $gradX $gradY;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    T SampleGrad(vector<float, Shape.dimensions+isArray> location, vector<float, Shape.planeDimensions> gradX, vector<float, Shape.planeDimensions> gradY, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleGrad";
            case glsl:
            __intrinsic_asm "$ctextureGradOffset($0, $1, $2, $3, $4)$z";
            case spirv:
                return spirv_asm
                {
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod $this $location None|Grad|ConstOffset $gradX $gradY $offset;
                    __truncate $$T result __sampledType(T) %sampled;

                };
        }
    }

    __glsl_extension(GL_ARB_sparse_texture_clamp)
    [__readNone]
    T SampleGrad(vector<float, Shape.dimensions+isArray> location, vector<float, Shape.planeDimensions> gradX, vector<float, Shape.planeDimensions> gradY, constexpr vector<int, Shape.planeDimensions> offset, float lodClamp)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleGrad";
            case glsl:
            __intrinsic_asm "$ctextureGradOffsetClampARB($0, $1, $2, $3, $4, $5)$z";
            case spirv:
                return spirv_asm
                {
                    OpCapability MinLod;
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod $this $location None|Grad|ConstOffset|MinLod $gradX $gradY $offset $lodClamp;
                    __truncate $$T result __sampledType(T) %sampled;

                };
        }
    }

    [__readNone]
    T SampleLevel(vector<float, Shape.dimensions+isArray> location, float level)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".SampleLevel";
            case glsl:
                __intrinsic_asm "$ctextureLod($0, $1, $2)$z";
            case cuda:
                if (isArray != 0)
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1DLayeredLod<$T0>($0, ($1).x, int(($1).y), ($2))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2DLayeredLod<$T0>($0, ($1).x, ($1).y, int(($1).z), ($2))";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemapLayeredLod<$T0>($0, ($1).x, ($1).y, ($1).z, int(($1).w), ($2))";
                    default:
                        __intrinsic_asm "<invalid intrinsic>";
                    }
                }
                else
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1DLod<$T0>($0, ($1), ($2))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2DLod<$T0>($0, ($1).x, ($1).y, ($2))";
                    case $(SLANG_TEXTURE_3D):
                        __intrinsic_asm "tex3DLod<$T0>($0, ($1).x, ($1).y, ($1).z, ($2))";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemapLod<$T0>($0, ($1).x, ($1).y, ($1).z, ($2))";
                    }
                }
            case spirv:
                return spirv_asm
                {
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod $this $location None|Lod $level;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    T SampleLevel(vector<float, Shape.dimensions+isArray> location, float level, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".SampleLevel";
            case glsl:
                __intrinsic_asm "$ctextureLodOffset($0, $1, $2, $3)$z";
            case spirv:
                return spirv_asm
                {
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod $this $location None|Lod|ConstOffset $level $offset;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }
}

// Non-combined texture types specific functions
__generic<T, Shape: __ITextureShape, let isArray:int, let isMS:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
extension __TextureImpl<T,Shape,isArray,isMS,sampleCount,access,isShadow,0,format>
{
    typealias TextureCoord = vector<float, Shape.dimensions>;

    __target_intrinsic(glsl, "textureQueryLod($p, $2).x")
    float CalculateLevelOfDetail(SamplerState s, TextureCoord location);

    __target_intrinsic(glsl, "textureQueryLod($p, $2).y")
    float CalculateLevelOfDetailUnclamped(SamplerState s, TextureCoord location);

    [__readNone]
    T Sample(SamplerState s, vector<float, Shape.dimensions+isArray> location)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Sample";
            case glsl:
                __intrinsic_asm "$ctexture($p, $2)$z";
            case cuda:
                if (isArray != 0)
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1DLayered<$T0>($0, ($2).x, int(($2).y))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2DLayered<$T0>($0, ($2).x, ($2).y, int(($2).z))";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemapLayered<$T0>($0, ($2).x, ($2).y, ($2).z, int(($2).w))";
                    default:
                        __intrinsic_asm "<invalid intrinsic>";
                    }
                }
                else
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1D<$T0>($0, ($2))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2D<$T0>($0, ($2).x, ($2).y)";
                    case $(SLANG_TEXTURE_3D):
                        __intrinsic_asm "tex3D<$T0>($0, ($2).x, ($2).y, ($2).z)";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemap<$T0>($0, ($2).x, ($2).y, ($2).z)";
                    }
                }
            case spirv:
                return spirv_asm
                {
                    %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                    %sampled : __sampledType(T) = OpImageSampleImplicitLod %sampledImage $location None;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    T Sample(SamplerState s, vector<float, Shape.dimensions+isArray> location, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Sample";
            case glsl:
                __intrinsic_asm "$ctextureOffset($p, $2, $3)$z";
            case spirv:
            return spirv_asm
            {
                %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                %sampled : __sampledType(T) = OpImageSampleImplicitLod %sampledImage $location None|ConstOffset $offset;
                __truncate $$T result __sampledType(T) %sampled;
            };
        }
    }

    [__readNone]
    __glsl_extension(GL_ARB_sparse_texture_clamp)
    T Sample(SamplerState s, vector<float, Shape.dimensions+isArray> location, constexpr vector<int, Shape.planeDimensions> offset, float clamp)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Sample";
            case glsl:
                __intrinsic_asm "$ctextureOffsetClampARB($p, $2, $3, $4)$z";
            case spirv:
                return spirv_asm
                {
                    OpCapability MinLod;
                    %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                    %sampled : __sampledType(T) = OpImageSampleImplicitLod %sampledImage $location None|ConstOffset|MinLod $offset $clamp;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    __target_intrinsic(hlsl)
    T Sample(SamplerState s, vector<float, Shape.dimensions+isArray> location, constexpr vector<int, Shape.planeDimensions> offset, float clamp, out uint status)
    {
        status = 0;
        return Sample(s, location, offset, clamp);
    }

    [__readNone]
    T SampleBias(SamplerState s, vector<float, Shape.dimensions+isArray> location, float bias)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleBias";
            case glsl:
            __intrinsic_asm "$ctexture($p, $2, $3)$z";
            case spirv:
            return spirv_asm
            {
                %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                %sampled : __sampledType(T) = OpImageSampleImplicitLod %sampledImage $location None|Bias $bias;
                __truncate $$T result __sampledType(T) %sampled;

            };
        }
    }

    [__readNone]
    T SampleBias(SamplerState s, vector<float, Shape.dimensions+isArray> location, float bias, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleBias";
            case glsl:
            __intrinsic_asm "$ctextureOffset($p, $2, $4, $3)$z";
            case spirv:
            return spirv_asm
            {
                %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                %sampled : __sampledType(T) = OpImageSampleImplicitLod %sampledImage $location None|Bias|ConstOffset $bias $offset;
                __truncate $$T result __sampledType(T) %sampled;
            };
        }
    }

    __target_intrinsic(glsl, "texture($p, $2)")
    float __glsl_texture(SamplerComparisonState s, vector<float, Shape.dimensions+isArray+1> value);

    __glsl_extension(GL_EXT_texture_shadow_lod)
    __target_intrinsic(glsl, "textureOffset($p, $2, $3)")
    float __glsl_texture_offset(SamplerComparisonState s, vector<float, Shape.dimensions+isArray+1> value, constexpr vector<int, Shape.planeDimensions> offset);

    __glsl_extension(GL_EXT_texture_shadow_lod)
    __target_intrinsic(glsl, "textureLod($p, $2, 0)")
    float __glsl_texture_level_zero(SamplerComparisonState s, vector<float, Shape.dimensions+isArray+1> value);

    __glsl_extension(GL_EXT_texture_shadow_lod)
    __target_intrinsic(glsl, "textureLodOffset($p, $2, 0, $3)")
    float __glsl_texture_offset_level_zero(SamplerComparisonState s, vector<float, Shape.dimensions+isArray+1> value, constexpr vector<int, Shape.planeDimensions> offset);

    [__readNone] [ForceInline]
    float SampleCmp(SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, float compareValue)
    {
        __target_switch
        {
        case glsl:
            return __glsl_texture(s, __makeVector(location,compareValue));
        case hlsl:
            __intrinsic_asm ".SampleCmp";
        }
    }

    [__readNone] [ForceInline]
    float SampleCmpLevelZero(SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, float compareValue)
    {
        __target_switch
        {
        case glsl:
            return __glsl_texture_level_zero(s, __makeVector(location,compareValue));
        case hlsl:
            __intrinsic_asm ".SampleCmpLevelZero";
        }
    }

    [__readNone] [ForceInline]
    float SampleCmp(SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, float compareValue, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
        case glsl:
            return __glsl_texture_offset(s, __makeVector(location,compareValue), offset);
        case hlsl:
            __intrinsic_asm ".SampleCmpLevelZero";
        }
    }

    [__readNone] [ForceInline]
    float SampleCmpLevelZero(SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, float compareValue, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
        case glsl:
            return __glsl_texture_offset_level_zero(s, __makeVector(location,compareValue), offset);
        case hlsl:
            __intrinsic_asm ".SampleCmpLevelZero";
        }
    }

    [__readNone]
    T SampleGrad(SamplerState s, vector<float, Shape.dimensions+isArray> location, vector<float, Shape.planeDimensions> gradX, vector<float, Shape.planeDimensions> gradY)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleGrad";
            case glsl:
            __intrinsic_asm "$ctextureGrad($p, $2, $3, $4)$z";
            case spirv:
                return spirv_asm
                {
                    %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod %sampledImage $location None|Grad $gradX $gradY;
                    __truncate $$T result __sampledType(T) %sampled;

                };
        }
    }

    [__readNone]
    T SampleGrad(SamplerState s, vector<float, Shape.dimensions+isArray> location, vector<float, Shape.planeDimensions> gradX, vector<float, Shape.planeDimensions> gradY, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleGrad";
            case glsl:
            __intrinsic_asm "$ctextureGradOffset($p, $2, $3, $4, $5)$z";
            case spirv:
                return spirv_asm
                {
                    %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod %sampledImage $location None|Grad|ConstOffset $gradX $gradY $offset;
                    __truncate $$T result __sampledType(T) %sampled;

                };
        }
    }

    __glsl_extension(GL_ARB_sparse_texture_clamp)
    [__readNone]
    T SampleGrad(SamplerState s, vector<float, Shape.dimensions+isArray> location, vector<float, Shape.planeDimensions> gradX, vector<float, Shape.planeDimensions> gradY, constexpr vector<int, Shape.planeDimensions> offset, float lodClamp)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".SampleGrad";
            case glsl:
            __intrinsic_asm "$ctextureGradOffsetClampARB($p, $2, $3, $4, $5, $6)$z";
            case spirv:
                return spirv_asm
                {
                    OpCapability MinLod;
                    %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod %sampledImage $location None|Grad|ConstOffset|MinLod $gradX $gradY $offset $lodClamp;
                    __truncate $$T result __sampledType(T) %sampled;

                };
        }
    }

    [__readNone]
    T SampleLevel(SamplerState s, vector<float, Shape.dimensions+isArray> location, float level)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".SampleLevel";
            case glsl:
                __intrinsic_asm "$ctextureLod($p, $2, $3)$z";
            case cuda:
                if (isArray != 0)
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1DLayeredLod<$T0>($0, ($2).x, int(($2).y), ($3))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2DLayeredLod<$T0>($0, ($2).x, ($2).y, int(($2).z), ($3))";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemapLayeredLod<$T0>($0, ($2).x, ($2).y, ($2).z, int(($2).w), ($3))";
                    default:
                        __intrinsic_asm "<invalid intrinsic>";
                    }
                }
                else
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "tex1DLod<$T0>($0, ($2), ($3))";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "tex2DLod<$T0>($0, ($2).x, ($2).y, ($3))";
                    case $(SLANG_TEXTURE_3D):
                        __intrinsic_asm "tex3DLod<$T0>($0, ($2).x, ($2).y, ($2).z, ($3))";
                    case $(SLANG_TEXTURE_CUBE):
                        __intrinsic_asm "texCubemapLod<$T0>($0, ($2).x, ($2).y, ($2).z, ($3))";
                    }
                }
            case spirv:
            return spirv_asm
            {
                %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                %sampled : __sampledType(T) = OpImageSampleExplicitLod %sampledImage $location None|Lod $level;
                __truncate $$T result __sampledType(T) %sampled;

            };
        }
    }

    [__readNone]
    T SampleLevel(SamplerState s, vector<float, Shape.dimensions+isArray> location, float level, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".SampleLevel";
            case glsl:
                __intrinsic_asm "$ctextureLodOffset($p, $2, $3, $4)$z";
            case spirv:
                return spirv_asm
                {
                    %sampledImage : __sampledImageType(this) = OpSampledImage $this $s;
                    %sampled : __sampledType(T) = OpImageSampleExplicitLod %sampledImage $location None|Lod|ConstOffset $level $offset;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }
}

// Texture.GetDimensions
${{{{
const char* kTextureShapeTypeNames[] = {
    "__Shape1D", "__Shape2D", "__Shape3D", "__ShapeCube"};
for (int shapeIndex = 0; shapeIndex < 4; shapeIndex++)
for (int isArray = 0; isArray <= 1; isArray++)
for (int isMS = 0; isMS <= 1; isMS++) {
    if (isMS)
    {
        if (shapeIndex != kStdlibShapeIndex2D)
            continue;
    }
    if (isArray)
    {
        if (shapeIndex == kStdlibShapeIndex3D)
            continue;
    }
    auto shapeTypeName = kTextureShapeTypeNames[shapeIndex];
    TextureTypeInfo textureTypeInfo(kBaseTextureShapes[shapeIndex], isArray, isMS, 0, sb, path);
}}}}

__generic<T, let sampleCount:int, let access:int, let isShadow:int, let isCombined:int, let format:int>
extension __TextureImpl<T,$(shapeTypeName),$(isArray),$(isMS),sampleCount,access,isShadow,isCombined,format>
{
    ${{{{
    textureTypeInfo.writeGetDimensionFunctions();
    }}}}
}

${{{{
}
}}}}

// Texture.GetSamplePosition(int s);
__generic<T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let isCombined:int, let format:int>
extension __TextureImpl<T,Shape,isArray,1,sampleCount,access,isShadow,isCombined,format>
{
    float2 GetSamplePosition(int s);
}

__intrinsic_op($(kIROp_MakeArray))
Array<T,4> __makeArray<T>(T v0, T v1, T v2, T v3);

// Gather for scalar textures.
__generic<TElement, T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
[ForceInline]
vector<TElement,4> __glsl_gather(__TextureImpl<T, Shape, isArray, 0, sampleCount, access, isShadow, 0, format> texture, SamplerState s, vector<float, Shape.dimensions+isArray> location, int component)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "textureGather($p, $2, $3)";
    case spirv:
        return spirv_asm {
            %sampledImage : __sampledImageType(texture) = OpSampledImage $texture $s;
            result:$$vector<TElement,4> = OpImageGather %sampledImage $location $component;
        };
    }
}
__generic<TElement, T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
[ForceInline]
vector<TElement,4> __glsl_gather_offset(__TextureImpl<T, Shape, isArray, 0, sampleCount, access, isShadow, 0, format> texture, SamplerState s, vector<float, Shape.dimensions+isArray> location, int component, vector<int, Shape.planeDimensions> offset)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "textureGatherOffset($p, $2, $3, $4)";
    case spirv:
        return spirv_asm {
            %sampledImage : __sampledImageType(texture) = OpSampledImage $texture $s;
            result:$$vector<TElement,4> = OpImageGather %sampledImage $location $component ConstOffset $offset;
        };
    }
}
__generic<TElement, T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
[ForceInline]
vector<TElement,4> __glsl_gather_offsets(__TextureImpl<T, Shape, isArray, 0, sampleCount, access, isShadow, 0, format> texture, SamplerState s, vector<float, Shape.dimensions+isArray> location, int component,
    vector<int, Shape.planeDimensions> offset1,
    vector<int, Shape.planeDimensions> offset2,
    vector<int, Shape.planeDimensions> offset3,
    vector<int, Shape.planeDimensions> offset4)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "textureGatherOffsets($p, $2, $3, $T4[]($4, $5, $6, $7))";
    case spirv:
        let offsets = __makeArray(offset1,offset2,offset3,offset4);
        return spirv_asm {
            OpCapability ImageGatherExtended;
            %sampledImage : __sampledImageType(texture) = OpSampledImage $texture $s;
            result:$$vector<TElement,4> = OpImageGather %sampledImage $location $component ConstOffsets $offsets;
        };
    }
}
__generic<TElement, T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
[ForceInline]
vector<TElement,4> __glsl_gatherCmp(__TextureImpl<T, Shape, isArray, 0, sampleCount, access, isShadow, 0, format> texture, SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, int componentIndex, TElement compareValue)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "textureGather($p, $2, $4)";
    case spirv:
        return spirv_asm {
            %sampledImage : __sampledImageType(texture) = OpSampledImage $texture $s;
            result:$$vector<TElement,4> = OpImageDrefGather %sampledImage $location $compareValue;
        };
    }
}
__generic<TElement, T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
[ForceInline]
vector<TElement,4> __glsl_gatherCmp_offset(__TextureImpl<T, Shape, isArray, 0, sampleCount, access, isShadow, 0, format> texture, SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, int componentIndex, TElement compareValue, vector<int, Shape.planeDimensions> offset)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "textureGatherOffset($p, $2, $4, $5)";
    case spirv:
        return spirv_asm {
            %sampledImage : __sampledImageType(texture) = OpSampledImage $texture $s;
            result:$$vector<TElement,4> = OpImageDrefGather %sampledImage $location $compareValue ConstOffset $offset;
        };
    }
}
__generic<TElement, T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let access:int, let isShadow:int, let format:int>
[ForceInline]
vector<TElement,4> __glsl_gatherCmp_offsets(__TextureImpl<T, Shape, isArray, 0, sampleCount, access, isShadow, 0, format> texture, SamplerComparisonState s, vector<float, Shape.dimensions+isArray> location, int componentIndex, TElement compareValue,
    vector<int, Shape.planeDimensions> offset1,
    vector<int, Shape.planeDimensions> offset2,
    vector<int, Shape.planeDimensions> offset3,
    vector<int, Shape.planeDimensions> offset4
    )
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "textureGatherOffsets($p, $2, $4, $T5[]($5, $6, $7, $8))";
    case spirv:
        let offsets = __makeArray(offset1,offset2,offset3,offset4);
        return spirv_asm {
            OpCapability ImageGatherExtended;
            %sampledImage : __sampledImageType(texture) = OpSampledImage $texture $s;
            result:$$vector<TElement,4> = OpImageDrefGather %sampledImage $location $compareValue ConstOffsets $offsets;
        };
    }
}

${{{{
for (int isScalarTexture = 0; isScalarTexture <= 1; isScalarTexture++) {
    if (isScalarTexture == 0)
    {
        sb << "__generic<T:__BuiltinArithmeticType, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let format:int>\n";
        sb << "extension __TextureImpl<T,Shape,isArray,0,sampleCount,0,isShadow,0,format>\n";
    }
    else
    {
        sb << "__generic<T:__BuiltinArithmeticType, let N:int, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let format:int>\n";
        sb << "extension __TextureImpl<vector<T,N>,Shape,isArray,0,sampleCount,0,isShadow,0,format>\n";
    }
}}}}
{ // begin extension for gather
${{{{
    // Gather component
    for (int isCmp = 0; isCmp <= 1; ++isCmp) {
        const char* cmp = isCmp ? "Cmp" : "";
        const char* cmpParam = isCmp? "T compareValue, " : "";
        const char* compareArg = isCmp ? "compareValue, " : "";
        const char* samplerStateType = isCmp ? "SamplerComparisonState" : "SamplerState";
        const char* componentNames[] = {"", "Red", "Green", "Blue", "Alpha"};
        for (auto componentId = 0;  componentId < 4; componentId++) {
            auto component = componentNames[componentId];
            auto componentIndex = componentId == 0 ? 0 : componentId - 1;
    }}}}
    [ForceInline]
    vector<T,4> Gather$(cmp)$(component)($(samplerStateType) s, vector<float, Shape.dimensions+isArray> location, $(cmpParam))
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Gather$(cmp)$(component)";
        case glsl:
        case spirv:
            return __glsl_gather$(cmp)<T>(this, s, location, $(componentIndex), $(compareArg));
        }
    }
    [ForceInline]
    vector<T,4> Gather$(cmp)$(component)($(samplerStateType) s, vector<float, Shape.dimensions+isArray> location, $(cmpParam) vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Gather$(cmp)$(component)";
        case glsl:
        case spirv:
            return __glsl_gather$(cmp)_offset<T>(this, s, location, $(componentIndex), $(compareArg) offset);
        }
    }
    [ForceInline]
    vector<T,4> Gather$(cmp)$(component)($(samplerStateType) s, vector<float, Shape.dimensions+isArray> location, $(cmpParam)
        vector<int, Shape.planeDimensions> offset1,
        vector<int, Shape.planeDimensions> offset2,
        vector<int, Shape.planeDimensions> offset3,
        vector<int, Shape.planeDimensions> offset4)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Gather$(cmp)$(component)";
        case glsl:
        case spirv:
            return __glsl_gather$(cmp)_offsets<T>(this, s, location, $(componentIndex), $(compareArg) offset1,offset2,offset3,offset4);
        }
    }
    ${{{{
    } // for (component)
    } // for (isCmp)
    }}}}
} // end extension for gather

${{{{
} // for (isScalarTexture)
}}}}

// Load/Subscript for readonly, no MS textures

__generic<T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let isCombined:int, let format:int>
extension __TextureImpl<T,Shape,isArray,0,sampleCount,0,isShadow,isCombined,format>
{
    static const int isMS = 0;
    static const int access = $(kStdlibResourceAccessReadOnly);

    __glsl_extension(GL_EXT_samplerless_texture_functions)
    [__readNone]
    T __glsl_load(vector<int, Shape.dimensions+isArray> location)
    {
        __intrinsic_asm "$ctexelFetch($0, ($1), 0)$z";
    }

    __glsl_extension(GL_EXT_samplerless_texture_functions)
    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray+1> location)
    {
        __target_switch
        {
        case cpp:
        case hlsl:
            __intrinsic_asm ".Load";
        case glsl:
            __intrinsic_asm "$ctexelFetch($0, ($1).$w1b, ($1).$w1e)$z";
        case spirv:
            const int lodLoc = Shape.dimensions+isArray;
            let coord = __vectorReshape<Shape.dimensions+isArray>(location);
            let lod = location[lodLoc];
            if (isCombined != 0)
            {
                return spirv_asm
                {
                    %image:__imageType(this) = OpImage $this;
                    %sampled:__sampledType(T) = OpImageFetch %image $coord Lod $lod;
                    __truncate $$T result __sampledType(T) %sampled;
                };
            }
            else
            {
                return spirv_asm
                {
                    %sampled:__sampledType(T) = OpImageFetch $this $coord Lod $lod;
                    __truncate $$T result __sampledType(T) %sampled;
                };
            }
        }
    }

    __glsl_extension(GL_EXT_samplerless_texture_functions)
    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray+1> location, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".Load";
            case glsl:
            __intrinsic_asm "$ctexelFetchOffset($0, ($1).$w1b, ($1).$w1e, ($2))$z";
            case spirv:
                const int lodLoc = Shape.dimensions+isArray;
                let coord = __vectorReshape<Shape.dimensions+isArray>(location);
                let lod = location[lodLoc];
                if (isCombined != 0)
                {
                    return spirv_asm
                    {
                        %image:__imageType(this) = OpImage $this;
                        %sampled:__sampledType(T) = OpImageFetch %image $coord Lod|ConstOffset $lod $offset;
                        __truncate $$T result __sampledType(T) %sampled;
                    };
                }
                else
                {
                    return spirv_asm
                    {
                        %sampled:__sampledType(T) = OpImageFetch $this $coord Lod|ConstOffset $lod $offset;
                        __truncate $$T result __sampledType(T) %sampled;
                    };
                }
        }
    }

    [__readNone]
    [ForceInline]
    __target_intrinsic(hlsl)
    T Load(vector<int, Shape.dimensions+isArray+1> location, constexpr vector<int, Shape.planeDimensions> offset, out uint status)
    {
        status = 0;
        return Load(location, offset);
    }

    __subscript(vector<uint, Shape.dimensions+isArray> location) -> T
    {
        __glsl_extension(GL_EXT_samplerless_texture_functions)
        [__readNone]
        [ForceInline]
        get
        {
            __target_switch
            {
                case cpp:
                case hlsl:
                    __intrinsic_asm ".operator[]";
                case glsl:
                    return __glsl_load(location);
                case spirv:
                    if (isCombined != 0)
                    {
                        return spirv_asm
                        {
                            %image:__imageType(this) = OpImage $this;
                            %sampled:__sampledType(T) = OpImageFetch %image $location;
                            __truncate $$T result __sampledType(T) %sampled;
                        };
                    }
                    else
                    {
                        return spirv_asm
                        {
                            %sampled:__sampledType(T) = OpImageFetch $this $location;
                            __truncate $$T result __sampledType(T) %sampled;
                        };
                    }
            }
        }
    }
}

// Texture Load/Subscript for readonly, MS textures

__generic<T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let isCombined:int, let format:int>
extension __TextureImpl<T,Shape,isArray,1,sampleCount,0,isShadow,isCombined,format>
{
    static const int access = $(kStdlibResourceAccessReadOnly);
    static const int isMS = 1;

    __glsl_extension(GL_EXT_samplerless_texture_functions)
    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, int sampleIndex)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".Load";
            case glsl:
                __intrinsic_asm "$ctexelFetch($0, $1, ($2))$z";
            case spirv:
                if (isCombined != 0)
                {
                    return spirv_asm
                    {
                        %image:__imageType(this) = OpImage $this;
                        %sampled:__sampledType(T) = OpImageFetch %image $location Sample $sampleIndex;
                        __truncate $$T result __sampledType(T) %sampled;
                    };
                }
                else
                {
                    return spirv_asm
                    {
                        %sampled:__sampledType(T) = OpImageFetch $this $location Sample $sampleIndex;
                        __truncate $$T result __sampledType(T) %sampled;
                    };
                }
        }
    }

    __glsl_extension(GL_EXT_samplerless_texture_functions)
    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, int sampleIndex, constexpr vector<int, Shape.planeDimensions> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
            __intrinsic_asm ".Load";
            case glsl:
            __intrinsic_asm "$ctexelFetchOffset($0, $1, ($2), ($3))$z";
            case spirv:
                if (isCombined != 0)
                {
                    return spirv_asm
                    {
                        %image:__imageType(this) = OpImage $this;
                        %sampled:__sampledType(T) = OpImageFetch %image $location ConstOffset|Sample $offset $sampleIndex;
                        __truncate $$T result __sampledType(T) %sampled;
                    };
                }
                else
                {
                    return spirv_asm
                    {
                        %sampled:__sampledType(T) = OpImageFetch $this $location ConstOffset|Sample $offset  $sampleIndex;
                        __truncate $$T result __sampledType(T) %sampled;
                    };
                }
        }
    }

    [__readNone]
    [ForceInline]
    __target_intrinsic(hlsl)
    T Load(vector<int, Shape.dimensions+isArray> location, int sampleIndex, constexpr vector<int, Shape.planeDimensions> offset, out uint status)
    {
        status = 0;
        return Load(location, sampleIndex, offset);
    }

    __subscript(vector<uint, Shape.dimensions+isArray> location, int sampleIndex) -> T
    {
        __glsl_extension(GL_EXT_samplerless_texture_functions)
        [__readNone]
        [ForceInline]
        get
        {
            __target_switch
            {
                case cpp:
                case hlsl:
                    __intrinsic_asm "($0).sample[$2][$1]";
                case glsl:
                case spirv:
                case cuda:
                    return Load(location, sampleIndex);
            }
        }
    }
}

// Load/Subscript for readwrite textures
${{{{
    for (int access = kStdlibResourceAccessReadWrite; access<=kStdlibResourceAccessRasterizerOrdered; access++) {
        const char* glslIntrinsic = "$cimageLoad($0, $1)$z";
        const char* glslIntrinsicOffset = "$cimageLoad($0, ($1)+($2))$z";
        const char* glslIntrinsicMS = "$cimageLoad($0, $1, $2)$z";
        const char* glslIntrinsicMSOffset = "$cimageLoad($0, ($1)+($3), $2)$z";
}}}}
__generic<T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let format:int>
extension __TextureImpl<T,Shape,isArray,0,sampleCount,$(access),isShadow, 0,format>
{
    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Load";
            case glsl:
                __intrinsic_asm "$(glslIntrinsic)";
            case cuda:
                if (isArray != 0)
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "surf1DLayeredread$C<$T0>($0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "surf2DLayeredread$C<$T0>($0, ($1).x * $E, ($1).y, ($1).z, SLANG_CUDA_BOUNDARY_MODE)";
                    case $(SLANG_TEXTURE_3D):
                        __intrinsic_asm "surf3DLayeredread$C<$T0>($0, ($1).x * $E, ($1).y, ($1).z, ($1).w, SLANG_CUDA_BOUNDARY_MODE)";
                    default:
                        __intrinsic_asm "<invalid intrinsic>";
                    }
                }
                else
                {
                    switch(Shape.flavor)
                    {
                    case $(SLANG_TEXTURE_1D):
                        __intrinsic_asm "surf1Dread$C<$T0>($0, ($1) * $E, SLANG_CUDA_BOUNDARY_MODE)";
                    case $(SLANG_TEXTURE_2D):
                        __intrinsic_asm "surf2Dread$C<$T0>($0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
                    case $(SLANG_TEXTURE_3D):
                        __intrinsic_asm "surf3Dread$C<$T0>($0, ($1).x * $E, ($1).y, ($1).z, SLANG_CUDA_BOUNDARY_MODE)";
                    default:
                        __intrinsic_asm "<invalid intrinsic>";
                    }
                }
            case spirv:
                return spirv_asm
                {
                    %sampled:__sampledType(T) = OpImageRead $this $location;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, vector<int, Shape.dimensions+isArray> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Load";
            case glsl:
                __intrinsic_asm "$(glslIntrinsicOffset)";
            case spirv:
                return spirv_asm
                {
                    %sampled:__sampledType(T) = OpImageRead $this $location ConstOffset $offset;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, vector<int, Shape.dimensions+isArray> offset, out uint status)
    {
        __target_switch
        {
        case hlsl:
        case cpp:
            __intrinsic_asm ".Load";
        default:
            status = 0;
            return Load(location, offset);
        }
    }

    void __glslImageStore(vector<int, Shape.dimensions+isArray> location, T value)
    {
        __intrinsic_asm "imageStore($0, $1, $V2)";
    }

    __subscript(vector<uint, Shape.dimensions+isArray> location) -> T
    {
        [__readNone]
        [ForceInline]
        get
        {
            __target_switch
            {
                case cpp:
                case hlsl:
                    __intrinsic_asm ".operator[]";
                case glsl:
                case spirv:
                case cuda:
                    return Load(location);
            }
        }

        [nonmutating]
        [ForceInline]
        set(T newValue)
        {
            __target_switch
            {
                case cpp:
                case hlsl:
                __intrinsic_asm ".operator[]";
                case glsl:
                    __glslImageStore(location, newValue);
                case cuda:
                    if (isArray != 0)
                    {
                        switch(Shape.flavor)
                        {
                        case $(SLANG_TEXTURE_1D):
                            __intrinsic_asm "surf1DLayeredwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
                        case $(SLANG_TEXTURE_2D):
                            __intrinsic_asm "surf2DLayeredwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y, ($1).z, SLANG_CUDA_BOUNDARY_MODE)";
                        case $(SLANG_TEXTURE_3D):
                            __intrinsic_asm "surf3DLayeredwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y, ($1).z, ($1).w, SLANG_CUDA_BOUNDARY_MODE)";
                        default:
                            __intrinsic_asm "<invalid intrinsic>";
                        }
                    }
                    else
                    {
                        switch(Shape.flavor)
                        {
                        case $(SLANG_TEXTURE_1D):
                            __intrinsic_asm "surf1Dwrite$C<$T0>($2, $0, ($1) * $E, SLANG_CUDA_BOUNDARY_MODE)";
                        case $(SLANG_TEXTURE_2D):
                            __intrinsic_asm "surf2Dwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
                        case $(SLANG_TEXTURE_3D):
                            __intrinsic_asm "surf3Dwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y, ($1).z, SLANG_CUDA_BOUNDARY_MODE)";
                        default:
                            __intrinsic_asm "<invalid intrinsic>";
                        }
                    }
                case spirv:
                    return spirv_asm
                    {
                        OpImageWrite $this $location $newValue;
                    };
            }
        }

        __intrinsic_op($(kIROp_ImageSubscript)) ref;
    }
}

${{{{
if (access == kStdlibResourceAccessReadWrite) {
}}}}
// RW MS textures.
__generic<T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let format:int>
extension __TextureImpl<T,Shape,isArray,1,sampleCount,$(access),isShadow, 0,format>
{
    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, int sampleIndex)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Load";
            case glsl:
                __intrinsic_asm "$(glslIntrinsicMS)";
            case spirv:
                return spirv_asm
                {
                    %sampled:__sampledType(T) = OpImageRead $this $location Sample $sampleIndex;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, int sampleIndex, vector<int, Shape.dimensions+isArray> offset)
    {
        __target_switch
        {
            case cpp:
            case hlsl:
                __intrinsic_asm ".Load";
            case glsl:
                __intrinsic_asm "$(glslIntrinsicMSOffset)";
            case spirv:
                return spirv_asm
                {
                    %sampled:__sampledType(T) = OpImageRead $this $location ConstOffset|Sample $offset $sampleIndex;
                    __truncate $$T result __sampledType(T) %sampled;
                };
        }
    }

    [__readNone]
    [ForceInline]
    T Load(vector<int, Shape.dimensions+isArray> location, int sampleIndex, vector<int, Shape.dimensions+isArray> offset, out uint status)
    {
        __target_switch
        {
        case hlsl:
        case cpp:
            __intrinsic_asm ".Load";
        default:
            status = 0;
            return Load(location, sampleIndex, offset);
        }
    }

    void __glslImageStore(vector<int, Shape.dimensions+isArray> location, int sampleIndex, T value)
    {
        __intrinsic_asm "imageStore($0, $1, $2, $V3)";
    }

    __subscript(vector<uint, Shape.dimensions+isArray> location, int sampleIndex) -> T
    {
        [__readNone]
        [ForceInline]
        get
        {
            __target_switch
            {
                case cpp:
                case hlsl:
                    __intrinsic_asm "$0.sample[$2][$1]";
                case glsl:
                case spirv:
                case cuda:
                    return Load(location, sampleIndex);
            }
        }

        [nonmutating]
        [ForceInline]
        set(T newValue)
        {
            __target_switch
            {
                case cpp:
                case hlsl:
                __intrinsic_asm "$0.sample[$2][$1]";
                case glsl:
                    __glslImageStore(location, sampleIndex, newValue);
                case spirv:
                    return spirv_asm
                    {
                        OpImageWrite $this $location $newValue Sample $sampleIndex;
                    };
            }
        }

        __intrinsic_op($(kIROp_ImageSubscript)) ref;
    }
}

${{{{
} // if (access == kStdlibResourceAccessReadWrite) // for RW MS textures.
} // for (access).
}}}}

// Texture type aliases.
// T, Shape: __ITextureShape, let isArray:int, let isMS:int, let sampleCount:int, let access:int, let isShadow:int, let isCombined:int, let format:int
${{{{
    const char* shapeTypeNames[] = {"1D", "2D", "3D", "Cube"};
    const char* accessPrefix[] = {"", "RW", "RasterizerOrdered", "Feedback"};
    const char* arrayPostFix[] = {"", "Array"};
    const char* msPostFix[] = {"", "MS"};
    for (int shape = 0; shape < 4; shape++)
    for (int isArray = 0; isArray<=1; isArray++)
    for (int isMS = 0; isMS<=1; isMS++)
    for (int isCombined = 0; isCombined<=1; isCombined++)
    for (int access = kStdlibResourceAccessReadOnly; access<=kStdlibResourceAccessFeedback; access++) {
        if (access != kStdlibResourceAccessReadOnly)
        {
            // No RW Cube.
            if (shape == kStdlibShapeIndexCube) continue;
        }
        if (access == kStdlibResourceAccessFeedback)
        {
            // Feedback only defined for Texture2D and Texture2DArray.
            if (shape != 1) continue;
            if (isMS) continue;
            if (isCombined) continue;
        }
        if (isMS)
        {
            // Only Texture2DMS.
            if (shape != kStdlibShapeIndex2D)
                continue;
            // Only Texture2DMS or RWTexture2DMS.
            if (access >= kStdlibShapeIndex3D)
                continue;
        }
        // No 3D Array.
        if (shape == kStdlibShapeIndex3D && isArray == 1)
            continue;
        const char* textureTypeName = isCombined ? "Sampler" : "Texture";
}}}}
typealias $(accessPrefix[access])$(textureTypeName)$(shapeTypeNames[shape])$(arrayPostFix[isArray])$(msPostFix[isMS])<T=float4, let sampleCount:int=0, let format:int=0> = __TextureImpl<T, __Shape$(shapeTypeNames[shape]), $(isArray), $(isMS), sampleCount, $(access), 0, $(isCombined), format>;
${{{{
}
}}}}

// AtomicAdd

// Make the GLSL atomicAdd available.
// We have separate int/float implementations, as the float version requires some specific extensions
// https://www.khronos.org/registry/OpenGL/extensions/NV/NV_shader_atomic_float.txt

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_float)
float __atomicAdd(__ref float value, float amount)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpExtension "SPV_EXT_shader_atomic_float_add";
            OpCapability AtomicFloat32AddEXT;
            result:$$float = OpAtomicFAddEXT &value Device None $amount
        };
    }
}

// Helper for hlsl, using NVAPI
__target_intrinsic(hlsl, "NvInterlockedAddUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicAdd(RWByteAddressBuffer buf, uint offset, uint2);

// atomic add for hlsl using SM6.6
__target_intrinsic(hlsl, "$0.InterlockedAdd64($1, $2, $3)")
void __atomicAdd(RWByteAddressBuffer buf, uint offset, int64_t value, out int64_t originalValue);
__target_intrinsic(hlsl, "$0.InterlockedAdd64($1, $2, $3)")
void __atomicAdd(RWByteAddressBuffer buf, uint offset, uint64_t value, out uint64_t originalValue);

// Int versions require glsl 4.30
// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/atomicAdd.xhtml

__glsl_version(430)
int __atomicAdd(__ref int value, int amount)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
        return spirv_asm
        {
            result:$$int = OpAtomicIAdd &value Device None $amount;
        };
    }
}

__glsl_version(430)
uint __atomicAdd(__ref uint value, uint amount)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
        return spirv_asm
        {
            result:$$uint = OpAtomicIAdd &value Device None $amount;
        };
    }
}

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
int64_t __atomicAdd(__ref int64_t value, int64_t amount)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$int64_t = OpAtomicIAdd &value Device None $amount
        };
    }
}

__target_intrinsic(glsl, "atomicAdd($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicAdd(__ref uint64_t value, uint64_t amount)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicIAdd &value Device None $amount
        };
    }
}

// Cas - Compare and swap

// Helper for HLSL, using NVAPI

__target_intrinsic(hlsl, "NvInterlockedCompareExchangeUint64($0, $1, $2, $3)")
[__requiresNVAPI]
uint2 __cas(RWByteAddressBuffer buf, uint offset, uint2 compareValue, uint2 value);

// CAS using SM6.6
__target_intrinsic(hlsl, "$0.InterlockedCompareExchange64($1, $2, $3, $4)")
void __cas(RWByteAddressBuffer buf, uint offset, in int64_t compare_value, in int64_t value, out int64_t original_value);
__target_intrinsic(hlsl, "$0.InterlockedCompareExchange64($1, $2, $3, $4)")
void __cas(RWByteAddressBuffer buf, uint offset, in uint64_t compare_value, in uint64_t value, out uint64_t original_value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
int64_t __cas(__ref int64_t ioValue, int64_t compareValue, int64_t newValue)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicCompSwap($0, $1, $2)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$int64_t = OpAtomicCompareExchange &ioValue Device None None $newValue $compareValue
        };
    }
}

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __cas(__ref uint64_t ioValue, uint64_t compareValue, uint64_t newValue)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicCompSwap($0, $1, $2)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicCompareExchange &ioValue Device None None $newValue $compareValue
        };
    }
}

// Max

__target_intrinsic(hlsl, "NvInterlockedMaxUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicMax(RWByteAddressBuffer buf, uint offset, uint2 value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicMax(__ref uint64_t ioValue, uint64_t value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicMax($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicUMax &ioValue Device None $value
        };
    }
}

// Min

__target_intrinsic(hlsl, "NvInterlockedMinUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicMin(RWByteAddressBuffer buf, uint offset, uint2 value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicMin(__ref uint64_t ioValue, uint64_t value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicMin($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicUMin &ioValue Device None $value
        };
    }
}

// And

__target_intrinsic(hlsl, "NvInterlockedAndUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicAnd(RWByteAddressBuffer buf, uint offset, uint2 value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicAnd(__ref uint64_t ioValue, uint64_t value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAnd($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicAnd &ioValue Device None $value
        };
    }
}

// Or

__target_intrinsic(hlsl, "NvInterlockedOrUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicOr(RWByteAddressBuffer buf, uint offset, uint2 value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicOr(__ref uint64_t ioValue, uint64_t value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicOr($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicOr &ioValue Device None $value
        };
    }
}

// Xor

__target_intrinsic(hlsl, "NvInterlockedXorUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicXor(RWByteAddressBuffer buf, uint offset, uint2 value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicXor(__ref uint64_t ioValue, uint64_t value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicXor($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicXor &ioValue Device None $value
        };
    }
}

// Exchange

__target_intrinsic(hlsl, "NvInterlockedExchangeUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicExchange(RWByteAddressBuffer buf, uint offset, uint2 value);

__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicExchange(__ref uint64_t ioValue, uint64_t value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicExchange($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Int64Atomics;
            result:$$uint64_t = OpAtomicExchange &ioValue Device None $value
        };
    }
}

// Conversion between uint64_t and uint2

uint2 __asuint2(uint64_t i)
{
    return uint2(uint(i), uint(uint64_t(i) >> 32));
}

uint64_t __asuint64(uint2 i)
{
    return (uint64_t(i.y) << 32) | i.x;
}

//

__intrinsic_op($(kIROp_ByteAddressBufferLoad))
T __byteAddressBufferLoad<T>(ByteAddressBuffer buffer, int offset);

__intrinsic_op($(kIROp_ByteAddressBufferLoad))
T __byteAddressBufferLoad<T>(RWByteAddressBuffer buffer, int offset);

__intrinsic_op($(kIROp_ByteAddressBufferLoad))
T __byteAddressBufferLoad<T>(RasterizerOrderedByteAddressBuffer buffer, int offset);

__intrinsic_op($(kIROp_ByteAddressBufferStore))
void __byteAddressBufferStore<T>(RWByteAddressBuffer buffer, int offset, T value);

__intrinsic_op($(kIROp_ByteAddressBufferStore))
void __byteAddressBufferStore<T>(RasterizerOrderedByteAddressBuffer buffer, int offset, T value);

__generic<T>
__magic_type(HLSLStructuredBufferType)
__intrinsic_type($(kIROp_HLSLStructuredBufferType))
struct StructuredBuffer
{
    [__readNone]
    [__unsafeForceInlineEarly]
    void GetDimensions(
        out uint numStructs,
        out uint stride)
    {
        let rs = __structuredBufferGetDimensions(this);
        numStructs = rs.x;
        stride = rs.y;
    }

    __intrinsic_op($(kIROp_StructuredBufferLoad))
    __target_intrinsic(glsl, "$0._data[$1]")
    __target_intrinsic(spirv, "%addr = OpAccessChain resultType*StorageBuffer resultId _0 const(int, 0) _1; OpLoad resultType resultId %addr;")
    [__readNone]
    T Load(int location);

    __intrinsic_op($(kIROp_StructuredBufferLoadStatus))
    T Load(int location, out uint status);

    __subscript(uint index) -> T
    {
        [__readNone]
        __intrinsic_op($(kIROp_StructuredBufferLoad))
        get;
    };
};

__generic<T>
__magic_type(HLSLConsumeStructuredBufferType)
__intrinsic_type($(kIROp_HLSLConsumeStructuredBufferType))
struct ConsumeStructuredBuffer
{
    __intrinsic_op($(kIROp_StructuredBufferConsume))
    T Consume();

    [ForceInline]
    void GetDimensions(
        out uint numStructs,
        out uint stride)
    {
        let result = __structuredBufferGetDimensions(this);
        numStructs = result.x;
        stride = result.y;
    }
};

__generic<T, let N : int>
__magic_type(HLSLInputPatchType)
__intrinsic_type($(kIROp_HLSLInputPatchType))
struct InputPatch
{
    __subscript(uint index) -> T;
};

__generic<T, let N : int>
__magic_type(HLSLOutputPatchType)
__intrinsic_type($(kIROp_HLSLOutputPatchType))
struct OutputPatch
{
    __subscript(uint index) -> T;
};

${{{{
static const struct {
    IROp op;
    char const* name;
} kMutableByteAddressBufferCases[] =
{
    { kIROp_HLSLRWByteAddressBufferType,                "RWByteAddressBuffer" },
    { kIROp_HLSLRasterizerOrderedByteAddressBufferType, "RasterizerOrderedByteAddressBuffer" },
};
for(auto item : kMutableByteAddressBufferCases) {
}}}}

__magic_type(HLSL$(item.name)Type)
__intrinsic_type($(item.op))
struct $(item.name)
{
    // Note(tfoley): supports all operations from `ByteAddressBuffer`
    // TODO(tfoley): can this be made a sub-type?

    __target_intrinsic(hlsl)
    __target_intrinsic(cpp)
    __target_intrinsic(cuda)
    [__unsafeForceInlineEarly]
    void GetDimensions(out uint dim);

    [__unsafeForceInlineEarly]
    __specialized_for_target(spirv)
    __specialized_for_target(glsl)
    void GetDimensions(out uint dim)
    {
        dim = __structuredBufferGetDimensions(__getEquivalentStructuredBuffer<uint>(this)).x*4;
    }

    __target_intrinsic(hlsl)
    [__NoSideEffect]
    uint Load(int location)
    {
        return __byteAddressBufferLoad<uint>(this, location);
    }

    [__NoSideEffect]
    uint Load(int location, out uint status);

    __target_intrinsic(hlsl)
    [__NoSideEffect]
    uint2 Load2(int location)
    {
        return __byteAddressBufferLoad<uint2>(this, location);
    }

    [__NoSideEffect]
    uint2 Load2(int location, out uint status);

    __target_intrinsic(hlsl)
    [__NoSideEffect]
    uint3 Load3(int location)
    {
        return __byteAddressBufferLoad<uint3>(this, location);
    }

    [__NoSideEffect]
    uint3 Load3(int location, out uint status);

    __target_intrinsic(hlsl)
    [__NoSideEffect]
    uint4 Load4(int location)
    {
        return __byteAddressBufferLoad<uint4>(this, location);
    }

    [__NoSideEffect]
    uint4 Load4(int location, out uint status);

    [__NoSideEffect]
    T Load<T>(int location)
    {
        return __byteAddressBufferLoad<T>(this, location);
    }
${{{{
    if (item.op == kIROp_HLSLRWByteAddressBufferType)
    {
}}}}

    // float32 and int64 atomic support. This is a Slang specific extension, it uses
    // GL_EXT_shader_atomic_float on Vulkan
    // NvAPI support on DX
    // NOTE! To use this feature on HLSL based targets the path to 'nvHLSLExtns.h' from the NvAPI SDK must
    // be set. That this include will be added to the *output* that is passed to a downstram compiler.
    // Also note that you *can* include NVAPI headers in your Slang source, and directly use NVAPI functions
    // Directly using NVAPI functions does *not* add the #include on the output
    // Finally note you can *mix* NVAPI direct calls, and use of NVAPI intrinsics below. This doesn't cause
    // any clashes, as Slang will emit any NVAPI function it parsed (say via a include in Slang source) with
    // unique functions.
    //
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VK_EXT_shader_atomic_float
    // https://htmlpreview.github.io/?https://github.com/KhronosGroup/SPIRV-Registry/blob/master/extensions/EXT/SPV_EXT_shader_atomic_float_add.html

    // F32 Add

    __cuda_sm_version(2.0)
    [__requiresNVAPI]
    void InterlockedAddF32(uint byteAddress, float valueToAdd, out float originalValue)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm "($3 = NvInterlockedAddFp32($0, $1, $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicAdd($0._getPtrAt<float>($1), $2))";
        case glsl:
        case spirv:
            {
                let buf = __getEquivalentStructuredBuffer<float>(this);
                originalValue = __atomicAdd(buf[byteAddress / 4], valueToAdd);
                return;
            }
        }
    }

    // Without returning original value

    [__requiresNVAPI]
    __cuda_sm_version(2.0)
    void InterlockedAddF32(uint byteAddress, float valueToAdd)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm "(NvInterlockedAddFp32($0, $1, $2))";
        case cuda: __intrinsic_asm "atomicAdd($0._getPtrAt<float>($1), $2)";
        case glsl:
        case spirv:
            {
                let buf = __getEquivalentStructuredBuffer<float>(this);
                __atomicAdd(buf[byteAddress / 4], valueToAdd);
                return;
            }
        }
    }

    // Int64 Add
    __cuda_sm_version(6.0)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd, out int64_t originalValue)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "(*$3 = atomicAdd($0._getPtrAt<uint64_t>($1), $2))";
        case hlsl:
            originalValue = __asuint64(__atomicAdd(this, byteAddress, __asuint2(valueToAdd)));
        case glsl:
        case spirv:
            {
                let buf = __getEquivalentStructuredBuffer<int64_t>(this);
                originalValue = __atomicAdd(buf[byteAddress / 8], valueToAdd);
            }
        }
    }

    // Without returning original value
    __cuda_sm_version(6.0)
    __target_intrinsic(cuda, "atomicAdd($0._getPtrAt<uint64_t>($1), $2)")
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd);

    __specialized_for_target(hlsl)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd)
    {
        __atomicAdd(this, byteAddress, __asuint2(valueToAdd));
    }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd)
    {
        let buf = __getEquivalentStructuredBuffer<int64_t>(this);
        __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }

    // Cas uint64_t

    __target_intrinsic(cuda, "(*$4 = atomicCAS($0._getPtrAt<uint64_t>($1), $2, $3))")
    void InterlockedCompareExchangeU64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue);

    __specialized_for_target(hlsl)
    void InterlockedCompareExchangeU64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        outOriginalValue = __asuint64(__cas(this, byteAddress, __asuint2(compareValue), __asuint2(value)));
    }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    void InterlockedCompareExchangeU64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        outOriginalValue = __cas(buf[byteAddress / 8], compareValue, value);
    }

    // Max

    __cuda_sm_version(3.5)
    __target_intrinsic(cuda, "atomicMax($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedMaxU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedMaxU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicMax(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    uint64_t InterlockedMaxU64(uint byteAddress, uint64_t value)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicMax(buf[byteAddress / 8], value);
    }

    // Min

    __cuda_sm_version(3.5)
    __target_intrinsic(cuda, "atomicMin($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedMinU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedMinU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicMin(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    uint64_t InterlockedMinU64(uint byteAddress, uint64_t value)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicMin(buf[byteAddress / 8], value);
    }

    // And

    __target_intrinsic(cuda, "atomicAnd($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedAndU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedAndU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicAnd(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    uint64_t InterlockedAndU64(uint byteAddress, uint64_t value)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicAnd(buf[byteAddress / 8], value);
    }

    // Or

    __target_intrinsic(cuda, "atomicOr($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedOrU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedOrU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicOr(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    uint64_t InterlockedOrU64(uint byteAddress, uint64_t value)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicOr(buf[byteAddress / 8], value);
    }

    // Xor

    __target_intrinsic(cuda, "atomicXor($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedXorU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedXorU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicXor(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    uint64_t InterlockedXorU64(uint byteAddress, uint64_t value)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicXor(buf[byteAddress / 8], value);
    }

    // Exchange

    __target_intrinsic(cuda, "atomicExch($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedExchangeU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedExchangeU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicExchange(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    uint64_t InterlockedExchangeU64(uint byteAddress, uint64_t value)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicExchange(buf[byteAddress / 8], value);
    }

    // SM6.6 6 64bit atomics.
    __specialized_for_target(hlsl)
    void InterlockedAdd64(uint byteAddress, int64_t valueToAdd, out int64_t outOriginalValue)
    {
        __atomicAdd(this, byteAddress, valueToAdd, outOriginalValue);
    }
    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    void InterlockedAdd64(uint byteAddress, int64_t valueToAdd, out int64_t originalValue)
    {
        let buf = __getEquivalentStructuredBuffer<int64_t>(this);
        originalValue = __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }
    __specialized_for_target(hlsl)
    void InterlockedAdd64(uint byteAddress, uint64_t valueToAdd, out uint64_t outOriginalValue)
    {
        __atomicAdd(this, byteAddress, valueToAdd, outOriginalValue);
    }
    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    void InterlockedAdd64(uint byteAddress, uint64_t valueToAdd, out uint64_t originalValue)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        originalValue = __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }
    __specialized_for_target(hlsl)
    void InterlockedCompareExchange64(uint byteAddress, int64_t compareValue, int64_t value, out int64_t outOriginalValue)
    {
        __cas(this, byteAddress, compareValue, value, outOriginalValue);
    }
    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    void InterlockedCompareExchange64(uint byteAddress, int64_t compareValue, int64_t value, out int64_t outOriginalValue)
    {
        let buf = __getEquivalentStructuredBuffer<int64_t>(this);
        outOriginalValue = __cas(buf[byteAddress / 8], compareValue, value);
    }
    __specialized_for_target(hlsl)
    void InterlockedCompareExchange64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        __cas(this, byteAddress, compareValue, value, outOriginalValue);
    }
    __specialized_for_target(glsl)
    __specialized_for_target(spirv)
    void InterlockedCompareExchange64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        let buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        outOriginalValue = __cas(buf[byteAddress / 8], compareValue, value);
    }
${{{{
    } // endif (type == RWByteAddressBuffer)
}}}}

    // Added operations:
    void InterlockedAdd(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($3 = atomicAdd($0._data[$1/4], $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicAdd($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedAdd";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedAdd(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedAdd(
        UINT dest,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicAdd($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "atomicAdd($0._getPtrAt<uint32_t>($1), $2)";
        case hlsl: __intrinsic_asm ".InterlockedAdd";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedAdd(buf[dest / 4], value);
        }
    }

    void InterlockedAnd(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "$3 = atomicAnd($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "(*$3 = atomicAnd($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedAnd";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedAnd(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedAnd(
        UINT dest,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicAnd($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "atomicAnd($0._getPtrAt<uint32_t>($1), $2)";
        case hlsl: __intrinsic_asm ".InterlockedAnd";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedAnd(buf[dest / 4], value);
        }
    }

    void InterlockedCompareExchange(
        UINT dest,
        UINT compare_value,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($4 = atomicCompSwap($0._data[$1/4], $2, $3))";
        case cuda: __intrinsic_asm "(*$4 = atomicCAS($0._getPtrAt<uint32_t>($1), $2, $3))";
        case hlsl: __intrinsic_asm ".InterlockedCompareExchange";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedCompareExchange(buf[dest / 4], compare_value, value, original_value);
        }
    }

    void InterlockedCompareStore(
        UINT dest,
        UINT compare_value,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicCompSwap($0._data[$1/4], $2, $3)";
        case cuda: __intrinsic_asm "atomicCAS($0._getPtrAt<uint32_t>($1), $2, $3)";
        case hlsl: __intrinsic_asm ".InterlockedCompareStore";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedCompareStore(buf[dest / 4], compare_value, value);
        }
    }

    void InterlockedExchange(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($3 = atomicExchange($0._data[$1/4], $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicExch($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedExchange";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedExchange(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedMax(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($3 = atomicMax($0._data[$1/4], $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicMax($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedMax";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedMax(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedMax(
        UINT dest,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicMax($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "atomicMax($0._getPtrAt<uint32_t>($1), $2)";
        case hlsl: __intrinsic_asm ".InterlockedMax";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedMax(buf[dest / 4], value);
        }
    }

    void InterlockedMin(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($3 = atomicMin($0._data[$1/4], $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicMin($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedMin";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedMin(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedMin(
        UINT dest,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicMin($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "atomicMin($0._getPtrAt<uint32_t>($1), $2)";
        case hlsl: __intrinsic_asm ".InterlockedMin";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedMin(buf[dest / 4], value);
        }
    }

    void InterlockedOr(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($3 = atomicOr($0._data[$1/4], $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicOr($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedOr";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedOr(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedOr(
        UINT dest,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicOr($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "atomicOr($0._getPtrAt<uint32_t>($1), $2)";
        case hlsl: __intrinsic_asm ".InterlockedOr";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedOr(buf[dest / 4], value);
        }
    }

    void InterlockedXor(
        UINT dest,
        UINT value,
        out UINT original_value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "($3 = atomicXor($0._data[$1/4], $2))";
        case cuda: __intrinsic_asm "(*$3 = atomicXor($0._getPtrAt<uint32_t>($1), $2))";
        case hlsl: __intrinsic_asm ".InterlockedXor";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedXor(buf[dest / 4], value, original_value);
        }
    }

    void InterlockedXor(
        UINT dest,
        UINT value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "atomicXor($0._data[$1/4], $2)";
        case cuda: __intrinsic_asm "atomicXor($0._getPtrAt<uint32_t>($1), $2)";
        case hlsl: __intrinsic_asm ".InterlockedXor";
        case spirv:
            let buf = __getEquivalentStructuredBuffer<uint>(this);
            ::InterlockedXor(buf[dest / 4], value);
        }
    }

    __target_intrinsic(hlsl)
    [ForceInline]
    void Store(
        uint address,
        uint value)
    {
        __byteAddressBufferStore(this, address, value);
    }

    __target_intrinsic(hlsl)
    [ForceInline]
    void Store2(uint address, uint2 value)
    {
        __byteAddressBufferStore(this, address, value);
    }

    __target_intrinsic(hlsl)
    [ForceInline]
    void Store3(
        uint address,
        uint3 value)
    {
        __byteAddressBufferStore(this, address, value);
    }

    __target_intrinsic(hlsl)
    [ForceInline]
    void Store4(
        uint address,
        uint4 value)
    {
        __byteAddressBufferStore(this, address, value);
    }

    void Store<T>(int offset, T value)
    {
        __byteAddressBufferStore(this, offset, value);
    }
};

${{{{
}
}}}}

${{{{
static const struct {
    IROp op;
    char const* name;
} kMutableStructuredBufferCases[] =
{
    { kIROp_HLSLRWStructuredBufferType,                "RWStructuredBuffer" },
    { kIROp_HLSLRasterizerOrderedStructuredBufferType, "RasterizerOrderedStructuredBuffer" },
};
for(auto item : kMutableStructuredBufferCases) {
}}}}

__generic<T>
__magic_type(HLSL$(item.name)Type)
__intrinsic_type($(item.op))
struct $(item.name)
{
    uint DecrementCounter();

    [__readNone]
    [__unsafeForceInlineEarly]
    __target_intrinsic(hlsl)
    void GetDimensions(
        out uint numStructs,
        out uint stride)
    {
        let rs = __structuredBufferGetDimensions(this);
        numStructs = rs.x;
        stride = rs.y;
    }

    uint IncrementCounter();

    [__NoSideEffect]
    __intrinsic_op($(kIROp_RWStructuredBufferLoad))
    T Load(int location);

    [__NoSideEffect]
    __intrinsic_op($(kIROp_RWStructuredBufferLoadStatus))
    T Load(int location, out uint status);

    __subscript(uint index) -> T
    {
        [__NoSideEffect]
        __intrinsic_op($(kIROp_RWStructuredBufferGetElementPtr))
        ref;
    }
};

${{{{
}
}}}}

__generic<T>
__magic_type(HLSLPointStreamType)
__intrinsic_type($(kIROp_HLSLPointStreamType))
struct PointStream
{
    [KnownBuiltin("GeometryStreamAppend")]
    void Append(T value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "EmitVertex()";
        case hlsl: __intrinsic_asm ".Append";
        case spirv: spirv_asm { OpEmitVertex; };
        }
    }

    [KnownBuiltin("GeometryStreamRestart")]
    void RestartStrip()
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "EndPrimitive()";
        case hlsl: __intrinsic_asm ".RestartStrip";
        case spirv: spirv_asm { OpEndPrimitive; };
        }
    }
};

__generic<T>
__magic_type(HLSLLineStreamType)
__intrinsic_type($(kIROp_HLSLLineStreamType))
struct LineStream
{
    [KnownBuiltin("GeometryStreamAppend")]
    void Append(T value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "EmitVertex()";
        case hlsl: __intrinsic_asm ".Append";
        case spirv: spirv_asm { OpEmitVertex; };
        }
    }

    [KnownBuiltin("GeometryStreamRestart")]
    void RestartStrip()
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "EndPrimitive()";
        case hlsl: __intrinsic_asm ".RestartStrip";
        case spirv: spirv_asm { OpEndPrimitive; };
        }
    }
};

__generic<T>
__magic_type(HLSLTriangleStreamType)
__intrinsic_type($(kIROp_HLSLTriangleStreamType))
struct TriangleStream
{
    [KnownBuiltin("GeometryStreamAppend")]
    void Append(T value)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "EmitVertex()";
        case hlsl: __intrinsic_asm ".Append";
        case spirv: spirv_asm { OpEmitVertex; };
        }
    }

    [KnownBuiltin("GeometryStreamRestart")]
    void RestartStrip()
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "EndPrimitive()";
        case hlsl: __intrinsic_asm ".RestartStrip";
        case spirv: spirv_asm { OpEndPrimitive; };
        }
    }
};

#define VECTOR_MAP_UNARY(TYPE, COUNT, FUNC, VALUE) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(VALUE[i]); } return result

#define MATRIX_MAP_UNARY(TYPE, ROWS, COLS, FUNC, VALUE) \
    matrix<TYPE,ROWS,COLS> result; for(int i = 0; i < ROWS; ++i) { result[i] = FUNC(VALUE[i]); } return result

#define VECTOR_MAP_BINARY(TYPE, COUNT, FUNC, LEFT, RIGHT) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(LEFT[i], RIGHT[i]); } return result

#define MATRIX_MAP_BINARY(TYPE, ROWS, COLS, FUNC, LEFT, RIGHT) \
    matrix<TYPE,ROWS,COLS> result; for(int i = 0; i < ROWS; ++i) { result[i] = FUNC(LEFT[i], RIGHT[i]); } return result

#define VECTOR_MAP_TRINARY(TYPE, COUNT, FUNC, A, B, C) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(A[i], B[i], C[i]); } return result

#define MATRIX_MAP_TRINARY(TYPE, ROWS, COLS, FUNC, A, B, C) \
    matrix<TYPE,ROWS,COLS> result; for(int i = 0; i < ROWS; ++i) { result[i] = FUNC(A[i], B[i], C[i]); } return result

// Try to terminate the current draw or dispatch call (HLSL SM 4.0)
void abort();

// Absolute value (HLSL SM 1.0)

__generic<T : __BuiltinIntegerType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_abs($0)")
__target_intrinsic(cpp, "$P_abs($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fi(FAbs, SAbs) _0")
[__readNone]
T abs(T x);
/*{
    // Note: this simple definition may not be appropriate for floating-point inputs
    return x < 0 ? -x : x;
}*/

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fi(FAbs, SAbs) _0")
[__readNone]
vector<T, N> abs(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, abs, x);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> abs(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, abs, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_abs($0)")
__target_intrinsic(cpp, "$P_abs($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fi(FAbs, SAbs) _0")
[__readNone]
T abs(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fi(FAbs, SAbs) _0")
[__readNone]
vector<T, N> abs(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, abs, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> abs(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, abs, x);
}

// Inverse cosine (HLSL SM 1.0)

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_acos($0)")
__target_intrinsic(cpp, "$P_acos($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Acos _0")
[__readNone]
T acos(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Acos _0")
[__readNone]
vector<T, N> acos(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, acos, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> acos(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, acos, x);
}

// Test if all components are non-zero (HLSL SM 1.0)
__generic<T : __BuiltinType>
[__readNone]
bool all(T x)
{
    __target_switch
    {
    default:
        __intrinsic_asm "bool($0)";
    case hlsl:
        __intrinsic_asm "all";
    case spirv:
        let zero = __default<T>();
        if (__isInt<T>())
            return spirv_asm
            {
                OpINotEqual $$bool result $x $zero
            };
        else if (__isFloat<T>())
            return spirv_asm
            {
                OpFUnordNotEqual $$bool result $x $zero
            };
        else if (__isBool<T>())
            return __slang_noop_cast<bool>(x);
    }
}

__generic<T : __BuiltinType, let N : int>
[__readNone]
bool all(vector<T,N> x)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "all";
    case glsl:
        __intrinsic_asm "all(bvec$N0($0))";
    case spirv:
        if (__isBool<T>())
            return spirv_asm
            {
                OpAll $$bool result $x
            };
        else if (__isInt<T>())
        {
            let zero = __default<vector<T,N>>();
            return spirv_asm
            {
                OpINotEqual $$vector<bool,N> %castResult $x $zero;
                OpAll $$bool result %castResult
            };
        }
        else
        {
            let zero = __default<T>();
            return spirv_asm
            {
                OpFUnordNotEqual $$vector<bool,N> %castResult $x $zero;
                OpAll $$bool result %castResult
            };
        }
    default:
        bool result = true;
        for(int i = 0; i < N; ++i)
            result = result && all(x[i]);
        return result;
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
bool all(matrix<T,N,M> x)
{
    bool result = true;
    for(int i = 0; i < N; ++i)
        result = result && all(x[i]);
    return result;
}

// Barrier for writes to all memory spaces (HLSL SM 5.0)
__glsl_extension(GL_KHR_memory_scope_semantics)
void AllMemoryBarrier()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "AllMemoryBarrier";
    case glsl: __intrinsic_asm "memoryBarrier(gl_ScopeDevice, (gl_StorageSemanticsShared|gl_StorageSemanticsImage|gl_StorageSemanticsBuffer), gl_SemanticsAcquireRelease)";
    case cuda: __intrinsic_asm "__threadfence()";
    case spirv: spirv_asm
        {
            OpMemoryBarrier Device AcquireRelease|UniformMemory|WorkgroupMemory|ImageMemory;
        };
    }
}

// Thread-group sync and barrier for writes to all memory spaces (HLSL SM 5.0)
__glsl_extension(GL_KHR_memory_scope_semantics)
void AllMemoryBarrierWithGroupSync()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "AllMemoryBarrierWithGroupSync";
    case glsl: __intrinsic_asm "controlBarrier(gl_ScopeWorkgroup, gl_ScopeDevice, (gl_StorageSemanticsShared|gl_StorageSemanticsImage|gl_StorageSemanticsBuffer), gl_SemanticsAcquireRelease)";
    case cuda: __intrinsic_asm "__syncthreads()";
    case spirv: spirv_asm
        {
            OpControlBarrier Workgroup Device AcquireRelease|UniformMemory|WorkgroupMemory|ImageMemory;
        };
    }
}

// Test if any components is non-zero (HLSL SM 1.0)

__generic<T : __BuiltinType>
[__readNone]
bool any(T x)
{
    __target_switch
    {
    default:
        __intrinsic_asm "bool($0)";
    case hlsl:
        __intrinsic_asm "any";
    case spirv:
        let zero = __default<T>();
        if (__isInt<T>())
            return spirv_asm
            {
                OpINotEqual $$bool result $x $zero
            };
        else if (__isFloat<T>())
            return spirv_asm
            {
                OpFUnordNotEqual $$bool result $x $zero
            };
        else if (__isBool<T>())
            return __slang_noop_cast<bool>(x);
    }
}

__generic<T : __BuiltinType, let N : int>
[__readNone]
bool any(vector<T, N> x)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "any";
    case glsl:
        __intrinsic_asm "any(bvec$N0($0))";
    case spirv:
        if (__isBool<T>())
            return spirv_asm
            {
                OpAny $$bool result $x
            };
        else if (__isInt<T>())
        {
            let zero = __default<vector<T,N>>();
            return spirv_asm
            {
                OpINotEqual $$vector<bool,N> %castResult $x $zero;
                OpAny $$bool result %castResult
            };
        }
        else
        {
            let zero = __default<T>();
            return spirv_asm
            {
                OpFUnordNotEqual $$vector<bool,N> %castResult $x $zero;
                OpAny $$bool result %castResult
            };
        }
    default:
        bool result = false;
        for(int i = 0; i < N; ++i)
            result = result || any(x[i]);
        return result;
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
bool any(matrix<T, N, M> x)
{
    bool result = false;
    for(int i = 0; i < N; ++i)
        result = result || any(x[i]);
    return result;
}


// Reinterpret bits as a double (HLSL SM 5.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "packDouble2x32(uvec2($0, $1))")
__target_intrinsic(cpp, "$P_asdouble($0, $1)")
__target_intrinsic(cuda, "$P_asdouble($0, $1)")
__target_intrinsic(spirv, "%v = OpCompositeConstruct _type(uint2) resultId _0 _1; OpExtInst resultType resultId glsl450 59 %v")
__glsl_extension(GL_ARB_gpu_shader5)
[__readNone]
double asdouble(uint lowbits, uint highbits);

// Reinterpret bits as a float (HLSL SM 4.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "intBitsToFloat")
__target_intrinsic(cpp, "$P_asfloat($0)")
__target_intrinsic(cuda, "$P_asfloat($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
float asfloat(int x);

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uintBitsToFloat")
__target_intrinsic(cpp, "$P_asfloat($0)")
__target_intrinsic(cuda, "$P_asfloat($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
float asfloat(uint x);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "intBitsToFloat")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
vector<float, N> asfloat(vector< int, N> x)
{
    VECTOR_MAP_UNARY(float, N, asfloat, x);
}

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uintBitsToFloat")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
vector<float,N> asfloat(vector<uint,N> x)
{
    VECTOR_MAP_UNARY(float, N, asfloat, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<float,N,M> asfloat(matrix< int,N,M> x)
{
    MATRIX_MAP_UNARY(float, N, M, asfloat, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<float,N,M> asfloat(matrix<uint,N,M> x)
{
    MATRIX_MAP_UNARY(float, N, M, asfloat, x);
}

// No op
[__unsafeForceInlineEarly]
[__readNone]
float asfloat(float x)
{ return x; }

__generic<let N : int>
[__unsafeForceInlineEarly]
[__readNone]
vector<float,N> asfloat(vector<float,N> x)
{ return x; }

__generic<let N : int, let M : int>
[__unsafeForceInlineEarly]
[__readNone]
matrix<float,N,M> asfloat(matrix<float,N,M> x)
{ return x; }

// Inverse sine (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_asin($0)")
__target_intrinsic(cpp, "$P_asin($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Asin _0")
[__readNone]
T asin(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Asin _0")
[__readNone]
vector<T, N> asin(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T,N,asin,x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> asin(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T,N,M,asin,x);
}

// Reinterpret bits as an int (HLSL SM 4.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToInt")
__target_intrinsic(cpp, "$P_asint($0)")
__target_intrinsic(cuda, "$P_asint($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
int asint(float x);

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "int($0)")
__target_intrinsic(cpp, "$P_asint($0)")
__target_intrinsic(cuda, "$P_asint($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
int asint(uint x);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToInt")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
vector<int, N> asint(vector<float, N> x)
{
    VECTOR_MAP_UNARY(int, N, asint, x);
}

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "ivec$N0($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
vector<int, N> asint(vector<uint, N> x)
{
    VECTOR_MAP_UNARY(int, N, asint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<int, N, M> asint(matrix<float, N, M> x)
{
    MATRIX_MAP_UNARY(int, N, M, asint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<int, N, M> asint(matrix<uint, N, M> x)
{
    MATRIX_MAP_UNARY(int, N, M, asint, x);
}

// No op
[__unsafeForceInlineEarly]
[__readNone]
int asint(int x)
{ return x; }

__generic<let N : int>
[__unsafeForceInlineEarly]
[__readNone]
vector<int,N> asint(vector<int,N> x)
{ return x; }

__generic<let N : int, let M : int>
[__unsafeForceInlineEarly]
[__readNone]
matrix<int,N,M> asint(matrix<int,N,M> x)
{ return x; }

// Reinterpret bits of double as a uint (HLSL SM 5.0)

__glsl_extension(GL_ARB_gpu_shader5)
[__readNone]
void asuint(double value, out uint lowbits, out uint highbits)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "asuint";
    case glsl: __intrinsic_asm "{ uvec2 v = unpackDouble2x32($0); $1 = v.x; $2 = v.y; }";
    case cpp:
    case cuda:
        __intrinsic_asm "$P_asuint($0, $1, $2)";
    case spirv:
        let uv = spirv_asm
        {
            result : $$uint2 = OpBitcast $value;
        };
        lowbits = uv.x;
        highbits = uv.y;
        return;
    }
}

// Reinterpret bits as a uint (HLSL SM 4.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToUint")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
__target_intrinsic(cpp, "$P_asuint($0)")
__target_intrinsic(cuda, "$P_asuint($0)")
[__readNone]
uint asuint(float x);

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uint($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
__target_intrinsic(cpp, "$P_asuint($0)")
__target_intrinsic(cuda, "$P_asuint($0)")
[__readNone]
uint asuint(int x);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToUint")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
vector<uint,N> asuint(vector<float,N> x)
{
    VECTOR_MAP_UNARY(uint, N, asuint, x);
}

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uvec$N0($0)")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
vector<uint, N> asuint(vector<int, N> x)
{
    VECTOR_MAP_UNARY(uint, N, asuint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<uint,N,M> asuint(matrix<float,N,M> x)
{
    MATRIX_MAP_UNARY(uint, N, M, asuint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<uint, N, M> asuint(matrix<int, N, M> x)
{
    MATRIX_MAP_UNARY(uint, N, M, asuint, x);
}

[__unsafeForceInlineEarly]
[__readNone]
uint asuint(uint x)
{ return x; }

__generic<let N : int>
[__unsafeForceInlineEarly]
[__readNone]
vector<uint,N> asuint(vector<uint,N> x)
{ return x; }

__generic<let N : int, let M : int>
[__unsafeForceInlineEarly]
[__readNone]
matrix<uint,N,M> asuint(matrix<uint,N,M> x)
{ return x; }


// 16-bit bitcast ops (HLSL SM 6.2)
//
// TODO: We need to map these to GLSL/SPIR-V
// operations that don't require an intermediate
// conversion to fp32.

// Identity cases:

[__unsafeForceInlineEarly][__readNone] float16_t asfloat16(float16_t value) { return value; }
[__unsafeForceInlineEarly][__readNone] vector<float16_t,N> asfloat16<let N : int>(vector<float16_t,N> value) { return value; }
[__unsafeForceInlineEarly][__readNone] matrix<float16_t,R,C> asfloat16<let R : int, let C : int>(matrix<float16_t,R,C> value) { return value; }

[__unsafeForceInlineEarly][__readNone] int16_t asint16(int16_t value) { return value; }
[__unsafeForceInlineEarly][__readNone] vector<int16_t,N> asint16<let N : int>(vector<int16_t,N> value) { return value; }
[__unsafeForceInlineEarly][__readNone] matrix<int16_t,R,C> asint16<let R : int, let C : int>(matrix<int16_t,R,C> value) { return value; }

[__unsafeForceInlineEarly][__readNone] uint16_t asuint16(uint16_t value) { return value; }
[__unsafeForceInlineEarly][__readNone] vector<uint16_t,N> asuint16<let N : int>(vector<uint16_t,N> value) { return value; }
[__unsafeForceInlineEarly][__readNone] matrix<uint16_t,R,C> asuint16<let R : int, let C : int>(matrix<uint16_t,R,C> value) { return value; }

// Signed<->unsigned cases:

[__unsafeForceInlineEarly][__readNone] int16_t asint16(uint16_t value) { return value; }
[__unsafeForceInlineEarly][__readNone] vector<int16_t,N> asint16<let N : int>(vector<uint16_t,N> value) { return value; }
[__unsafeForceInlineEarly][__readNone] matrix<int16_t,R,C> asint16<let R : int, let C : int>(matrix<uint16_t,R,C> value) { return value; }

[__unsafeForceInlineEarly][__readNone] uint16_t asuint16(int16_t value) { return value; }
[__unsafeForceInlineEarly][__readNone] vector<uint16_t,N> asuint16<let N : int>(vector<int16_t,N> value) { return value; }
[__unsafeForceInlineEarly][__readNone] matrix<uint16_t,R,C> asuint16<let R : int, let C : int>(matrix<int16_t,R,C> value) { return value; }

// Float->unsigned cases:

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uint16_t(packHalf2x16(vec2($0, 0.0)))")
__target_intrinsic(cuda, "__half_as_ushort")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
uint16_t asuint16(float16_t value);

[__readNone]
vector<uint16_t,N> asuint16<let N : int>(vector<float16_t,N> value)
{ VECTOR_MAP_UNARY(uint16_t, N, asuint16, value); }

[__readNone]
matrix<uint16_t,R,C> asuint16<let R : int, let C : int>(matrix<float16_t,R,C> value)
{ MATRIX_MAP_UNARY(uint16_t, R, C, asuint16, value); }

// Unsigned->float cases:

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "float16_t(unpackHalf2x16($0).x)")
__target_intrinsic(cuda, "__ushort_as_half")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
float16_t asfloat16(uint16_t value);

[__readNone]
vector<float16_t,N> asfloat16<let N : int>(vector<uint16_t,N> value)
{ VECTOR_MAP_UNARY(float16_t, N, asfloat16, value); }

[__readNone]
matrix<float16_t,R,C> asfloat16<let R : int, let C : int>(matrix<uint16_t,R,C> value)
{ MATRIX_MAP_UNARY(float16_t, R, C, asfloat16, value); }

// Float<->signed cases:

__target_intrinsic(hlsl)
__target_intrinsic(cuda, "__half_as_short")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__unsafeForceInlineEarly][__readNone] int16_t asint16(float16_t value) { return asuint16(value); }
__target_intrinsic(hlsl) [__unsafeForceInlineEarly][__readNone] vector<int16_t,N> asint16<let N : int>(vector<float16_t,N> value) { return asuint16(value); }
__target_intrinsic(hlsl) [__unsafeForceInlineEarly][__readNone] matrix<int16_t,R,C> asint16<let R : int, let C : int>(matrix<float16_t,R,C> value) { return asuint16(value); }

__target_intrinsic(hlsl)
__target_intrinsic(cuda, "__short_as_half")
__target_intrinsic(spirv, "OpBitcast resultType resultId _0")
[__readNone]
[__unsafeForceInlineEarly] float16_t asfloat16(int16_t value) { return asfloat16(asuint16(value)); }

__target_intrinsic(hlsl) [__unsafeForceInlineEarly][__readNone] vector<float16_t,N> asfloat16<let N : int>(vector<int16_t,N> value) { return asfloat16(asuint16(value)); }
__target_intrinsic(hlsl) [__unsafeForceInlineEarly][__readNone] matrix<float16_t,R,C> asfloat16<let R : int, let C : int>(matrix<int16_t,R,C> value) { return asfloat16(asuint16(value)); }

// Inverse tangent (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_atan($0)")
__target_intrinsic(cpp, "$P_atan($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Atan _0")
[__readNone]
T atan(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Atan _0")
[__readNone]
vector<T, N> atan(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, atan, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> atan(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, atan, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"atan($0,$1)")
__target_intrinsic(cuda, "$P_atan2($0, $1)")
__target_intrinsic(cpp, "$P_atan2($0, $1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Atan2 _0 _1")
[__readNone]
T atan2(T y, T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"atan($0,$1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Atan2 _0 _1")
[__readNone]
vector<T, N> atan2(vector<T, N> y, vector<T, N> x)
{
    VECTOR_MAP_BINARY(T, N, atan2, y, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> atan2(matrix<T,N,M> y, matrix<T,N,M> x)
{
    MATRIX_MAP_BINARY(T, N, M, atan2, y, x);
}

// Ceiling (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_ceil($0)")
__target_intrinsic(cpp, "$P_ceil($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Ceil _0")
[__readNone]
T ceil(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Ceil _0")
[__readNone]
vector<T, N> ceil(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ceil, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ceil(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ceil, x);
}


// Check access status to tiled resource
bool CheckAccessFullyMapped(uint status);

// Clamp (HLSL SM 1.0)
__generic<T : __BuiltinIntegerType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FClamp, UClamp, SClamp) _0 _1 _2")
[__readNone]
T clamp(T x, T minBound, T maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FClamp, UClamp, SClamp) _0 _1 _2")
[__readNone]
vector<T, N> clamp(vector<T, N> x, vector<T, N> minBound, vector<T, N> maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> clamp(matrix<T,N,M> x, matrix<T,N,M> minBound, matrix<T,N,M> maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FClamp, UClamp, SClamp) _0 _1 _2")
[__readNone]
T clamp(T x, T minBound, T maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FClamp, UClamp, SClamp) _0 _1 _2")
[__readNone]
vector<T, N> clamp(vector<T, N> x, vector<T, N> minBound, vector<T, N> maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> clamp(matrix<T,N,M> x, matrix<T,N,M> minBound, matrix<T,N,M> maxBound)
{
    return min(max(x, minBound), maxBound);
}

// Clip (discard) fragment conditionally
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
void clip(T x)
{
    if(x < T(0)) discard;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
void clip(vector<T,N> x)
{
    if(any(x < T(0))) discard;
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
void clip(matrix<T,N,M> x)
{
    if(any(x < T(0))) discard;
}

// Cosine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_cos($0)")
__target_intrinsic(cpp, "$P_cos($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Cos _0")
[__readNone]
T cos(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Cos _0")
[__readNone]
vector<T, N> cos(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T,N, cos, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> cos(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, cos, x);
}

// Hyperbolic cosine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_cosh($0)")
__target_intrinsic(cpp, "$P_cosh($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Cosh _0")
[__readNone]
T cosh(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Cosh _0")
[__readNone]
vector<T,N> cosh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T,N, cosh, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> cosh(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, cosh, x);
}

// Population count
[__readNone]
uint countbits(uint value)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "countbits";
    case glsl:
        __intrinsic_asm "bitCount";
    case cuda:
    case cpp:
        __intrinsic_asm "$P_countbits($0)";
    case spirv:
        return spirv_asm {OpBitCount $$uint result $value};
    }
}

// Cross product
// TODO: SPIRV does not support integer vectors.
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Cross _0 _1")
[__readNone]
vector<T,3> cross(vector<T,3> left, vector<T,3> right)
{
    return vector<T,3>(
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x);
}

__generic<T : __BuiltinIntegerType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Cross _0 _1")
[__readNone]
vector<T, 3> cross(vector<T, 3> left, vector<T, 3> right)
{
    return vector<T, 3>(
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x);
}

// Convert encoded color
__target_intrinsic(hlsl)
[__readNone]
int4 D3DCOLORtoUBYTE4(float4 color)
{
    let scaled = color.zyxw * 255.001999f;
    return int4(scaled);
}

// Partial-difference derivatives
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, dFdx)
[__readNone]
T ddx(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, dFdx)
[__readNone]
vector<T, N> ddx(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddx, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ddx(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddx, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
[__readNone]
T ddx_coarse(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
[__readNone]
vector<T, N> ddx_coarse(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddx_coarse, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ddx_coarse(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddx_coarse, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
[__readNone]
T ddx_fine(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
[__readNone]
vector<T, N> ddx_fine(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddx_fine, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ddx_fine(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddx_fine, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, dFdy)
[__readNone]
T ddy(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, dFdy)
[__readNone]
vector<T, N> ddy(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddy, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ddy(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddy, x);
}

__generic<T : __BuiltinFloatingPointType>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
[__readNone]
T ddy_coarse(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
[__readNone]
vector<T, N> ddy_coarse(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddy_coarse, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ddy_coarse(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddy_coarse, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
[__readNone]
T ddy_fine(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
[__readNone]
vector<T, N> ddy_fine(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddy_fine, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ddy_fine(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddy_fine, x);
}


// Radians to degrees

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Degrees _0")
[__readNone]
T degrees(T x)
{
    return x * (T(180) / T.getPi());
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Degrees _0")
[__readNone]
vector<T, N> degrees(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, degrees, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> degrees(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, degrees, x);
}

// Matrix determinant

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Determinant _0")
[__readNone]
[PreferCheckpoint]
T determinant(matrix<T,N,N> m);

// Barrier for device memory
__glsl_extension(GL_KHR_memory_scope_semantics)
void DeviceMemoryBarrier()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "DeviceMemoryBarrier";
    case glsl: __intrinsic_asm "memoryBarrier(gl_ScopeDevice, (gl_StorageSemanticsImage|gl_StorageSemanticsBuffer), gl_SemanticsAcquireRelease)";
    case cuda: __intrinsic_asm "__threadfence()";
    case spirv: spirv_asm
        {
            OpMemoryBarrier Device AcquireRelease|UniformMemory|ImageMemory;
        };
    }
}

__glsl_extension(GL_KHR_memory_scope_semantics)
void DeviceMemoryBarrierWithGroupSync()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "DeviceMemoryBarrierWithGroupSync";
    case glsl: __intrinsic_asm "controlBarrier(gl_ScopeWorkgroup, gl_ScopeDevice, (gl_StorageSemanticsImage|gl_StorageSemanticsBuffer), gl_SemanticsAcquireRelease)";
    case cuda: __intrinsic_asm "__syncthreads()";
    case spirv: spirv_asm
        {
            OpControlBarrier Workgroup Device AcquireRelease|UniformMemory|ImageMemory;
        };
    }
}

// Vector distance

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Distance _0 _1")
[__readNone]
T distance(vector<T, N> x, vector<T, N> y)
{
    return length(x - y);
}

__generic<T : __BuiltinFloatingPointType>
[__readNone]
T distance(T x, T y)
{
    return length(x - y);
}

// Vector dot product

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpDot resultType resultId _0 _1")
[__readNone]
T dot(vector<T, N> x, vector<T, N> y)
{
    T result = T(0);
    for(int i = 0; i < N; ++i)
        result += x[i] * y[i];
    return result;
}

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
T dot(vector<T, N> x, vector<T, N> y)
{
    T result = T(0);
    for(int i = 0; i < N; ++i)
        result += x[i] * y[i];
    return result;
}


// Helper for computing distance terms for lighting (obsolete)

__generic<T : __BuiltinFloatingPointType> vector<T,4> dst(vector<T,4> x, vector<T,4> y);

// Given a RWByteAddressBuffer allow it to be interpreted as a RWStructuredBuffer
__intrinsic_op($(kIROp_GetEquivalentStructuredBuffer))
RWStructuredBuffer<T> __getEquivalentStructuredBuffer<T>(RWByteAddressBuffer b);

__intrinsic_op($(kIROp_GetEquivalentStructuredBuffer))
StructuredBuffer<T> __getEquivalentStructuredBuffer<T>(ByteAddressBuffer b);

__intrinsic_op($(kIROp_GetEquivalentStructuredBuffer))
RasterizerOrderedStructuredBuffer<T> __getEquivalentStructuredBuffer<T>(RasterizerOrderedByteAddressBuffer b);

// Error message

// void errorf( string format, ... );

// Attribute evaluation

// TODO: The matrix cases of these functions won't actuall work
// when compiled to GLSL, since they only support scalar/vector

// TODO: Should these be constrains to `__BuiltinFloatingPointType`?
// TODO: SPIRV-direct does not support non-floating-point types.

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, interpolateAtCentroid)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 InterpolateAtCentroid _0")
[__readNone]
T EvaluateAttributeAtCentroid(T x);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, interpolateAtCentroid)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 InterpolateAtCentroid _0")
[__readNone]
vector<T,N> EvaluateAttributeAtCentroid(vector<T,N> x);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, interpolateAtCentroid)
[__readNone]
matrix<T,N,M> EvaluateAttributeAtCentroid(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, EvaluateAttributeAtCentroid, x);
}

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 InterpolateAtSample _0 _1")
[__readNone]
T EvaluateAttributeAtSample(T x, uint sampleindex);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 InterpolateAtSample _0 _1")
[__readNone]
vector<T,N> EvaluateAttributeAtSample(vector<T,N> x, uint sampleindex);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
[__readNone]
matrix<T,N,M> EvaluateAttributeAtSample(matrix<T,N,M> x, uint sampleindex)
{
    matrix<T,N,M> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = EvaluateAttributeAtSample(x[i], sampleindex);
    }
    return result;
}

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
__target_intrinsic(spirv, "%foffset = OpConvertSToF _type(float2) resultId _1; %offsetdiv16 = 136 _type(float2) resultId %foffset const(float2, 16.0, 16.0); OpExtInst resultType resultId glsl450 78 _0 %offsetdiv16")
[__readNone]
T EvaluateAttributeSnapped(T x, int2 offset);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
__target_intrinsic(spirv, "%foffset = OpConvertSToF _type(float2) resultId _1; %offsetdiv16 = 136 _type(float2) resultId %foffset const(float2, 16.0, 16.0); OpExtInst resultType resultId glsl450 78 _0 %offsetdiv16")
[__readNone]
vector<T,N> EvaluateAttributeSnapped(vector<T,N> x, int2 offset);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
[__readNone]
matrix<T,N,M> EvaluateAttributeSnapped(matrix<T,N,M> x, int2 offset)
{
    matrix<T,N,M> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = EvaluateAttributeSnapped(x[i], offset);
    }
    return result;
}

// Base-e exponent

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_exp($0)")
__target_intrinsic(cpp, "$P_exp($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Exp _0")
[__readNone]
T exp(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Exp _0")
[__readNone]
vector<T, N> exp(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, exp, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> exp(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, exp, x);
}

// Base-2 exponent

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_exp2($0)")
__target_intrinsic(cpp, "$P_exp2($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Exp2 _0")
[__readNone]
T exp2(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Exp2 _0")
[__readNone]
vector<T,N> exp2(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, exp2, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> exp2(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, exp2, x);
}


// Convert 16-bit float stored in low bits of integer
__target_intrinsic(glsl, "unpackHalf2x16($0).x")
__glsl_version(420)
__target_intrinsic(hlsl)
__cuda_sm_version(6.0)
__target_intrinsic(cuda, "__half2float(__ushort_as_half($0))")
__target_intrinsic(spirv, R"(
                    %lowBits = OpUConvert _type(uint16_t) resultId _0;
                    %half = OpBitcast _type(half) resultId %lowBits;
                    OpFConvert resultType resultId %half)")
[__readNone]
float f16tof32(uint value);

__generic<let N : int>
__target_intrinsic(hlsl)
[__readNone]
vector<float, N> f16tof32(vector<uint, N> value)
{
    VECTOR_MAP_UNARY(float, N, f16tof32, value);
}



// Convert to 16-bit float stored in low bits of integer
__target_intrinsic(glsl, "packHalf2x16(vec2($0,0.0))")
__glsl_version(420)
__target_intrinsic(hlsl)
__cuda_sm_version(6.0)
__target_intrinsic(cuda, "__half_as_ushort(__float2half($0))")
__target_intrinsic(spirv, R"(
                    %half = OpFConvert _type(half) resultId _0;
                    %lowBits = OpBitcast _type(uint16_t) resultId %half;
                    OpUConvert resultType resultId %lowBits)")
[__readNone]
uint f32tof16(float value);

__generic<let N : int>
__target_intrinsic(hlsl)
[__readNone]
vector<uint, N> f32tof16(vector<float, N> value)
{
    VECTOR_MAP_UNARY(uint, N, f32tof16, value);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// The following is Slang specific and NOT part of standard HLSL
// It's not clear what happens with float16 time in HLSL -> can the float16 coerce to uint for example? If so that would
// give the wrong result

__target_intrinsic(glsl, "unpackHalf2x16($0).x")
__target_intrinsic(cuda, "__half2float")
__target_intrinsic(spirv, "OpFConvert resultType resultId _0")
__glsl_version(420)
[__readNone]
float f16tof32(float16_t value);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "__half2float")
__target_intrinsic(spirv, "OpFConvert resultType resultId _0")
[__readNone]
vector<float, N> f16tof32(vector<float16_t, N> value)
{
    VECTOR_MAP_UNARY(float, N, f16tof32, value);
}

// Convert to float16_t
__target_intrinsic(glsl, "packHalf2x16(vec2($0,0.0))")
__glsl_version(420)
__target_intrinsic(cuda, "__float2half")
__target_intrinsic(spirv, "OpFConvert resultType resultId _0")
[__readNone]
float16_t f32tof16_(float value);

__generic<let N : int>
__target_intrinsic(cuda, "__float2half")
__target_intrinsic(spirv, "OpFConvert resultType resultId _0")
[__readNone]
vector<float16_t, N> f32tof16_(vector<float, N> value)
{
    VECTOR_MAP_UNARY(float16_t, N, f32tof16, value);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Flip surface normal to face forward, if needed
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FaceForward _0 _1 _2")
[__readNone]
vector<T,N> faceforward(vector<T,N> n, vector<T,N> i, vector<T,N> ng)
{
    return dot(ng, i) < T(0.0f) ? n : -n;
}

// Find first set bit starting at high bit and working down
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(cuda, "$P_firstbithigh($0)")
__target_intrinsic(cpp, "$P_firstbithigh($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindSMsb _0")
[__readNone]
int firstbithigh(int value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindSMsb _0")
__generic<let N : int>
[__readNone]
vector<int, N> firstbithigh(vector<int, N> value)
{
    VECTOR_MAP_UNARY(int, N, firstbithigh, value);
}

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(cuda, "$P_firstbithigh($0)")
__target_intrinsic(cpp, "$P_firstbithigh($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindUMsb _0")
[__readNone]
uint firstbithigh(uint value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindUMsb _0")
__generic<let N : int>
[__readNone]
vector<uint,N> firstbithigh(vector<uint,N> value)
{
    VECTOR_MAP_UNARY(uint, N, firstbithigh, value);
}

// Find first set bit starting at low bit and working up
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__target_intrinsic(cuda, "$P_firstbitlow($0)")
__target_intrinsic(cpp, "$P_firstbitlow($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindILsb _0")
[__readNone]
int firstbitlow(int value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindILsb _0")
__generic<let N : int>
[__readNone]
vector<int,N> firstbitlow(vector<int,N> value)
{
    VECTOR_MAP_UNARY(int, N, firstbitlow, value);
}

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__target_intrinsic(cuda, "$P_firstbitlow($0)")
__target_intrinsic(cpp, "$P_firstbitlow($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindILsb _0")
[__readNone]
uint firstbitlow(uint value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__generic<let N : int>
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FindILsb _0")
[__readNone]
vector<uint,N> firstbitlow(vector<uint,N> value)
{
    VECTOR_MAP_UNARY(uint, N, firstbitlow, value);
}

// Floor (HLSL SM 1.0)

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_floor($0)")
__target_intrinsic(cpp, "$P_floor($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Floor _0")
[__readNone]
T floor(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Floor _0")
[__readNone]
vector<T, N> floor(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, floor, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> floor(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, floor, x);
}

// Fused multiply-add for doubles
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_fma($0, $1, $2)")
__target_intrinsic(cpp, "$P_fma($0, $1, $2)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fma _0 _1 _2")
[__readNone]
double fma(double a, double b, double c);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fma _0 _1 _2")
[__readNone]
vector<double, N> fma(vector<double, N> a, vector<double, N> b, vector<double, N> c)
{
    VECTOR_MAP_TRINARY(double, N, fma, a, b, c);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<double, N, M> fma(matrix<double, N, M> a, matrix<double, N, M> b, matrix<double, N, M> c)
{
    MATRIX_MAP_TRINARY(double, N, M, fma, a, b, c);
}

// Floating point remainder of x/y
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "$P_fmod($0, $1)")
__target_intrinsic(cpp, "$P_fmod($0, $1)")
[__readNone]
T fmod(T x, T y)
{
    return x - y * trunc(x/y);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
vector<T, N> fmod(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, fmod, x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> fmod(matrix<T, N, M> x, matrix<T, N, M> y)
{
    MATRIX_MAP_BINARY(T, N, M, fmod, x, y);
}

// Fractional part
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fract)
__target_intrinsic(cuda, "$P_frac($0)")
__target_intrinsic(cpp, "$P_frac($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fract _0")
[__readNone]
T frac(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fract)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fract _0")
[__readNone]
vector<T, N> frac(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, frac, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[__readNone]
matrix<T, N, M> frac(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, frac, x);
}

// Split float into mantissa and exponent
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(cpp, "$P_frexp($0, $1)")
__target_intrinsic(cuda, "$P_frexp($0, $1)")
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Frexp _0 _1")
[__readNone]
T frexp(T x, out int exp);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Frexp _0 _1")
[__readNone]
vector<T, N> frexp(vector<T, N> x, out vector<int, N> exp)
{
    VECTOR_MAP_BINARY(T, N, frexp, x, exp);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int, let L : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> frexp(matrix<T, N, M> x, out matrix<int, N, M, L> exp)
{
    MATRIX_MAP_BINARY(T, N, M, frexp, x, exp);
}

// Texture filter width
__generic<T : __BuiltinFloatingPointType>
[__readNone]
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpFWidth resultType resultId _0")
T fwidth(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpFWidth resultType resultId _0")
[__readNone]
vector<T, N> fwidth(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, fwidth, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> fwidth(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, fwidth, x);
}

/// Get the value of a vertex attribute at a specific vertex.
///
/// The `GetAttributeAtVertex()` function can be used in a fragment shader
/// to get the value of the given `attribute` at the vertex of the primitive
/// that corresponds to the given `vertexIndex`.
///
/// Note that the `attribute` must have been a declared varying input to
/// the fragment shader with the `nointerpolation` modifier.
///
/// This function can be applied to scalars, vectors, and matrices of
/// built-in scalar types.
///
__generic<T : __BuiltinType>
[__readNone]
__glsl_version(450)
T GetAttributeAtVertex(T attribute, uint vertexIndex)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "GetAttributeAtVertex";
    case GL_NV_fragment_shader_barycentric:
    case GL_EXT_fragment_shader_barycentric: 
        __intrinsic_asm "$0[$1]";
    case spirv:
        return spirv_asm {
            %_ptr_Input_T = OpTypePointer Input $$T;
            %addr = OpAccessChain %_ptr_Input_T $attribute $vertexIndex;
            result:$$T = OpLoad %addr;
        };
    }
}

/// Get the value of a vertex attribute at a specific vertex.
///
/// The `GetAttributeAtVertex()` function can be used in a fragment shader
/// to get the value of the given `attribute` at the vertex of the primitive
/// that corresponds to the given `vertexIndex`.
///
/// Note that the `attribute` must have been a declared varying input to
/// the fragment shader with the `nointerpolation` modifier.
///
/// This function can be applied to scalars, vectors, and matrices of
/// built-in scalar types.
///
__generic<T : __BuiltinType, let N : int>
[__readNone]
__glsl_version(450)
vector<T,N> GetAttributeAtVertex(vector<T,N> attribute, uint vertexIndex)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "GetAttributeAtVertex";
    case GL_NV_fragment_shader_barycentric:
    case GL_EXT_fragment_shader_barycentric: 
        __intrinsic_asm "$0[$1]";
    case spirv:
        return spirv_asm {
            %_ptr_Input_vectorT = OpTypePointer Input $$vector<T,N>;
            %addr = OpAccessChain %_ptr_Input_vectorT $attribute $vertexIndex;
            result:$$vector<T,N> = OpLoad %addr;
        };
    }
}

/// Get the value of a vertex attribute at a specific vertex.
///
/// The `GetAttributeAtVertex()` function can be used in a fragment shader
/// to get the value of the given `attribute` at the vertex of the primitive
/// that corresponds to the given `vertexIndex`.
///
/// Note that the `attribute` must have been a declared varying input to
/// the fragment shader with the `nointerpolation` modifier.
///
/// This function can be applied to scalars, vectors, and matrices of
/// built-in scalar types.
///
__generic<T : __BuiltinType, let N : int, let M : int>
[__readNone]
__glsl_version(450)
matrix<T,N,M> GetAttributeAtVertex(matrix<T,N,M> attribute, uint vertexIndex)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "GetAttributeAtVertex";
    case GL_NV_fragment_shader_barycentric:
    case GL_EXT_fragment_shader_barycentric: 
        __intrinsic_asm "$0[$1]";
    case spirv:
        return spirv_asm {
            %_ptr_Input_matrixT = OpTypePointer Input $$matrix<T,N,M>;
            %addr = OpAccessChain %_ptr_Input_matrixT $attribute $vertexIndex;
            result:$$matrix<T,N,M> = OpLoad %addr;
        };
    }
}

// Get number of samples in render target
[__readNone]
uint GetRenderTargetSampleCount();

// Get position of given sample
[__readNone]
float2 GetRenderTargetSamplePosition(int Index);

// Group memory barrier
__glsl_extension(GL_KHR_memory_scope_semantics)
void GroupMemoryBarrier()
{
    __target_switch
    {
    case glsl: __intrinsic_asm "memoryBarrier(gl_ScopeWorkgroup, gl_StorageSemanticsShared, gl_SemanticsAcquireRelease)";
    case hlsl: __intrinsic_asm "GroupMemoryBarrier";
    case cuda: __intrinsic_asm "__threadfence_block";
    case spirv:
        spirv_asm
        {
            OpMemoryBarrier Workgroup AcquireRelease|WorkgroupMemory
        };
    }
}

void __subgroupBarrier()
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBarrier";
    case hlsl: __intrinsic_asm "GroupMemoryBarrierWithGroupSync";
    case cuda: __intrinsic_asm "__syncthreads()";
    case spirv:
        spirv_asm
        {
            OpControlBarrier Subgroup Subgroup AcquireRelease|WorkgroupMemory|ImageMemory|UniformMemory
        };
    }
}

void GroupMemoryBarrierWithGroupSync()
{
    __target_switch
    {
    case glsl: __intrinsic_asm "barrier";
    case hlsl: __intrinsic_asm "GroupMemoryBarrierWithGroupSync";
    case cuda: __intrinsic_asm "__syncthreads()";
    case spirv:
        spirv_asm
        {
            OpControlBarrier Workgroup Workgroup AcquireRelease|WorkgroupMemory
        };
    }
}

// Atomics

__glsl_version(430)
void InterlockedAdd(__ref  int dest,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAdd";
    case cuda: __intrinsic_asm "atomicAdd($0, $1)";
    case glsl: __intrinsic_asm "$atomicAdd($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicIAdd &dest Device None $value 
        };
    }
}

__glsl_version(430)
void InterlockedAdd(__ref uint dest, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAdd";
    case cuda: __intrinsic_asm "atomicAdd((int*)$0, $1)";
    case glsl: __intrinsic_asm "$atomicAdd($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicIAdd &dest Device None $value 
        };
    }
}

[ForceInline]
void InterlockedAdd(__ref uint dest, int value)
{
    InterlockedAdd(dest, (uint)value);
}

__glsl_version(430)
void InterlockedAdd(__ref  int dest,  int value, out  int original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAdd";
    case cuda: __intrinsic_asm "(*$2 = atomicAdd($0, $1))";
    case glsl: __intrinsic_asm "($2 = $atomicAdd($A, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$int = OpAtomicIAdd &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedAdd(__ref uint dest, uint value, out uint original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAdd";
    case cuda: __intrinsic_asm "(*$2 = (uint)atomicAdd((int*)$0, $1))";
    case glsl: __intrinsic_asm "($2 = $atomicAdd($A, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$uint = OpAtomicIAdd &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedAnd(__ref  int dest,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAnd";
    case cuda: __intrinsic_asm "atomicAnd($0, $1)";
    case glsl: __intrinsic_asm "$atomicAnd($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicAnd &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedAnd(__ref uint dest, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAnd";
    case cuda: __intrinsic_asm "atomicAnd((int*)$0, $1)";
    case glsl: __intrinsic_asm "$atomicAnd($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicAnd &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedAnd(__ref  int dest,  int value, out  int original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAnd";
    case cuda: __intrinsic_asm "(*$2 = atomicAnd($0, $1))";
    case glsl: __intrinsic_asm "($2 = $atomicAnd($A, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$int = OpAtomicAnd &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedAnd(__ref uint dest, uint value, out uint original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedAnd";
    case glsl: __intrinsic_asm "($2 = atomicAnd($0, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicAnd((int*)$0, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$uint = OpAtomicAnd &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedCompareExchange(__ref  int dest,  int compare_value,  int value, out  int original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedCompareExchange";
    case glsl: __intrinsic_asm "($3 = $atomicCompSwap($A, $1, $2))";
    case cuda: __intrinsic_asm "(*$3 = atomicCAS($0, $1, $2))";
    case spirv:
        spirv_asm
        {
            %original:$$int = OpAtomicCompareExchange &dest Device None None $value $compare_value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedCompareExchange(__ref uint dest, uint compare_value, uint value, out uint original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedCompareExchange";
    case glsl: __intrinsic_asm "($3 = $atomicCompSwap($A, $1, $2))";
    case cuda: __intrinsic_asm "(*$3 = (uint)atomicCAS((int*)$0, $1, $2))";
    case spirv:
        spirv_asm
        {
            %original:$$uint = OpAtomicCompareExchange &dest Device None None $value $compare_value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedCompareStore(__ref int dest,  int compare_value,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedCompareStore";
    case glsl: __intrinsic_asm "$atomicCompSwap($A, $1, $2)";
    case cuda: __intrinsic_asm "atomicCAS($0, $1, $2)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicCompareExchange &dest Device None None $value $compare_value;
        };
    }
}

__glsl_version(430)
void InterlockedCompareStore(__ref uint dest, uint compare_value, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedCompareStore";
    case glsl: __intrinsic_asm "$atomicCompSwap($A, $1, $2)";
    case cuda: __intrinsic_asm "atomicCAS((int*)$0, $1, $2)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicCompareExchange &dest Device None None $value $compare_value;
        };
    }
}

__glsl_version(430)
void InterlockedExchange(__ref  int dest,  int value, out  int original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedExchange";
    case glsl: __intrinsic_asm "($2 = $atomicExchange($A, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicExch($0, $1))";
    case spirv:
        spirv_asm
        {
            %r:$$int = OpAtomicExchange &dest Device None $value;
            OpStore &original_value %r
        };
    }
}

__glsl_version(430)
void InterlockedExchange(__ref uint dest, uint value, out uint original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedExchange";
    case glsl: __intrinsic_asm "($2 = $atomicExchange($A, $1))";
    case cuda: __intrinsic_asm "(*$2 = (uint)atomicExch((int*)$0, $1))";
    case spirv:
        spirv_asm
        {
            %r:$$uint = OpAtomicExchange &dest Device None $value;
            OpStore &original_value %r
        };
    }
}

__glsl_version(430)
void InterlockedMax(__ref  int dest,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMax";
    case glsl: __intrinsic_asm "$atomicMax($A, $1)";
    case cuda: __intrinsic_asm "atomicMax($0, $1)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicSMax &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedMax(__ref uint dest, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMax";
    case glsl: __intrinsic_asm "$atomicMax($A, $1)";
    case cuda: __intrinsic_asm "atomicMax((int*)$0, $1)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicUMax &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedMax(__ref  int dest,  int value, out  int original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMax";
    case glsl: __intrinsic_asm "($2 = $atomicMax($A, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicMax($0, $1))";
    case spirv:
        spirv_asm
        {
            %v:$$int = OpAtomicSMax &dest Device None $value;
            OpStore &original_value %v
        };
    }
}

__glsl_version(430)
void InterlockedMax(__ref uint dest, uint value, out uint original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMax";
    case glsl: __intrinsic_asm "($2 = $atomicMax($A, $1))";
    case cuda: __intrinsic_asm "(*$2 = (uint)atomicMax((int*)$0, $1))";
    case spirv:
        spirv_asm
        {
            %v:$$uint = OpAtomicUMax &dest Device None $value;
            OpStore &original_value %v
        };
    }
}

__glsl_version(430)
void InterlockedMin(__ref  int dest,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMin";
    case glsl: __intrinsic_asm "$atomicMin($A, $1)";
    case cuda: __intrinsic_asm "atomicMin($0, $1)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicSMin &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedMin(__ref uint dest, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMin";
    case glsl: __intrinsic_asm "$atomicMin($A, $1)";
    case cuda: __intrinsic_asm "atomicMin((int*)$0, $1)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicUMin &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedMin(__ref  int dest,  int value, out  int original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMin";
    case glsl: __intrinsic_asm "($2 = $atomicMin($A, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicMin($0, $1))";
    case spirv:
        spirv_asm
        {
            %v:$$int = OpAtomicSMin &dest Device None $value;
            OpStore &original_value %v
        };
    }
}

__glsl_version(430)
void InterlockedMin(__ref uint dest, uint value, out uint original_value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedMin";
    case glsl: __intrinsic_asm "($2 = $atomicMin($A, $1))";
    case cuda: __intrinsic_asm "(*$2 = (uint)atomicMin((int*)$0, $1))";
    case spirv:
        spirv_asm
        {
            %v:$$uint = OpAtomicUMin &dest Device None $value;
            OpStore &original_value %v
        };
    }
}

__glsl_version(430)
void InterlockedOr(__ref  int dest,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedOr";
    case cuda: __intrinsic_asm "atomicOr((int*)$0, $1)";
    case glsl: __intrinsic_asm "$atomicOr($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicOr &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedOr(__ref uint dest, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedOr";
    case cuda: __intrinsic_asm "atomicOr((int*)$0, $1)";
    case glsl: __intrinsic_asm "$atomicOr($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicOr &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedOr(__ref  int dest,  int value, out  int original_value)
{
     __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedOr";
    case glsl: __intrinsic_asm "($2 = atomicOr($0, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicOr($0, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$int = OpAtomicOr &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedOr(__ref uint dest, uint value, out uint original_value)
{
     __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedOr";
    case glsl: __intrinsic_asm "($2 = atomicOr($0, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicOr((int*)$0, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$uint = OpAtomicOr &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedXor(__ref  int dest,  int value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedXor";
    case cuda: __intrinsic_asm "atomicXor((int*)$0, $1)";
    case glsl: __intrinsic_asm "$atomicXor($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$int = OpAtomicXor &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedXor(__ref uint dest, uint value)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedXor";
    case cuda: __intrinsic_asm "atomicXor((int*)$0, $1)";
    case glsl: __intrinsic_asm "$atomicXor($A, $1)";
    case spirv:
        spirv_asm
        {
            result:$$uint = OpAtomicXor &dest Device None $value;
        };
    }
}

__glsl_version(430)
void InterlockedXor(__ref  int dest,  int value, out  int original_value)
{
     __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedXor";
    case glsl: __intrinsic_asm "($2 = atomicXor($0, $1))";
    case cuda: __intrinsic_asm "(*$2 = atomicXor($0, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$int = OpAtomicXor &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

__glsl_version(430)
void InterlockedXor(__ref uint dest, uint value, out uint original_value)
{
     __target_switch
    {
    case hlsl: __intrinsic_asm "InterlockedXor";
    case glsl: __intrinsic_asm "($2 = atomicXor($0, $1))";
    case cuda: __intrinsic_asm "(*$2 = (uint)atomicXor((int*)$0, $1))";
    case spirv:
        spirv_asm
        {
            %original:$$uint = OpAtomicXor &dest Device None $value;
            OpStore &original_value %original
        };
    }
}

// Is floating-point value finite?

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
[__readNone]
bool isfinite(T x)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "isfinite";
    case cuda:
    case cpp:
        __intrinsic_asm "$P_isfinite($0)";
    default:
        return !(isinf(x) || isnan(x));
    }
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[__readNone]
vector<bool, N> isfinite(vector<T, N> x)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "isfinite";
    default:
        VECTOR_MAP_UNARY(bool, N, isfinite, x);
    }
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<bool, N, M> isfinite(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(bool, N, M, isfinite, x);
}

// Is floating-point value infinite?
__generic<T : __BuiltinFloatingPointType>
[__readNone]
bool isinf(T x)
{
    __target_switch
    {
    case hlsl:
    case glsl:
        __intrinsic_asm "isinf";
    case cuda:
    case cpp:
        __intrinsic_asm "$P_isinf($0)";
    case spirv:
        return spirv_asm { result:$$bool = OpIsInf $x};
    }
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[__readNone]
vector<bool, N> isinf(vector<T, N> x)
{
    __target_switch
    {
    case hlsl:
    case glsl:
        __intrinsic_asm "isinf";
    case spirv:
        return spirv_asm { result:$$vector<bool,N> = OpIsInf $x};
    default:
        VECTOR_MAP_UNARY(bool, N, isinf, x);
    }
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<bool, N, M> isinf(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(bool, N, M, isinf, x);
}

// Is floating-point value not-a-number?
__generic<T : __BuiltinFloatingPointType>
[__readNone]
bool isnan(T x)
{
    __target_switch
    {
    case hlsl:
    case glsl:
        __intrinsic_asm "isnan";
    case cuda:
    case cpp:
        __intrinsic_asm "$P_isnan($0)";
    case spirv:
        return spirv_asm { result:$$bool = OpIsNan $x};
    }
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[__readNone]
vector<bool, N> isnan(vector<T, N> x)
{
    __target_switch
    {
    case hlsl:
    case glsl:
        __intrinsic_asm "isnan";
    case spirv:
        return spirv_asm { result:$$vector<bool, N> = OpIsNan $x};
    default:
        VECTOR_MAP_UNARY(bool, N, isnan, x);
    }
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<bool, N, M> isnan(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(bool, N, M, isnan, x);
}

// Construct float from mantissa and exponent

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
[__readNone]
T ldexp(T x, T exp)
{
    return x * exp2(exp);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
vector<T, N> ldexp(vector<T, N> x, vector<T, N> exp)
{
    return x * exp2(exp);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> ldexp(matrix<T, N, M> x, matrix<T, N, M> exp)
{
    MATRIX_MAP_BINARY(T, N, M, ldexp, x, exp);
}

// Vector length
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Length _0")
[__readNone]
T length(vector<T, N> x)
{
    return sqrt(dot(x, x));
}

// Scalar float length
__generic<T : __BuiltinFloatingPointType>
T length(T x)
{
    return abs(x);
}

// Linear interpolation
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, mix)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FMix _0 _1 _2")
[__readNone]
T lerp(T x, T y, T s)
{
    return x * (T(1.0f) - s) + y * s;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, mix)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 FMix _0 _1 _2")
[__readNone]
vector<T, N> lerp(vector<T, N> x, vector<T, N> y, vector<T, N> s)
{
    return x * (T(1.0f) - s) + y * s;
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> lerp(matrix<T,N,M> x, matrix<T,N,M> y, matrix<T,N,M> s)
{
    MATRIX_MAP_TRINARY(T, N, M, lerp, x, y, s);
}

// Legacy lighting function (obsolete)
__target_intrinsic(hlsl)
[__readNone]
float4 lit(float n_dot_l, float n_dot_h, float m)
{
    let ambient = 1.0f;
    let diffuse = max(n_dot_l, 0.0f);
    let specular = step(0.0f, n_dot_l) * max(pow(n_dot_h, m), 0.0f);
    return float4(ambient, diffuse, specular, 1.0f);
}

// Base-e logarithm
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_log($0)")
__target_intrinsic(cpp, "$P_log($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Log _0")
[__readNone]
T log(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Log _0")
[__readNone]
vector<T, N> log(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, log, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> log(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, log, x);
}

// Base-10 logarithm
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "(log( $0 ) * $S0( 0.43429448190325182765112891891661) )" )
__target_intrinsic(cuda, "$P_log10($0)")
__target_intrinsic(cpp, "$P_log10($0)")
__target_intrinsic(spirv, "%baseElog = OpExtInst resultType resultId glsl450 Log _0; OpFMul resultType resultId %baseElog const(_p,0.43429448190325182765112891891661)")
[__readNone]
T log10(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "(log( $0 ) * $S0(0.43429448190325182765112891891661) )" )
__target_intrinsic(spirv, "%baseElog = OpExtInst resultType resultId glsl450 Log _0; OpVectorTimesScalar resultType resultId %baseElog const(_p,0.43429448190325182765112891891661)")
[__readNone]
vector<T,N> log10(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, log10, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> log10(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, log10, x);
}

// Base-2 logarithm
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_log2($0)")
__target_intrinsic(cpp, "$P_log2($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Log2 _0")
[__readNone]
T log2(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Log2 _0")
[__readNone]
vector<T,N> log2(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, log2, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> log2(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, log2, x);
}

// multiply-add

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fma)
__target_intrinsic(cuda, "$P_fma($0, $1, $2)")
__target_intrinsic(cpp, "$P_fma($0, $1, $2)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fma _0 _1 _2")
[__readNone]
T mad(T mvalue, T avalue, T bvalue);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fma)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fma _0 _1 _2")
[__readNone]
vector<T, N> mad(vector<T, N> mvalue, vector<T, N> avalue, vector<T, N> bvalue)
{
    VECTOR_MAP_TRINARY(T, N, mad, mvalue, avalue, bvalue);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> mad(matrix<T, N, M> mvalue, matrix<T, N, M> avalue, matrix<T, N, M> bvalue)
{
    MATRIX_MAP_TRINARY(T, N, M, mad, mvalue, avalue, bvalue);
}

__generic<T : __BuiltinIntegerType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fma)
__target_intrinsic(cuda, "$P_fma($0, $1, $2)")
__target_intrinsic(cpp, "$P_fma($0, $1, $2)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fma _0 _1 _2")
[__readNone]
T mad(T mvalue, T avalue, T bvalue);

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fma)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Fma _0 _1 _2")
[__readNone]
vector<T, N> mad(vector<T, N> mvalue, vector<T, N> avalue, vector<T, N> bvalue)
{
    VECTOR_MAP_TRINARY(T, N, mad, mvalue, avalue, bvalue);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> mad(matrix<T, N, M> mvalue, matrix<T, N, M> avalue, matrix<T, N, M> bvalue)
{
    MATRIX_MAP_TRINARY(T, N, M, mad, mvalue, avalue, bvalue);
}


// maximum
__generic<T : __BuiltinIntegerType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_max($0, $1)")
__target_intrinsic(cpp, "$P_max($0, $1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMax, UMax, SMax) _0 _1")
[__readNone]
T max(T x, T y);
// Note: a stdlib implementation of `max` (or `min`) will require splitting
// floating-point and integer cases apart, because the floating-point
// version needs to correctly handle the case where one of the inputs
// is not-a-number.

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMax, UMax, SMax) _0 _1")
[__readNone]
vector<T, N> max(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, max, x, y);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> max(matrix<T, N, M> x, matrix<T, N, M> y)
{
    MATRIX_MAP_BINARY(T, N, M, max, x, y);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_max($0, $1)")
__target_intrinsic(cpp, "$P_max($0, $1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMax, UMax, SMax) _0 _1")
[__readNone]
T max(T x, T y);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMax, UMax, SMax) _0 _1")
[__readNone]
vector<T, N> max(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, max, x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> max(matrix<T, N, M> x, matrix<T, N, M> y)
{
    MATRIX_MAP_BINARY(T, N, M, max, x, y);
}

// minimum
__generic<T : __BuiltinIntegerType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_min($0, $1)")
__target_intrinsic(cpp, "$P_min($0, $1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMin, UMin, SMin) _0 _1")
[__readNone]
T min(T x, T y);

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMin, UMin, SMin) _0 _1")
[__readNone]
vector<T,N> min(vector<T,N> x, vector<T,N> y)
{
    VECTOR_MAP_BINARY(T, N, min, x, y);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> min(matrix<T,N,M> x, matrix<T,N,M> y)
{
    MATRIX_MAP_BINARY(T, N, M, min, x, y);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_min($0, $1)")
__target_intrinsic(cpp, "$P_min($0, $1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMin, UMin, SMin) _0 _1")
[__readNone]
T min(T x, T y);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 fus(FMin, UMin, SMin) _0 _1")
[__readNone]
vector<T,N> min(vector<T,N> x, vector<T,N> y)
{
    VECTOR_MAP_BINARY(T, N, min, x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> min(matrix<T,N,M> x, matrix<T,N,M> y)
{
    MATRIX_MAP_BINARY(T, N, M, min, x, y);
}

// split into integer and fractional parts (both with same sign)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Modf _0 _1")
[__readNone]
T modf(T x, out T ip);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
[__readNone]
vector<T,N> modf(vector<T,N> x, out vector<T,N> ip)
{
    VECTOR_MAP_BINARY(T, N, modf, x, ip);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int, let L : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> modf(matrix<T,N,M> x, out matrix<T,N,M,L> ip)
{
    MATRIX_MAP_BINARY(T, N, M, modf, x, ip);
}

// msad4 (whatever that is)
__target_intrinsic(hlsl)
[__readNone]
uint4 msad4(uint reference, uint2 source, uint4 accum)
{
    int4 bytesRef = (reference >> uint4(24, 16, 8, 0)) & 0xFF;
    int4 bytesX   = (source.x  >> uint4(24, 16, 8, 0)) & 0xFF;
    int4 bytesY   = (source.y  >> uint4(24, 16, 8, 0)) & 0xFF;

    uint4 mask = select(bytesRef == 0, 0, 0xFFFFFFFFu);

    uint4 result = accum;
    result += mask.x & abs(bytesRef - int4(bytesX.x,           bytesY.y, bytesY.z, bytesY.w));
    result += mask.y & abs(bytesRef - int4(bytesX.x, bytesX.y,           bytesY.z, bytesY.w));
    result += mask.z & abs(bytesRef - int4(bytesX.x, bytesX.y, bytesX.z,           bytesY.w));
    result += mask.w & abs(bytesRef - int4(bytesX.x, bytesX.y, bytesX.z, bytesX.w));
    return result;
}

// General inner products

// scalar-scalar
__generic<T : __BuiltinArithmeticType>
__intrinsic_op($(kIROp_Mul))
[__readNone]
T mul(T x, T y);

// scalar-vector and vector-scalar
__generic<T : __BuiltinArithmeticType, let N : int>
__intrinsic_op($(kIROp_Mul))
[__readNone]
vector<T, N> mul(vector<T, N> x, T y);

__generic<T : __BuiltinArithmeticType, let N : int>
__intrinsic_op($(kIROp_Mul))
[__readNone]
vector<T, N> mul(T x, vector<T, N> y);

// scalar-matrix and matrix-scalar
__generic<T : __BuiltinArithmeticType, let N : int, let M :int>
__intrinsic_op($(kIROp_Mul))
[__readNone]
matrix<T, N, M> mul(matrix<T, N, M> x, T y);

__generic<T : __BuiltinArithmeticType, let N : int, let M :int>
__intrinsic_op($(kIROp_Mul))
[__readNone]
matrix<T, N, M> mul(T x, matrix<T, N, M> y);

// vector-vector (dot product)
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "dot")
[__readNone]
T mul(vector<T, N> x, vector<T, N> y)
{
    return dot(x, y);
}
__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
T mul(vector<T, N> x, vector<T, N> y)
{
    return dot(x, y);
}

// vector-matrix
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
__target_intrinsic(spirv, "OpMatrixTimesVector resultType resultId _1 _0")
[__readNone]
vector<T, M> mul(vector<T, N> left, matrix<T, N, M> right)
{
    vector<T,M> result;
    for( int j = 0; j < M; ++j )
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum += left[i] * right[i][j];
        }
        result[j] = sum;
    }
    return result;
}
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
[__readNone]
vector<T, M> mul(vector<T, N> left, matrix<T, N, M> right)
{
    vector<T,M> result;
    for( int j = 0; j < M; ++j )
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum += left[i] * right[i][j];
        }
        result[j] = sum;
    }
    return result;
}
__generic<T : __BuiltinLogicalType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
[__readNone]
vector<T, M> mul(vector<T, N> left, matrix<T, N, M> right)
{
    vector<T,M> result;
    for( int j = 0; j < M; ++j )
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum |= left[i] & right[i][j];
        }
        result[j] = sum;
    }
    return result;
}

// matrix-vector
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
__target_intrinsic(spirv, "OpVectorTimesMatrix resultType resultId _1 _0")
[__readNone]
vector<T,N> mul(matrix<T,N,M> left, vector<T,M> right)
{
    vector<T,N> result;
    for( int i = 0; i < N; ++i )
    {
        T sum = T(0);
        for( int j = 0; j < M; ++j )
        {
            sum += left[i][j] * right[j];
        }
        result[i] = sum;
    }
    return result;
}
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
[__readNone]
vector<T,N> mul(matrix<T,N,M> left, vector<T,M> right)
{
    vector<T,N> result;
    for( int i = 0; i < N; ++i )
    {
        T sum = T(0);
        for( int j = 0; j < M; ++j )
        {
            sum += left[i][j] * right[j];
        }
        result[i] = sum;
    }
    return result;
}
__generic<T : __BuiltinLogicalType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
[__readNone]
vector<T,N> mul(matrix<T,N,M> left, vector<T,M> right)
{
    vector<T,N> result;
    for( int i = 0; i < N; ++i )
    {
        T sum = T(0);
        for( int j = 0; j < M; ++j )
        {
            sum |= left[i][j] & right[j];
        }
        result[i] = sum;
    }
    return result;
}

// matrix-matrix
__generic<T : __BuiltinFloatingPointType, let R : int, let N : int, let C : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
__target_intrinsic(spirv, "OpMatrixTimesMatrix resultType resultId _1 _0")
[__readNone]
matrix<T,R,C> mul(matrix<T,R,N> right, matrix<T,N,C> left)
{
    matrix<T,R,C> result;
    for( int r = 0; r < R; ++r)
    for( int c = 0; c < C; ++c)
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum += left[r][i] * right[i][c];
        }
        result[r][c] = sum;
    }
    return result;
}
__generic<T : __BuiltinIntegerType, let R : int, let N : int, let C : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
[__readNone]
matrix<T,R,C> mul(matrix<T,R,N> right, matrix<T,N,C> left)
{
    matrix<T,R,C> result;
    for( int r = 0; r < R; ++r)
    for( int c = 0; c < C; ++c)
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum += left[r][i] * right[i][c];
        }
        result[r][c] = sum;
    }
    return result;
}
__generic<T : __BuiltinLogicalType, let R : int, let N : int, let C : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
[__readNone]
matrix<T,R,C> mul(matrix<T,R,N> right, matrix<T,N,C> left)
{
    matrix<T,R,C> result;
    for( int r = 0; r < R; ++r)
    for( int c = 0; c < C; ++c)
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum |= left[r][i] & right[i][c];
        }
        result[r][c] = sum;
    }
    return result;
}

// noise (deprecated)

[__readNone]
[deprecated("Always returns 0")]
float noise(float x)
{
    return 0;
}

[__readNone]
[deprecated("Always returns 0")]
__generic<let N : int> float noise(vector<float, N> x)
{
    return 0;
}

/// Indicate that an index may be non-uniform at execution time.
///
/// Shader Model 5.1 and 6.x introduce support for dynamic indexing
/// of arrays of resources, but place the restriction that *by default*
/// the implementation can assume that any value used as an index into
/// such arrays will be dynamically uniform across an entire `Draw` or `Dispatch`
/// (when using instancing, the value must be uniform across all instances;
/// it does not seem that the restriction extends to draws within a multi-draw).
///
/// In order to indicate to the implementation that it cannot make the
/// uniformity assumption, a shader programmer is required to pass the index
/// to the `NonUniformResourceIndex` function before using it as an index.
/// The function superficially acts like an identity function.
///
/// Note: a future version of Slang may take responsibility for inserting calls
/// to this function as necessary in output code, rather than make this
/// the user's responsibility, so that the default behavior of the language
/// is more semantically "correct."
__glsl_extension(GL_EXT_nonuniform_qualifier)
[__readNone]
[ForceInline]
uint NonUniformResourceIndex(uint index)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "NonUniformResourceIndex";
    case glsl:
        __intrinsic_asm "nonuniformEXT";
    case spirv:
        spirv_asm
        {
            OpCapability ShaderNonUniform;
            OpDecorate $index NonUniform;
        };
        return index;
    default:
        return index;
    }
}

__glsl_extension(GL_EXT_nonuniform_qualifier)
[__readNone]
int NonUniformResourceIndex(int index)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "NonUniformResourceIndex";
    case glsl:
        __intrinsic_asm "nonuniformEXT";
    case spirv:
        spirv_asm
        {
            OpCapability ShaderNonUniform;
            OpDecorate $index NonUniform;
        };
        return index;
    default:
        return index;
    }
}

/// HLSL allows NonUniformResourceIndex around non int/uint types.
/// It's effect is presumably to ignore it, which the following implementation does.
/// We should also look to add a warning for this scenario.
[__unsafeForceInlineEarly]
[deprecated("NonUniformResourceIndex on a type other than uint/int is deprecated and has no effect")]
T NonUniformResourceIndex<T>(T value) { return value; }

// Normalize a vector
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Normalize _0")
[__readNone]
vector<T,N> normalize(vector<T,N> x)
{
    return x / length(x);
}

// Raise to a power
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_pow($0, $1)")
__target_intrinsic(cpp, "$P_pow($0, $1)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Pow _0 _1")
[__readNone]
T pow(T x, T y);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Pow _0 _1")
[__readNone]
vector<T, N> pow(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, pow, x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> pow(matrix<T,N,M> x, matrix<T,N,M> y)
{
    MATRIX_MAP_BINARY(T, N, M, pow, x, y);
}

// Output message
// TODO: add check to ensure format is const literal.

${{{{
for (int argCount = 0; argCount < 12; argCount++)
{
    StringBuilder paramList;
    StringBuilder argList;
    StringBuilder spirvArgList;
    StringBuilder genericParamList;
    if (argCount > 0)
        genericParamList << "<";
    for (int i = 0; i < argCount; i++)
    {
        if (i > 0)
            genericParamList << ", ";
        genericParamList << "T" << i;

        paramList << ", T" << i << " v" << i;
        argList << ", $" << i+1;
        spirvArgList << " $v" << i;
    }
    if (argCount > 0)
        genericParamList << ">";
    auto params = paramList.toString();
    auto args = argList.toString();
    auto spirvArgs = spirvArgList.toString();
}}}}
__glsl_extension(GL_EXT_debug_printf)
void printf$(genericParamList.toString())(NativeString format $(paramList))
{
    __target_switch
    {
    case hlsl:
    case cpp:
    case cuda:
        __intrinsic_asm "printf";
    case glsl:
        __intrinsic_asm "debugPrintfEXT($0 $(argList))";
    case spirv:
        spirv_asm {
          OpExtension "SPV_KHR_non_semantic_info";
          result:$$void = OpExtInst debugPrintf 1 $format $(spirvArgs);
        };
    }
}
${{{{
}
}}}}

// Tessellation factor fixup routines

void Process2DQuadTessFactorsAvg(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void Process2DQuadTessFactorsMax(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void Process2DQuadTessFactorsMin(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessIsolineTessFactors(
    in  float RawDetailFactor,
    in  float RawDensityFactor,
    out float RoundedDetailFactor,
    out float RoundedDensityFactor);

void ProcessQuadTessFactorsAvg(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessQuadTessFactorsMax(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessQuadTessFactorsMin(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessTriTessFactorsAvg(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactor,
    out float UnroundedInsideTessFactor);

void ProcessTriTessFactorsMax(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactor,
    out float UnroundedInsideTessFactor);

void ProcessTriTessFactorsMin(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactors,
    out float UnroundedInsideTessFactors);

// Degrees to radians
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Radians _0")
[__readNone]
T radians(T x)
{
    return x * (T.getPi() / T(180.0f));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Radians _0")
[__readNone]
vector<T, N> radians(vector<T, N> x)
{
    return x * (T.getPi() / T(180.0f));
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> radians(matrix<T, N, M> x)
{
    return x * (T.getPi() / T(180.0f));
}

// Approximate reciprocal
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
[__readNone]
T rcp(T x)
{
    return T(1.0) / x;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
vector<T, N> rcp(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, rcp, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> rcp(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, rcp, x);
}

// Reflect incident vector across plane with given normal
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Reflect _0 _1")
[__readNone]
vector<T,N> reflect(vector<T,N> i, vector<T,N> n)
{
    return i - T(2) * dot(n,i) * n;
}

// Refract incident vector given surface normal and index of refraction
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Refract _0 _1 _2")
[__readNone]
vector<T,N> refract(vector<T,N> i, vector<T,N> n, T eta)
{
    let dotNI = dot(n,i);
    let k = T(1) - eta*eta*(T(1) - dotNI * dotNI);
    if(k < T(0)) return vector<T,N>(T(0));
    return eta * i - (eta * dotNI + sqrt(k)) * n;
}

// Reverse order of bits
[__readNone]
uint reversebits(uint value)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "reversebits";
    case glsl:
        __intrinsic_asm "bitfieldReverse";
    case cuda:
    case cpp:
        __intrinsic_asm "$P_reversebits($0)";
    case spirv:
        return spirv_asm {OpBitReverse $$uint result $value};
    }
}

__target_intrinsic(glsl, "bitfieldReverse")
__generic<let N : int>
[__readNone]
vector<uint, N> reversebits(vector<uint, N> value)
{
    __target_switch
    {
    default:
        VECTOR_MAP_UNARY(uint, N, reversebits, value);
    case glsl:
        __intrinsic_asm "bitfieldReverse";
    case spirv:
        return spirv_asm {OpBitReverse $$vector<uint, N> result $value};
    }
}

// Round-to-nearest
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_round($0)")
__target_intrinsic(cpp, "$P_round($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Round _0")
[__readNone]
T round(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Round _0")
[__readNone]
vector<T, N> round(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, round, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> round(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, round, x);
}

// Reciprocal of square root
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "inversesqrt($0)")
__target_intrinsic(cuda, "$P_rsqrt($0)")
__target_intrinsic(cpp, "$P_rsqrt($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 InverseSqrt _0")
[__readNone]
T rsqrt(T x)
{
    return T(1.0) / sqrt(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "inversesqrt($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 InverseSqrt _0")
[__readNone]
vector<T, N> rsqrt(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, rsqrt, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> rsqrt(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, rsqrt, x);
}

// Clamp value to [0,1] range

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
[__readNone]
T saturate(T x)
{
    return clamp<T>(x, T(0), T(1));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
vector<T,N> saturate(vector<T,N> x)
{
    return clamp<T,N>(x,
        vector<T,N>(T(0)),
        vector<T,N>(T(1)));
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> saturate(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, saturate, x);
}

__generic<T:__BuiltinArithmeticType, U:__BuiltinArithmeticType>
__intrinsic_op($(kIROp_IntCast))
T __int_cast(U val);

__generic<T:__BuiltinArithmeticType, U:__BuiltinArithmeticType, let N : int>
__intrinsic_op($(kIROp_IntCast))
vector<T,N> __int_cast(vector<U,N> val);

// Extract sign of value
__generic<T : __BuiltinSignedArithmeticType>
[__readNone]
int sign(T x)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "sign";
    case glsl: __intrinsic_asm "int(sign($0))";
    case cuda:
    case cpp:
        __intrinsic_asm "$P_sign($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm
            {
                %fsign:$$T = OpExtInst glsl450 FSign $x;
                result:$$int = OpConvertFToS %fsign
            };
        else
            return __int_cast<int>(spirv_asm {OpExtInst $$T result glsl450 SSign $x});
    }
}

__generic<T : __BuiltinSignedArithmeticType, let N : int>
[__readNone]
vector<int, N> sign(vector<T, N> x)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "sign";
    case glsl: __intrinsic_asm "ivec$N0(sign($0))";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm
            {
                %fsign:$$vector<T, N> = OpExtInst glsl450 FSign $x;
                result:$$vector<int, N> = OpConvertFToS %fsign
            };
        else
            return __int_cast<int>(spirv_asm {OpExtInst $$vector<T,N> result glsl450 SSign $x});
    default:
        VECTOR_MAP_UNARY(int, N, sign, x);
    }
}

__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<int, N, M> sign(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(int, N, M, sign, x);
}


// Sine

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_sin($0)")
__target_intrinsic(cpp, "$P_sin($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Sin _0")
[__readNone]
T sin(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Sin _0")
[__readNone]
vector<T, N> sin(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, sin, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> sin(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, sin, x);
}

// Sine and cosine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "$P_sincos($0, $1, $2)")
[__readNone]
void sincos(T x, out T s, out T c)
{
    s = sin(x);
    c = cos(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
[__readNone]
void sincos(vector<T,N> x, out vector<T,N> s, out vector<T,N> c)
{
    s = sin(x);
    c = cos(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int, let L1: int, let L2 : int>
__target_intrinsic(hlsl)
[__readNone]
void sincos(matrix<T,N,M> x, out matrix<T,N,M,L1> s, out matrix<T,N,M,L2> c)
{
    s = sin(x);
    c = cos(x);
}

// Hyperbolic Sine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_sinh($0)")
__target_intrinsic(cpp, "$P_sinh($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Sinh _0")
[__readNone]
T sinh(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Sinh _0")
[__readNone]
vector<T, N> sinh(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, sinh, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> sinh(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, sinh, x);
}

// Smooth step (Hermite interpolation)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 SmoothStep _0 _1 _2")
[__readNone]
T smoothstep(T min, T max, T x)
{
    let t = saturate((x - min) / (max - min));
    return t * t * (T(3.0f) - (t + t));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 SmoothStep _0 _1 _2")
[__readNone]
vector<T, N> smoothstep(vector<T, N> min, vector<T, N> max, vector<T, N> x)
{
    VECTOR_MAP_TRINARY(T, N, smoothstep, min, max, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> smoothstep(matrix<T, N, M> min, matrix<T, N, M> max, matrix<T, N, M> x)
{
    MATRIX_MAP_TRINARY(T, N, M, smoothstep, min, max, x);
}

// Square root
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_sqrt($0)")
__target_intrinsic(cpp, "$P_sqrt($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Sqrt _0")
[__readNone]
T sqrt(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Sqrt _0")
[__readNone]
vector<T, N> sqrt(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, sqrt, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> sqrt(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, sqrt, x);
}

// Step function
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Step _0 _1")
[__readNone]
T step(T y, T x)
{
    return x < y ? T(0.0f) : T(1.0f);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Step _0 _1")
[__readNone]
vector<T,N> step(vector<T,N> y, vector<T,N> x)
{
    VECTOR_MAP_BINARY(T, N, step, y, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> step(matrix<T, N, M> y, matrix<T, N, M> x)
{
    MATRIX_MAP_BINARY(T, N, M, step, y, x);
}

// Tangent
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_tan($0)")
__target_intrinsic(cpp, "$P_tan($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Tan _0")
[__readNone]
T tan(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Tan _0")
[__readNone]
vector<T, N> tan(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, tan, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> tan(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, tan, x);
}

// Hyperbolic tangent
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_tanh($0)")
__target_intrinsic(cpp, "$P_tanh($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Tanh _0")
[__readNone]
T tanh(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Tanh _0")
[__readNone]
vector<T,N> tanh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, tanh, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T,N,M> tanh(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, tanh, x);
}

// Matrix transpose
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpTranspose resultType resultId _0")
[__readNone]
[PreferRecompute]
matrix<T, M, N> transpose(matrix<T, N, M> x)
{
    matrix<T,M,N> result;
    for(int r = 0; r < M; ++r)
        for(int c = 0; c < N; ++c)
            result[r][c] = x[c][r];
    return result;
}
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpTranspose resultType resultId _0")
[__readNone]
[PreferRecompute]
matrix<T, M, N> transpose(matrix<T, N, M> x)
{
    matrix<T, M, N> result;
    for (int r = 0; r < M; ++r)
        for (int c = 0; c < N; ++c)
            result[r][c] = x[c][r];
    return result;
}
__generic<T : __BuiltinLogicalType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpTranspose resultType resultId _0")
[__readNone]
[PreferRecompute]
matrix<T, M, N> transpose(matrix<T, N, M> x)
{
    matrix<T, M, N> result;
    for (int r = 0; r < M; ++r)
        for (int c = 0; c < N; ++c)
            result[r][c] = x[c][r];
    return result;
}

// Truncate to integer
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_trunc($0)")
__target_intrinsic(cpp, "$P_trunc($0)")
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Trunc _0")
[__readNone]
T trunc(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv, "OpExtInst resultType resultId glsl450 Trunc _0")
[__readNone]
vector<T, N> trunc(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, trunc, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
[__readNone]
matrix<T, N, M> trunc(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, trunc, x);
}

// Slang Specific 'Mask' Wave Intrinsics

typedef uint WaveMask;

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
WaveMask WaveGetConvergedMask()
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallot(true).x";
    case hlsl:
        __intrinsic_asm "WaveActiveBallot(true).x";
    case cuda:
        __intrinsic_asm "__activemask()";
    case spirv:
        let _true = true;
        return (spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBallot $$uint4 result Subgroup $_true
        }).x;
    }
}

__intrinsic_op($(kIROp_WaveGetActiveMask))
WaveMask __WaveGetActiveMask();

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
WaveMask WaveGetActiveMask()
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallot(true).x";
    case hlsl:
        __intrinsic_asm "WaveActiveBallot(true).x";
    case spirv:
        let _true = true;
        return (spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBallot $$uint4 result Subgroup $_true
        }).x;
    default:
        return __WaveGetActiveMask();
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
bool WaveMaskIsFirstLane(WaveMask mask)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupElect()";
    case cuda:
        __intrinsic_asm "(($0 & -$0) == (WarpMask(1) << _getLaneId()))";
    case hlsl:
        __intrinsic_asm "WaveIsFirstLane()";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformElect $$bool result Subgroup
        };
    default:
        return false;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
bool WaveMaskAllTrue(WaveMask mask, bool condition)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAll($1)";
    case cuda:
        __intrinsic_asm "(__all_sync($0, $1) != 0)";
    case hlsl:
        __intrinsic_asm "WaveActiveAllTrue($1)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformAll $$bool result Subgroup $condition
        };
    default:
        return false;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
bool WaveMaskAnyTrue(WaveMask mask, bool condition)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAny($1)";
    case cuda:
        __intrinsic_asm "(__any_sync($0, $1) != 0)";
    case hlsl:
        __intrinsic_asm "WaveActiveAnyTrue($1)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformAny $$bool result Subgroup $condition
        };
    default:
        return false;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
WaveMask WaveMaskBallot(WaveMask mask, bool condition)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallot($1).x";
    case cuda:
        __intrinsic_asm "__ballot_sync($0, $1)";
    case hlsl:
        __intrinsic_asm "WaveActiveBallot($1)";
    case spirv:
        return (spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBallot $$uint4 result Subgroup $condition
        }).x;
    default:
        return 0;
    }
}

uint WaveMaskCountBits(WaveMask mask, bool value)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm  "__popc(__ballot_sync($0, $1))";
    case hlsl:
        __intrinsic_asm  "WaveActiveCountBits($1)";
    default:
        return _WaveCountBits(WaveActiveBallot(value));
    }
}

// Waits until all warp lanes named in mask have executed a WaveMaskSharedSync (with the same mask)
// before resuming execution. Guarantees memory ordering in shared memory among threads participating
// in the barrier.
//
// The CUDA intrinsic says it orders *all* memory accesses, which appears to match most closely subgroupBarrier.
//
// TODO(JS):
// For HLSL it's not clear what to do. There is no explicit mechanism to 'reconverge' threads. In the docs it describes
// behavior as
// "These intrinsics are dependent on active lanes and therefore flow control. In the model of this document, implementations
// must enforce that the number of active lanes exactly corresponds to the programmer’s view of flow control."
//
// It seems this can only mean the active threads are the "threads the program flow would lead to". This implies a lockstep
// "straight SIMD" style interpretation. That being the case this op on HLSL is just a memory barrier without any Sync.

void AllMemoryBarrierWithWaveMaskSync(WaveMask mask)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__syncwarp($0)";
    case hlsl:
        __intrinsic_asm "AllMemoryBarrier()";
    case glsl:
    case spirv:
        __subgroupBarrier();
        return;
    }
}

// On GLSL, it appears we can't use subgroupMemoryBarrierShared, because it only implies a memory ordering, it does not
// imply convergence. For subgroupBarrier we have from the docs..
// "The function subgroupBarrier() enforces that all active invocations within a subgroup must execute this function before any
// are allowed to continue their execution"
// TODO(JS):
// It's not entirely clear what to do here on HLSL.
// Reading the dxc wiki (https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics), we have statements like:
//    ... these intrinsics enable the elimination of barrier constructs when the scope of synchronization is within the width of the SIMD processor.
//    Wave: A set of lanes executed simultaneously in the processor. No explicit barriers are required to guarantee that they execute in parallel.
// Which seems to imply at least some memory barriers like Shared might not be needed.
//
// The barrier is left here though, because not only is the barrier make writes before the barrier across the wave appear to others afterwards, it's
// also there to inform the compiler on what order reads and writes can take place. This might seem to be silly because of the 'Active' lanes
// aspect of HLSL seems to make everything in lock step - but that's not quite so, it only has to apparently be that way as far as the programmers
// model appears - divergence could perhaps potentially still happen.

void GroupMemoryBarrierWithWaveMaskSync(WaveMask mask)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__syncwarp($0)";
    case hlsl:
        __intrinsic_asm "GroupMemoryBarrier()";
    case glsl:
    case spirv:
        __subgroupBarrier();
        return;
    }
}

void AllMemoryBarrierWithWaveSync()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__syncwarp()";
    case hlsl:
        __intrinsic_asm "AllMemoryBarrier()";
    case glsl:
    case spirv:
        __subgroupBarrier();
        return;
    }
}

void GroupMemoryBarrierWithWaveSync()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__syncwarp()";
    case hlsl:
        __intrinsic_asm "GroupMemoryBarrier()";
    case glsl:
    case spirv:
        __subgroupBarrier();
        return;
    }
}

// NOTE! WaveMaskBroadcastLaneAt is *NOT* standard HLSL
// It is provided as access to subgroupBroadcast which can only take a
// constexpr laneId.
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
// Versions SPIR-V greater than 1.4 loosen this restriction, and allow 'dynamic uniform' index
// If that's the behavior required then client code should use WaveReadLaneAt which works this way.

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
T WaveMaskBroadcastLaneAt(WaveMask mask, T value, constexpr int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcast($1, $2)";
    case cuda: __intrinsic_asm "__shfl_sync($0, $1, $2)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt($1, $2)";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBroadcast $$T result Subgroup $value $ulane;
        };
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
vector<T,N> WaveMaskBroadcastLaneAt(WaveMask mask, vector<T,N> value, constexpr int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcast($1, $2)";
    case cuda: __intrinsic_asm "_waveShuffleMultiple($0, $1, $2)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt($1, $2)";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBroadcast $$vector<T,N> result Subgroup $value $ulane;
        };
    }
}
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
matrix<T,N,M> WaveMaskBroadcastLaneAt(WaveMask mask, matrix<T,N,M> value, constexpr int lane);

// TODO(JS): If it can be determines that the `laneId` is constExpr, then subgroupBroadcast
// could be used on GLSL. For now we just use subgroupShuffle
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
T WaveMaskReadLaneAt(WaveMask mask, T value, int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupShuffle($1, $2)";
    case cuda: __intrinsic_asm "__shfl_sync($0, $1, $2)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt($1, $2)";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {
            OpCapability GroupNonUniformShuffle;
            OpGroupNonUniformShuffle $$T result Subgroup $value $ulane;
        };
    }
}
__generic<T : __BuiltinType, let N : int>
__spirv_version(1.3)__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
vector<T,N> WaveMaskReadLaneAt(WaveMask mask, vector<T,N> value, int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupShuffle($1, $2)";
    case cuda: __intrinsic_asm "_waveShuffleMultiple($0, $1, $2)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt($1, $2)";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {
            OpCapability GroupNonUniformShuffle;
            OpGroupNonUniformShuffle $$vector<T,N> result Subgroup $value $ulane;
        };
    }
}
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
matrix<T,N,M> WaveMaskReadLaneAt(WaveMask mask, matrix<T,N,M> value, int lane);

// NOTE! WaveMaskShuffle is a NON STANDARD HLSL intrinsic! It will map to WaveReadLaneAt on HLSL
// which means it will only work on hardware which allows arbitrary laneIds which is not true
// in general because it breaks the HLSL standard, which requires it's 'dynamically uniform' across the Wave.
__generic<T : __BuiltinType>
[__unsafeForceInlineEarly]
T WaveMaskShuffle(WaveMask mask, T value, int lane)
{
    return WaveMaskReadLaneAt(mask, value, lane);
}
__generic<T : __BuiltinType, let N : int>
[__unsafeForceInlineEarly]
vector<T,N> WaveMaskShuffle(WaveMask mask, vector<T,N> value, int lane)
{
    return WaveMaskReadLaneAt(mask, value, lane);
}
__generic<T : __BuiltinType, let N : int, let M : int>
[__unsafeForceInlineEarly]
matrix<T,N,M> WaveMaskShuffle(WaveMask mask, matrix<T,N,M> value, int lane)
{
    return WaveMaskReadLaneAt(mask, value, lane);
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
uint WaveMaskPrefixCountBits(WaveMask mask, bool value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBallotExclusiveBitCount(subgroupBallot($1))";
    case cuda: __intrinsic_asm "__popc(__ballot_sync($0, $1)  & _getLaneLtMask())";
    case hlsl: __intrinsic_asm "WavePrefixCountBits($1)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            %mask:$$uint4 = OpGroupNonUniformBallot Subgroup $value;
            OpGroupNonUniformBallotBitCount $$uint result Subgroup 2 %mask
        };
    }
}

// Across lane ops

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskBitAnd(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupAnd($1)";
    case cuda: __intrinsic_asm "_waveAnd($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveBitAnd($1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformArithmetic;
            OpGroupNonUniformBitwiseAnd $$T result Subgroup 0 $expr
        };
    }
}

__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskBitAnd(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupAnd($1)";
    case cuda: __intrinsic_asm "_waveAndMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveBitAnd($1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformArithmetic;
            OpGroupNonUniformBitwiseAnd $$vector<T,N> result Subgroup 0 $expr
        };
    }
}
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveAndMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitAnd($1)")
matrix<T,N,M> WaveMaskBitAnd(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskBitOr(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupOr($1)";
    case cuda: __intrinsic_asm "_waveOr($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveBitOr($1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformArithmetic;
            OpGroupNonUniformBitwiseOr $$T result Subgroup 0 $expr
        };
    }
}
__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskBitOr(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupOr($1)";
    case cuda: __intrinsic_asm "_waveOrMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveBitOr($1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformArithmetic;
            OpGroupNonUniformBitwiseOr $$vector<T,N> result Subgroup 0 $expr
        };
    }
}
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveOrMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitOr($1)")
matrix<T,N,M> WaveMaskBitOr(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskBitXor(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupXor($1)";
    case cuda: __intrinsic_asm "_waveXor($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveBitXor($1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformArithmetic;
            OpGroupNonUniformBitwiseXor $$T result Subgroup 0 $expr
        };
    }
}
__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskBitXor(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupXor($1)";
    case cuda: __intrinsic_asm "_waveXorMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveBitXor($1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformArithmetic;
            OpGroupNonUniformBitwiseXor $$vector<T,N> result Subgroup 0 $expr
        };
    }
}
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveXorMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitXor($1)")
matrix<T,N,M> WaveMaskBitXor(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskMax(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupMax($1)";
    case cuda: __intrinsic_asm "_waveMax($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveMax($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$T result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$T result Subgroup 0 $expr};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$T result Subgroup 0 $expr};
    }
}
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskMax(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupMax($1)";
    case cuda: __intrinsic_asm "_waveMaxMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveMax($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$vector<T,N> result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$vector<T,N> result Subgroup 0 $expr};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$vector<T,N> result Subgroup 0 $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveMaxMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMax($1)")
matrix<T,N,M> WaveMaskMax(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskMin(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupMin($1)";
    case cuda: __intrinsic_asm "_waveMin($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveMin($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$T result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$T result Subgroup 0 $expr};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$T result Subgroup 0 $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskMin(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupMin($1)";
    case cuda: __intrinsic_asm "_waveMinMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveMin($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$vector<T,N>  result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$vector<T,N>  result Subgroup 0 $expr};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$vector<T,N>  result Subgroup 0 $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveMinMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMin($1)")
matrix<T,N,M> WaveMaskMin(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskProduct(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupMul($1)";
    case cuda: __intrinsic_asm "_waveProduct($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveProduct($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$T result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $expr;
                OpGroupNonUniformIMul $$uint %mulResult Subgroup 0 %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$T result Subgroup 0 $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskProduct(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupMul($1)";
    case cuda: __intrinsic_asm "_waveProductMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveProduct($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$vector<T,N> result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$vector<uint,N> %uvalue $expr;
                OpGroupNonUniformIMul $$vector<uint,N> %mulResult Subgroup 0 %uvalue;
                OpBitcast $$vector<T,N> result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$vector<T,N> result Subgroup 0 $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveProductMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveProduct($1)")
matrix<T,N,M> WaveMaskProduct(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskSum(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupAdd($1)";
    case cuda: __intrinsic_asm "_waveSum($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveSum($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$T result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $expr;
                OpGroupNonUniformIAdd $$uint %mulResult Subgroup 0 %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$T result Subgroup 0 $expr};
    }
}
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskSum(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupAdd($1)";
    case cuda: __intrinsic_asm "_waveSumMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveActiveSum($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$vector<T,N> result Subgroup 0 $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic; 
                // TODO: use the correct integer width
                OpBitcast $$vector<uint,N> %uvalue $expr;
                OpGroupNonUniformIAdd $$vector<uint,N> %mulResult Subgroup 0 %uvalue;
                OpBitcast $$vector<T,N> result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$vector<T,N> result Subgroup 0 $expr};
    }
}
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveSumMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveSum($1)")
matrix<T,N,M> WaveMaskSum(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__cuda_sm_version(7.0)
bool WaveMaskAllEqual(WaveMask mask, T value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAllEqual($1)";
    case hlsl:
        __intrinsic_asm "WaveActiveAllEqual($1)";
    case cuda:
        __intrinsic_asm "_waveAllEqual($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAllEqual $$bool result Subgroup $value
        };
    default:
        return false;
    }
}
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__cuda_sm_version(7.0)
bool WaveMaskAllEqual(WaveMask mask, vector<T,N> value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAllEqual($1)";
    case hlsl:
        __intrinsic_asm "WaveActiveAllEqual($1)";
    case cuda:
        __intrinsic_asm "_waveAllEqualMultiple($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAllEqual $$bool result Subgroup $value
        };
    default:
        return false;
    }
}
__generic<T : __BuiltinType, let N : int, let M : int>
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveAllEqualMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveAllEqual($1)")
bool WaveMaskAllEqual(WaveMask mask, matrix<T,N,M> value);

// Prefix

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskPrefixProduct(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveMul($1)";
    case cuda: __intrinsic_asm "_wavePrefixProduct($0, $1)";
    case hlsl: __intrinsic_asm "WavePrefixProduct($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$T result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $expr;
                OpGroupNonUniformIMul $$uint %mulResult Subgroup ExclusiveScan %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpGroupNonUniformIMul $$T result Subgroup ExclusiveScan $expr};
    }
}
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskPrefixProduct(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveMul($1)";
    case cuda: __intrinsic_asm "_wavePrefixProductMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WavePrefixProduct($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$vector<T,N> result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$vector<uint,N> %uvalue $expr;
                OpGroupNonUniformIMul $$vector<uint,N> %mulResult Subgroup ExclusiveScan %uvalue;
                OpBitcast $$vector<T,N> result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$vector<T,N> result Subgroup ExclusiveScan $expr};
    }
}
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_wavePrefixProductMultiple($0, $1)")
__target_intrinsic(hlsl, "WavePrefixProduct($1)")
matrix<T,N,M> WaveMaskPrefixProduct(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskPrefixSum(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAdd($1)";
    case cuda: __intrinsic_asm "_wavePrefixSum($0, $1)";
    case hlsl: __intrinsic_asm "WavePrefixSum($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$T result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                %uvalue:$$uint = OpBitcast $expr;
                %mulResult:$$uint = OpGroupNonUniformIAdd Subgroup ExclusiveScan %uvalue;
                result:$$T = OpBitcast %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$T result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskPrefixSum(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAdd($1)";
    case cuda: __intrinsic_asm "_wavePrefixSumMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WavePrefixSum($1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$vector<T,N> result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                %uvalue: $$vector<uint,N> = OpBitcast $expr;
                %mulResult: $$vector<uint,N> = OpGroupNonUniformIAdd Subgroup ExclusiveScan %uvalue;
                result: $$vector<T,N> = OpBitcast  %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$vector<T,N> result Subgroup ExclusiveScan $expr};
    }
}
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_wavePrefixSumMultiple($0, $1)")
__target_intrinsic(hlsl, "WavePrefixSum($1)")
matrix<T,N,M> WaveMaskPrefixSum(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
T WaveMaskReadLaneFirst(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcastFirst($1)";
    case cuda: __intrinsic_asm "_waveReadFirst($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneFirst($1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformBallot; OpGroupNonUniformBroadcastFirst $$T result Subgroup $expr};
    }
}
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
vector<T,N> WaveMaskReadLaneFirst(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcastFirst($1)";
    case cuda: __intrinsic_asm "_waveReadFirstMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneFirst($1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformBallot; OpGroupNonUniformBroadcastFirst $$vector<T,N> result Subgroup $expr};
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveReadFirstMultiple($0, $1)")
matrix<T,N,M> WaveMaskReadLaneFirst(WaveMask mask, matrix<T,N,M> expr);

// WaveMask SM6.5 like intrinsics

// TODO(JS): On HLSL it only works for 32 bits or less

__generic<T : __BuiltinType>
__glsl_extension(GL_NV_shader_subgroup_partitioned)
__spirv_version(1.1)
__cuda_sm_version(7.0)
WaveMask WaveMaskMatch(WaveMask mask, T value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupPartitionNV($1).x";
    case cuda: __intrinsic_asm "_waveMatchScalar($0, $1).x";
    case hlsl: __intrinsic_asm "WaveMatch($1).x";
    case spirv:
        return (spirv_asm
        {
            OpCapability GroupNonUniformPartitionedNV;
            OpExtension "SPV_NV_shader_subgroup_partitioned";
            OpGroupNonUniformPartitionNV $$uint4 result $value
        }).x;
    }
}
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_NV_shader_subgroup_partitioned)
__spirv_version(1.1)
__cuda_sm_version(7.0)
WaveMask WaveMaskMatch(WaveMask mask, vector<T,N> value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupPartitionNV($1).x";
    case cuda: __intrinsic_asm "_waveMatchMultiple($0, $1).x";
    case hlsl: __intrinsic_asm "WaveMatch($1).x";
    case spirv:
        return (spirv_asm
        {
            OpCapability GroupNonUniformPartitionedNV;
            OpExtension "SPV_NV_shader_subgroup_partitioned";
            OpGroupNonUniformPartitionNV $$uint4 result $value
        }).x;
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMatch($1).x")
__glsl_extension(GL_NV_shader_subgroup_partitioned)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupPartitionNV($1).x")
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveMatchMultiple($0, $1)")
WaveMask WaveMaskMatch(WaveMask mask, matrix<T,N,M> value);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskPrefixBitAnd(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAnd($1)";
    case cuda: __intrinsic_asm "_wavePrefixAnd($0, $1)";
    case hlsl: __intrinsic_asm "WaveMultiPrefixBitAnd($1, uint4($0, 0, 0, 0))";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$T result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskPrefixBitAnd(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAnd($1)";
    case cuda: __intrinsic_asm "_wavePrefixAndMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveMultiPrefixBitAnd($1, uint4($0, 0, 0, 0))";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$vector<T,N> result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitAnd($1, uint4($0, 0, 0, 0))")
__target_intrinsic(cuda, "_wavePrefixAndMultiple(_getMultiPrefixMask($0, $1)")
matrix<T,N,M> WaveMaskPrefixBitAnd(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskPrefixBitOr(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveOr($1)";
    case cuda: __intrinsic_asm "_wavePrefixOr($0, $1)";
    case hlsl: __intrinsic_asm "WaveMultiPrefixBitOr($1, uint4($0, 0, 0, 0))";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$T result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskPrefixBitOr(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveOr($1)";
    case cuda: __intrinsic_asm "_wavePrefixOrMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveMultiPrefixBitOr($1, uint4($0, 0, 0, 0))";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseOr $$vector<T,N> result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitOr($1, uint4($0, 0, 0, 0))")
__target_intrinsic(cuda, "_wavePrefixOrMultiple($0, $1)")
matrix<T,N,M> WaveMaskPrefixBitOr(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveMaskPrefixBitXor(WaveMask mask, T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveXor($1)";
    case cuda: __intrinsic_asm "_wavePrefixXor($0, $1)";
    case hlsl: __intrinsic_asm "WaveMultiPrefixBitXor($1, uint4($0, 0, 0, 0))";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$T result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WaveMaskPrefixBitXor(WaveMask mask, vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveXor($1)";
    case cuda: __intrinsic_asm "_wavePrefixXorMultiple($0, $1)";
    case hlsl: __intrinsic_asm "WaveMultiPrefixBitXor($1, uint4($0, 0, 0, 0))";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$vector<T,N> result Subgroup ExclusiveScan $expr};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitXor($1, uint4($0, 0, 0, 0))")
__target_intrinsic(cuda, "_wavePrefixXorMultiple($0, $1)")
matrix<T,N,M> WaveMaskPrefixBitXor(WaveMask mask, matrix<T,N,M> expr);

// Shader model 6.0 stuff

// Information for GLSL wave/subgroup support
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
T QuadReadLaneAt(T sourceValue, uint quadLaneID)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadLaneAt";
    case glsl:
        __intrinsic_asm "subgroupQuadBroadcast";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$T = OpGroupNonUniformQuadBroadcast Subgroup $sourceValue $quadLaneID;
        };
    }
}
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
vector<T,N> QuadReadLaneAt(vector<T,N> sourceValue, uint quadLaneID)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadLaneAt";
    case glsl:
        __intrinsic_asm "subgroupQuadBroadcast";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$vector<T,N> = OpGroupNonUniformQuadBroadcast Subgroup $sourceValue $quadLaneID;
        };
    }
}
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadLaneAt(matrix<T,N,M> sourceValue, uint quadLaneID);


__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
T QuadReadAcrossX(T localValue)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadAcrossX";
    case glsl:
        __intrinsic_asm "subgroupQuadSwapHorizontal($0)";
    case spirv:
        uint direction = 0u;
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$T = OpGroupNonUniformQuadSwap Subgroup $localValue $direction;
        };
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
vector<T,N> QuadReadAcrossX(vector<T,N> localValue)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadAcrossX";
    case glsl:
        __intrinsic_asm "subgroupQuadSwapHorizontal($0)";
    case spirv:
        uint direction = 0u;
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$vector<T,N> = OpGroupNonUniformQuadSwap Subgroup $localValue $direction;
        };
    }
}
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadAcrossX(matrix<T,N,M> localValue);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
T QuadReadAcrossY(T localValue)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadAcrossY";
    case glsl:
        __intrinsic_asm "subgroupQuadSwapVertical($0)";
    case spirv:
        uint direction = 1u;
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$T = OpGroupNonUniformQuadSwap Subgroup $localValue $direction;
        };
    }
}
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
vector<T,N> QuadReadAcrossY(vector<T,N> localValue)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadAcrossY";
    case glsl:
        __intrinsic_asm "subgroupQuadSwapVertical($0)";
    case spirv:
        uint direction = 1u;
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$vector<T,N> = OpGroupNonUniformQuadSwap Subgroup $localValue $direction;
        };
    }
}

__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadAcrossY(matrix<T,N,M> localValue);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
T QuadReadAcrossDiagonal(T localValue)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadAcrossDiagonal";
    case glsl:
        __intrinsic_asm "subgroupQuadSwapDiagonal($0)";
    case spirv:
        uint direction = 2u;
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$T = OpGroupNonUniformQuadSwap Subgroup $localValue $direction;
        };
    }
}
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
vector<T,N> QuadReadAcrossDiagonal(vector<T,N> localValue)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "QuadReadAcrossDiagonal";
    case glsl:
        __intrinsic_asm "subgroupQuadSwapDiagonal($0)";
    case spirv:
        uint direction = 2u;
        return spirv_asm {
            OpCapability GroupNonUniformQuad;
            result:$$vector<T,N> = OpGroupNonUniformQuadSwap Subgroup $localValue $direction;
        };
    }
}
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadAcrossDiagonal(matrix<T,N,M> localValue);

// WaveActiveBitAnd, WaveActiveBitOr, WaveActiveBitXor
${{{{
struct WaveActiveBitOpEntry { const char* hlslName; const char* glslName; const char* spirvName; };
const WaveActiveBitOpEntry kWaveActiveBitOpEntries[] = {{"BitAnd", "And", "BitwiseAnd"}, {"BitOr", "Or", "BitwiseOr"}, {"BitXor", "Xor", "BitwiseXor"}};
for (auto opName : kWaveActiveBitOpEntries) {
}}}}

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveActive$(opName.hlslName)(T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroup$(opName.glslName)($0)";
    case hlsl: __intrinsic_asm "WaveActive$(opName.hlslName)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniform$(opName.spirvName) $$T result Subgroup Reduce $expr};
    default:
        return WaveMask$(opName.hlslName)(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T, N> WaveActive$(opName.hlslName)(vector<T, N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroup$(opName.glslName)($0)";
    case hlsl: __intrinsic_asm "WaveActive$(opName.hlslName)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniform$(opName.spirvName) $$vector<T, N> result Subgroup Reduce $expr};
    default:
        return WaveMask$(opName.hlslName)(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActive$(opName.hlslName)(matrix<T, N, M> expr)
{
    return WaveMask$(opName.hlslName)(WaveGetActiveMask(), expr);
}
${{{{
} // WaveActiveBitAnd, WaveActiveBitOr, WaveActiveBitXor
}}}}

// WaveActiveMin/Max
${{{{
const char* kWaveActiveMinMaxNames[] = {"Min", "Max"};
for (const char* opName : kWaveActiveMinMaxNames) {
}}}}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveActive$(opName)(T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroup$(opName)($0)";
    case hlsl: __intrinsic_asm "WaveActive$(opName)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformF$(opName) $$T result Subgroup Reduce $expr};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformU$(opName) $$T result Subgroup Reduce $expr};
        else
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformS$(opName) $$T result Subgroup Reduce $expr};
    default:
        return WaveMask$(opName)(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T, N> WaveActive$(opName)(vector<T, N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroup$(opName)($0)";
    case hlsl: __intrinsic_asm "WaveActive$(opName)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformF$(opName) $$vector<T, N> result Subgroup Reduce $expr};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformU$(opName) $$vector<T, N> result Subgroup Reduce $expr};
        else
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformS$(opName) $$vector<T, N> result Subgroup Reduce $expr};
    default:
        return WaveMask$(opName)(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActive$(opName)(matrix<T, N, M> expr)
{
    return WaveMask$(opName)(WaveGetActiveMask(), expr);
}

${{{{
} // WaveActiveMinMax.
}}}}

// WaveActiveProduct/Sum
${{{{
struct WaveActiveProductSumEntry { const char* hlslName; const char* glslName; };
const WaveActiveProductSumEntry kWaveActivProductSumNames[] = {{"Product", "Mul"}, {"Sum", "Add"}};
for (auto opName : kWaveActivProductSumNames) {
}}}}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WaveActive$(opName.hlslName)(T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroup$(opName.glslName)($0)";
    case hlsl: __intrinsic_asm "WaveActive$(opName.hlslName)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformF$(opName.glslName) $$T result Subgroup 0 $expr
            };
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $expr;
                OpGroupNonUniformI$(opName.glslName) $$uint %mulResult Subgroup 0 %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformI$(opName.glslName) $$T result Subgroup 0 $expr
            };
    default:
        return WaveMask$(opName.hlslName)(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(hlsl)
vector<T,N> WaveActive$(opName.hlslName)(vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroup$(opName.glslName)($0)";
    case hlsl: __intrinsic_asm "WaveActive$(opName.hlslName)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformF$(opName.glslName) $$vector<T,N> result Subgroup 0 $expr
            };
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$vector<uint,N> %uvalue $expr;
                OpGroupNonUniformI$(opName.glslName) $$vector<uint,N> %$(opName.glslName)Result Subgroup 0 %uvalue;
                OpBitcast $$vector<T,N> result %$(opName.glslName)Result
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformI$(opName.glslName) $$vector<T,N> result Subgroup 0 $expr
            };
    default:
        return WaveMask$(opName.hlslName)(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActive$(opName.hlslName)(matrix<T, N, M> expr)
{
    return WaveMask$(opName.hlslName)(WaveGetActiveMask(), expr);
}
${{{{
} // WaveActiveProduct/WaveActiveProductSum.
}}}}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
bool WaveActiveAllEqual(T value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAllEqual($0)";
    case hlsl:
        __intrinsic_asm "WaveActiveAllEqual";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAllEqual $$bool result Subgroup $value
        };
    default:
        return WaveMaskAllEqual(WaveGetActiveMask(), value);
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
bool WaveActiveAllEqual(vector<T,N> value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAllEqual($0)";
    case hlsl:
        __intrinsic_asm "WaveActiveAllEqual";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAllEqual $$bool result Subgroup $value
        };
    default:
        return WaveMaskAllEqual(WaveGetActiveMask(), value);
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
bool WaveActiveAllEqual(matrix<T, N, M> value)
{
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
bool WaveActiveAllTrue(bool condition)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAll($0)";
    case hlsl:
        __intrinsic_asm "WaveActiveAllTrue($0)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAll $$bool result Subgroup $condition
        };
    default:
        return WaveMaskAllTrue(WaveGetActiveMask(), condition);
    }
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
bool WaveActiveAnyTrue(bool condition)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupAny($0)";
    case hlsl:
        __intrinsic_asm "WaveActiveAnyTrue($0)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAny $$bool result Subgroup $condition
        };
    default:
        return WaveMaskAnyTrue(WaveGetActiveMask(), condition);
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
uint4 WaveActiveBallot(bool condition)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallot($0)";
    case hlsl:
        __intrinsic_asm "WaveActiveBallot";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBallot $$uint4 result Subgroup $condition
        };
    default:
        return WaveMaskBallot(WaveGetActiveMask(), condition);
    }
}

__target_intrinsic(hlsl)
uint WaveActiveCountBits(bool value)
{
    return WaveMaskCountBits(WaveGetActiveMask(), value);
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
uint WaveGetLaneCount()
{
    __target_switch
    {
    case glsl: __intrinsic_asm  "(gl_SubgroupSize)";
    case cuda: __intrinsic_asm  "(warpSize)";
    case hlsl: __intrinsic_asm  "WaveGetLaneCount()";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniform;
            result:$$uint = OpLoad builtin(SubgroupSize:uint)
        };
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
uint WaveGetLaneIndex()
{
    __target_switch
    {
    case glsl: __intrinsic_asm  "(gl_SubgroupInvocationID)";
    case cuda: __intrinsic_asm  "_getLaneId()";
    case hlsl: __intrinsic_asm  "WaveGetLaneIndex()";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniform;
            result:$$uint = OpLoad builtin(SubgroupLocalInvocationId:uint)
        };
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
bool WaveIsFirstLane()
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupElect()";
    case hlsl:
        __intrinsic_asm "WaveIsFirstLane()";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformElect $$bool result Subgroup
        };
    default:
        return WaveMaskIsFirstLane(WaveGetActiveMask());
    }
}

// It's useful to have a wave uint4 version of countbits, because some wave functions return uint4.
// This implementation tries to limit the amount of work required by the actual lane count.
uint _WaveCountBits(uint4 value)
{
    __target_switch
    {
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBallotBitCount $$uint result Subgroup Reduce $value
        };
    default:
        // Assume since WaveGetLaneCount should be known at compile time, the branches will hopefully boil away
        const uint waveLaneCount = WaveGetLaneCount();
        switch ((waveLaneCount - 1) / 32)
        {
            default:
            case 0: return countbits(value.x);
            case 1: return countbits(value.x) + countbits(value.y);
            case 2: return countbits(value.x) + countbits(value.y) + countbits(value.z);
            case 3: return countbits(value.x) + countbits(value.y) + countbits(value.z) + countbits(value.w);
        }
    }
}


// Prefix

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WavePrefixProduct(T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveMul($0)";
    case hlsl: __intrinsic_asm "WavePrefixProduct";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformFMul $$T result Subgroup ExclusiveScan $expr
            };
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $expr;
                OpGroupNonUniformIMul $$uint %mulResult Subgroup ExclusiveScan %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$T result Subgroup ExclusiveScan $expr};
    default:
        return WaveMaskPrefixProduct(WaveGetActiveMask(), expr);
    }
}


__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WavePrefixProduct(vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveMul($0)";
    case hlsl: __intrinsic_asm "WavePrefixProduct";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$vector<T,N> result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$vector<uint,N> %uvalue $expr;
                OpGroupNonUniformIMul $$vector<uint,N> %mulResult Subgroup ExclusiveScan %uvalue;
                OpBitcast $$vector<T,N> result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$vector<T,N> result Subgroup ExclusiveScan $expr};
    default:
        return WaveMaskPrefixProduct(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WavePrefixProduct(matrix<T, N, M> expr)
{
    return WaveMaskPrefixProduct(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
T WavePrefixSum(T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAdd($0)";
    case hlsl: __intrinsic_asm "WavePrefixSum";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$T result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                %uvalue:$$uint = OpBitcast $expr;
                %mulResult:$$uint = OpGroupNonUniformIAdd Subgroup ExclusiveScan %uvalue;
                result:$$T = OpBitcast %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$T result Subgroup ExclusiveScan $expr};
    default:
        return WaveMaskPrefixSum(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
vector<T,N> WavePrefixSum(vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAdd($0)";
    case hlsl: __intrinsic_asm "WavePrefixSum";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$vector<T,N> result Subgroup ExclusiveScan $expr};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                %uvalue:$$vector<uint,N> = OpBitcast $expr;
                %mulResult:$$vector<uint,N> = OpGroupNonUniformIAdd Subgroup ExclusiveScan %uvalue;
                result:$$vector<T,N> = OpBitcast %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$vector<T,N> result Subgroup ExclusiveScan $expr};
    default:
        return WaveMaskPrefixSum(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> WavePrefixSum(matrix<T,N,M> expr)
{
    return WaveMaskPrefixSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
T WaveReadLaneFirst(T expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcastFirst($0)";
    case hlsl: __intrinsic_asm "WaveReadLaneFirst";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformBallot; OpGroupNonUniformBroadcastFirst $$T result Subgroup $expr};
    default:
        return WaveMaskReadLaneFirst(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
vector<T,N> WaveReadLaneFirst(vector<T,N> expr)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcastFirst($0)";
    case hlsl: __intrinsic_asm "WaveReadLaneFirst";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformBallot; OpGroupNonUniformBroadcastFirst $$vector<T,N> result Subgroup $expr};
    default:
        return WaveMaskReadLaneFirst(WaveGetActiveMask(), expr);
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> WaveReadLaneFirst(matrix<T,N,M> expr)
{
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), expr);
}

// NOTE! WaveBroadcastLaneAt is *NOT* standard HLSL
// It is provided as access to subgroupBroadcast which can only take a
// constexpr laneId.
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
// Versions SPIR-V greater than 1.4 loosen this restriction, and allow 'dynamic uniform' index
// If that's the behavior required then client code should use WaveReadLaneAt which works this way.
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
T WaveBroadcastLaneAt(T value, constexpr int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcast($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {OpCapability GroupNonUniformBallot; OpGroupNonUniformBroadcast $$T result Subgroup $value $ulane};
    default:
        return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, lane);
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
vector<T,N> WaveBroadcastLaneAt(vector<T,N> value, constexpr int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBroadcast($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {OpCapability GroupNonUniformBallot; OpGroupNonUniformBroadcast $$vector<T,N> result Subgroup $value $ulane};
    default:
        return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, lane);
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple(_getActiveMask(), $0, $1)")
__target_intrinsic(hlsl, "WaveReadLaneAt")
matrix<T, N, M> WaveBroadcastLaneAt(matrix<T, N, M> value, constexpr int lane)
{
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, lane);
}

// TODO(JS): If it can be determines that the `laneId` is constExpr, then subgroupBroadcast
// could be used on GLSL. For now we just use subgroupShuffle
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
T WaveReadLaneAt(T value, int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupShuffle($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {OpCapability GroupNonUniformShuffle; OpGroupNonUniformShuffle $$T result Subgroup $value $ulane};
    default:
        return WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane);
    }
}

__generic<T : __BuiltinType, let N : int>
__spirv_version(1.3)
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
vector<T,N> WaveReadLaneAt(vector<T,N> value, int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupShuffle($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {OpCapability GroupNonUniformShuffle; OpGroupNonUniformShuffle $$vector<T,N> result Subgroup $value $ulane};
    default:
        return WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane);
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple(_getActiveMask(), $0, $1)")
__target_intrinsic(hlsl)
matrix<T, N, M> WaveReadLaneAt(matrix<T, N, M> value, int lane)
{
    return WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane);
}

// NOTE! WaveShuffle is a NON STANDARD HLSL intrinsic! It will map to WaveReadLaneAt on HLSL
// which means it will only work on hardware which allows arbitrary laneIds which is not true
// in general because it breaks the HLSL standard, which requires it's 'dynamically uniform' across the Wave.
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
T WaveShuffle(T value, int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupShuffle($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {OpCapability GroupNonUniformShuffle; OpGroupNonUniformShuffle $$T result Subgroup $value $ulane};
    default:
        return WaveMaskShuffle(WaveGetActiveMask(), value, lane);
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
vector<T,N> WaveShuffle(vector<T,N> value, int lane)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupShuffle($0, $1)";
    case hlsl: __intrinsic_asm "WaveReadLaneAt";
    case spirv:
        let ulane = uint(lane);
        return spirv_asm {OpCapability GroupNonUniformShuffle; OpGroupNonUniformShuffle $$vector<T,N> result Subgroup $value $ulane};
    default:
        return WaveMaskShuffle(WaveGetActiveMask(), value, lane);
    }
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveReadLaneAt")
matrix<T, N, M> WaveShuffle(matrix<T, N, M> value, int lane)
{
    return WaveMaskShuffle(WaveGetActiveMask(), value, lane);
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
uint WavePrefixCountBits(bool value)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBallotExclusiveBitCount(subgroupBallot($0))";
    case hlsl: __intrinsic_asm "WavePrefixCountBits($0)";
    case spirv:
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            %mask:$$uint4 = OpGroupNonUniformBallot Subgroup $value;
            OpGroupNonUniformBallotBitCount $$uint result Subgroup 2 %mask
        };
    default:
        return WaveMaskPrefixCountBits(WaveGetActiveMask(), value);
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
uint4 WaveGetConvergedMulti()
{
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupBallot(true)";
    case hlsl: __intrinsic_asm "WaveActiveBallot(true)";
    case cuda: __intrinsic_asm "make_uint4(__activemask(), 0, 0, 0)";
    case spirv:
        let _true = true;
        return spirv_asm
        {
            OpCapability GroupNonUniformBallot;
            OpGroupNonUniformBallot $$uint4 result Subgroup $_true
        };
    }
}

[ForceInline]
uint4 WaveGetActiveMulti()
{
    return WaveGetConvergedMulti();
}

// Shader model 6.5 stuff
// https://github.com/microsoft/DirectX-Specs/blob/master/d3d/HLSL_ShaderModel6_5.md

__generic<T : __BuiltinType>
__target_intrinsic(hlsl)
uint4 WaveMatch(T value)
{
    return WaveMaskMatch(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int>
__target_intrinsic(hlsl)
uint4 WaveMatch(vector<T,N> value)
{
    return WaveMaskMatch(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
uint4 WaveMatch(matrix<T,N,M> value)
{
    return WaveMaskMatch(WaveGetActiveMask(), value);
}

__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_popc(__ballot_sync(($1).x, $0) & _getLaneLtMask())")
uint WaveMultiPrefixCountBits(bool value, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAnd($0)")
__target_intrinsic(cuda, "_wavePrefixAnd(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixBitAnd(T expr, uint4 mask);

__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAnd($0)")
__target_intrinsic(cuda, "_wavePrefixAndMultiple(_getMultiPrefixMask(($1).x), $0)")
__generic<T : __BuiltinArithmeticType, let N : int>
vector<T,N> WaveMultiPrefixBitAnd(vector<T,N> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixAndMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixBitAnd(matrix<T,N,M> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveOr($0)")
__target_intrinsic(cuda, "_wavePrefixOr(, _getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixBitOr(T expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveOr($0)")
__target_intrinsic(cuda, "_wavePrefixOrMultiple(_getMultiPrefixMask(($1).x), $0)")
vector<T,N> WaveMultiPrefixBitOr(vector<T,N> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixOrMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixBitOr(matrix<T,N,M> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveXor($0)")
__target_intrinsic(cuda, "_wavePrefixXor(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixBitXor(T expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveXor($0)")
__target_intrinsic(cuda, "_wavePrefixXorMultiple(_getMultiPrefixMask(($1).x), $0)")
vector<T,N> WaveMultiPrefixBitXor(vector<T,N> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixXorMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixBitXor(matrix<T,N,M> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixProduct(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixProduct(T value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixProductMultiple(_getMultiPrefixMask(($1).x), $0)")
vector<T,N> WaveMultiPrefixProduct(vector<T,N> value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixProductMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixProduct(matrix<T,N,M> value, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixSum(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixSum(T value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixSumMultiple(_getMultiPrefixMask(($1).x), $0 )")
vector<T,N> WaveMultiPrefixSum(vector<T,N> value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixSumMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixSum(matrix<T,N,M> value, uint4 mask);

// `typedef`s to help with the fact that HLSL has been sorta-kinda case insensitive at various points
typedef Texture2D texture2D;

${{{{

// Buffer types

static const struct {
    char const*         name;
    SlangResourceAccess access;
} kBaseBufferAccessLevels[] = {
    { "",                   SLANG_RESOURCE_ACCESS_READ },
    { "RW",                 SLANG_RESOURCE_ACCESS_READ_WRITE },
    { "RasterizerOrdered",  SLANG_RESOURCE_ACCESS_RASTER_ORDERED },
};
static const int kBaseBufferAccessLevelCount = sizeof(kBaseBufferAccessLevels) / sizeof(kBaseBufferAccessLevels[0]);

for (int aa = 0; aa < kBaseBufferAccessLevelCount; ++aa)
{
    auto access = kBaseBufferAccessLevels[aa].access;
    sb << "__generic<T,let format:int=0>\n";
    sb << "typealias ";
    sb << kBaseBufferAccessLevels[aa].name;
    sb << "Buffer = __TextureImpl<T, __ShapeBuffer, 0, 0, 0, " << aa << ", 0, 0, format>;\n";
    
    bool isReadOnly = aa == 0;

    char const* glslTextureSizeFunc = (isReadOnly) ? "textureSize" : "imageSize";
    char const* glslLoadFuncName = (isReadOnly) ? "texelFetch" : "imageLoad";
    char const* spvLoadInstName = (isReadOnly) ? "OpImageFetch" : "OpImageRead";
}}}}

__generic<T, let format:int>
extension __TextureImpl<T, __ShapeBuffer, 0, 0, 0, $(aa), 0, 0, format>
{
    [__readNone]
    void GetDimensions(out uint dim)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetDimensions";
        case glsl: __intrinsic_asm "($1 = $(glslTextureSizeFunc)($0))";
        case spirv:
            dim = spirv_asm {
                OpCapability ImageQuery;
                result:$$uint = OpImageQuerySize $this;
            };
        }
    }

    __glsl_extension(GL_EXT_samplerless_texture_functions)
    $(isReadOnly?"[__readNone] ":"")
    T Load(int location)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Load";
        case glsl: __intrinsic_asm "$(glslLoadFuncName)($0, $1)$z";
        case spirv: return spirv_asm {
                %sampled:__sampledType(T) = $(spvLoadInstName) $this $location;
                __truncate $$T result __sampledType(T) %sampled;
            };
        }
    }

    $(isReadOnly?"[__readNone] ":"")
    T Load(int location, out uint status);

    __subscript(uint index) -> T {

        $(isReadOnly?"[__readNone] ":"")
        [ForceInline]
        get { return Load((int)index); }
${{{{
        if (access != SLANG_RESOURCE_ACCESS_READ) {
}}}}
            [nonmutating] set
            {
                __target_switch
                {
                case hlsl: __intrinsic_asm "($0)[$1] = $2";
                case glsl: __intrinsic_asm "imageStore($0, int($1), $V2)";
                case spirv: spirv_asm {
                        OpImageWrite $this $index $newValue;
                    };
                }
            }

            __intrinsic_op($(kIROp_ImageSubscript))
            ref;
${{{{
        } // access != SLANG_RESOURCE_ACCESS_READ
}}}}

        }
    

    };  // end extension
${{{{
}
}}}}


// DirectX Raytracing (DXR) Support
//
// The following is based on the experimental DXR SDK v0.09.01.
//
// Numbering follows the sections in the "D3D12 Raytracing Functional Spec" v0.09 (2018-03-12)
//

// 10.1.1 - Ray Flags

typedef uint RAY_FLAG;

static const RAY_FLAG RAY_FLAG_NONE                             = 0x00;
static const RAY_FLAG RAY_FLAG_FORCE_OPAQUE                     = 0x01;
static const RAY_FLAG RAY_FLAG_FORCE_NON_OPAQUE                 = 0x02;
static const RAY_FLAG RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH  = 0x04;
static const RAY_FLAG RAY_FLAG_SKIP_CLOSEST_HIT_SHADER          = 0x08;
static const RAY_FLAG RAY_FLAG_CULL_BACK_FACING_TRIANGLES       = 0x10;
static const RAY_FLAG RAY_FLAG_CULL_FRONT_FACING_TRIANGLES      = 0x20;
static const RAY_FLAG RAY_FLAG_CULL_OPAQUE                      = 0x40;
static const RAY_FLAG RAY_FLAG_CULL_NON_OPAQUE                  = 0x80;
static const RAY_FLAG RAY_FLAG_SKIP_TRIANGLES                   = 0x100;
static const RAY_FLAG RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES       = 0x200;

// 10.1.2 - Ray Description Structure

__target_intrinsic(hlsl, RayDesc)
__target_intrinsic(cuda, RayDesc)
struct RayDesc
{
    __target_intrinsic(hlsl, Origin)
    __target_intrinsic(cuda, Origin)
    float3 Origin;

    __target_intrinsic(hlsl, TMin)
    __target_intrinsic(cuda, TMin)
    float  TMin;

    __target_intrinsic(hlsl, Direction)
    __target_intrinsic(cuda, Direction)
    float3 Direction;

    __target_intrinsic(hlsl, TMax)
    __target_intrinsic(cuda, TMax)
    float  TMax;
};

// 10.1.3 - Ray Acceleration Structure

__builtin
__magic_type(RaytracingAccelerationStructureType)
__intrinsic_type($(kIROp_RaytracingAccelerationStructureType))
struct RaytracingAccelerationStructure {};

// 10.1.4 - Subobject Definitions

// TODO: We may decide to support these, but their reliance on C++ implicit
// constructor call syntax (`SomeType someVar(arg0, arg1);`) makes them
// annoying for the current Slang parsing strategy, and using global variables
// for this stuff comes across as a kludge rather than the best possible design.

// 10.1.5 - Intersection Attributes Structure

__target_intrinsic(hlsl, BuiltInTriangleIntersectionAttributes)
struct BuiltInTriangleIntersectionAttributes
{
    __target_intrinsic(hlsl, barycentrics)
    float2 barycentrics;
};

// 10.2 Shaders

// Right now new shader stages need to be added directly to the compiler
// implementation, rather than being something that can be declared in the stdlib.

// 10.3 - Intrinsics

// 10.3.1

// `executeCallableNV` is the GLSL intrinsic that will be used to implement
// `CallShader()` for GLSL-based targets.
//
__target_intrinsic(GL_NV_ray_tracing, "executeCallableNV")
__target_intrinsic(GL_EXT_ray_tracing, "executeCallableEXT")
void __executeCallable(uint shaderIndex, int payloadLocation);

// Next is the custom intrinsic that will compute the payload location
// for a type being used in a `CallShader()` call for GLSL-based targets.
//
__generic<Payload>
[__readNone]
__intrinsic_op($(kIROp_GetVulkanRayTracingPayloadLocation))
int __callablePayloadLocation(__ref Payload payload);

// Now we provide a hard-coded definition of `CallShader()` for GLSL-based
// targets, which maps the generic HLSL operation into the non-generic
// GLSL equivalent.
//
__generic<Payload>
void CallShader(uint shaderIndex, inout Payload payload)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "CallShader";
    case glsl:
        {
            [__vulkanCallablePayload]
            static Payload p;

            p = payload;
            __executeCallable(shaderIndex, __callablePayloadLocation(p));
            payload = p;
        }
    case spirv:
        {
            [__vulkanCallablePayload]
            static Payload p;

            p = payload;
            spirv_asm {
                OpExecuteCallableKHR $shaderIndex &p
            };
            payload = p;
        }
    }
}

// 10.3.2

__target_intrinsic(GL_NV_ray_tracing, "traceNV")
__target_intrinsic(GL_EXT_ray_tracing, "traceRayEXT")
void __traceRay(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    float3                          Origin,
    float                           TMin,
    float3                          Direction,
    float                           TMax,
    int                             PayloadLocation);

// TODO: Slang's parsing logic currently puts modifiers on
// the `GenericDecl` rather than the inner decl when
// using our default syntax, which seems wrong. We need
// to fix this, but for now using the expanded `__generic`
// syntax works in a pinch.
//
__generic<Payload>
[__readNone]
__intrinsic_op($(kIROp_GetVulkanRayTracingPayloadLocation))
int __rayPayloadLocation(__ref Payload payload);

__generic<payload_t>
void TraceRay(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    inout payload_t                 Payload)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "TraceRay";
    case cuda: __intrinsic_asm "traceOptiXRay";
    case glsl:
    {
        [__vulkanRayPayload]
        static payload_t p;

        p = Payload;
        __traceRay(
            AccelerationStructure,
            RayFlags,
            InstanceInclusionMask,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            Ray.Origin,
            Ray.TMin,
            Ray.Direction,
            Ray.TMax,
            __rayPayloadLocation(p));
        Payload = p;
    }
    case spirv:
    {
        [__vulkanRayPayload]
        static payload_t p;

        p = Payload;
        let origin = Ray.Origin;
        let direction = Ray.Direction;
        let tmin = Ray.TMin;
        let tmax = Ray.TMax;
        spirv_asm {
            OpTraceRayKHR 
                /**/ $AccelerationStructure
                /**/ $RayFlags
                /**/ $InstanceInclusionMask
                /**/ $RayContributionToHitGroupIndex
                /**/ $MultiplierForGeometryContributionToHitGroupIndex
                /**/ $MissShaderIndex
                /**/ $origin
                /**/ $tmin
                /**/ $direction
                /**/ $tmax
                /**/ &p;
        };
        Payload = p;
    }
    }
}

// NOTE!
// The name of the following functions may change when DXR supports
// a feature similar to the `GL_NV_ray_tracing_motion_blur` extension
//
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/nv/GLSL_NV_ray_tracing_motion_blur.txt

__target_intrinsic(glsl, "traceRayMotionNV")
__glsl_version(460)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
__glsl_extension(GL_EXT_ray_tracing)
void __traceMotionRay(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    float3                          Origin,
    float                           TMin,
    float3                          Direction,
    float                           TMax,
    float                           CurrentTime,
    int                             PayloadLocation);

__generic<payload_t>
void TraceMotionRay(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    float                           CurrentTime,
    inout payload_t                 Payload)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "TraceMotionRay";
    case glsl:
    {
        [__vulkanRayPayload]
        static payload_t p;

        p = Payload;
        __traceMotionRay(
            AccelerationStructure,
            RayFlags,
            InstanceInclusionMask,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            Ray.Origin,
            Ray.TMin,
            Ray.Direction,
            Ray.TMax,
            CurrentTime,
            __rayPayloadLocation(p));
        Payload = p;
    }
    case spirv:
    {
        [__vulkanRayPayload]
        static payload_t p;
        
        let origin = Ray.Origin;
        let direction = Ray.Direction;
        let tmin = Ray.TMin;
        let tmax = Ray.TMax;

        p = Payload;
        spirv_asm {
            OpCapability RayTracingMotionBlurNV;
            OpExtension "SPV_NV_ray_tracing_motion_blur";

            OpTraceRayMotionNV
                /**/ $AccelerationStructure
                /**/ $RayFlags
                /**/ $InstanceInclusionMask
                /**/ $RayContributionToHitGroupIndex
                /**/ $MultiplierForGeometryContributionToHitGroupIndex
                /**/ $MissShaderIndex
                /**/ $origin
                /**/ $tmin
                /**/ $direction
                /**/ $tmax
                /**/ $CurrentTime
                /**/ &p;
        };
        Payload = p;
    }
    }
}

// 10.3.3
__target_intrinsic(hlsl)
bool ReportHit<A>(float tHit, uint hitKind, A attributes);

bool __reportIntersection(float tHit, uint hitKind)
{
    __target_switch
    {
    case GL_EXT_ray_tracing: __intrinsic_asm "reportIntersectionEXT";
    case GL_NV_ray_tracing: __intrinsic_asm "reportIntersectionNV";
    case spirv:
        return spirv_asm {
            result:$$bool = OpReportIntersectionKHR $tHit $hitKind;
        };
    }
}

__generic<A>
__specialized_for_target(glsl)
__specialized_for_target(spirv)
bool ReportHit(float tHit, uint hitKind, A attributes)
{
    [__vulkanHitAttributes]
    static A a;

    a = attributes;
    return __reportIntersection(tHit, hitKind);
}

// 10.3.4
void IgnoreHit()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "IgnoreHit";
    case GL_EXT_ray_tracing: __intrinsic_asm "ignoreIntersectionEXT;";
    case GL_NV_ray_tracing: __intrinsic_asm "ignoreIntersectionNV";
    case cuda: __intrinsic_asm "optixIgnoreIntersection";
    case spirv: spirv_asm { OpIgnoreIntersectionKHR; %_ = OpLabel };
    }
}

// 10.3.5
void AcceptHitAndEndSearch()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "AcceptHitAndEndSearch";
    case GL_EXT_ray_tracing: __intrinsic_asm "terminateRayEXT;";
    case GL_NV_ray_tracing: __intrinsic_asm "terminateRayNV";
    case cuda: __intrinsic_asm "optixTerminateRay";
    case spirv: spirv_asm { OpTerminateRayKHR; %_ = OpLabel };
    }
}

// 10.4 - System Values and Special Semantics

// TODO: Many of these functions need to be restricted so that
// they can only be accessed from specific stages.

// 10.4.1 - Ray Dispatch System Values

uint3 DispatchRaysIndex()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "DispatchRaysIndex";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_LaunchIDEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_LaunchIDNV)";
    case cuda: __intrinsic_asm "optixGetLaunchIndex";
    case spirv:
        return spirv_asm {
            result:$$uint3 = OpLoad builtin(LaunchIdKHR:uint3);
        };
    }
}

uint3 DispatchRaysDimensions()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "DispatchRaysDimensions";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_LaunchSizeEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_LaunchSizeNV)";
    case cuda: __intrinsic_asm "optixGetLaunchDimensions";
    case spirv:
        return spirv_asm {
            result:$$uint3 = OpLoad builtin(LaunchSizeKHR:uint3);
        };
    }
}

// 10.4.2 - Ray System Values

float3 WorldRayOrigin()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "WorldRayOrigin";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_WorldRayOriginEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_WorldRayOriginNV)";
    case cuda: __intrinsic_asm "optixGetWorldRayOrigin";
    case spirv:
        return spirv_asm {
            result:$$float3 = OpLoad builtin(WorldRayOriginKHR:float3);
        };
    }
}

float3 WorldRayDirection()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "WorldRayDirection";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_WorldRayDirectionEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_WorldRayDirectionNV)";
    case cuda: __intrinsic_asm "optixGetWorldRayDirection";
    case spirv:
        return spirv_asm {
            result:$$float3 = OpLoad builtin(WorldRayDirectionKHR:float3);
        };
    }
}

float RayTMin()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "RayTMin";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_RayTminEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_RayTminNV)";
    case cuda: __intrinsic_asm "optixGetRayTmin";
    case spirv:
        return spirv_asm {
            result:$$float = OpLoad builtin(RayTminKHR:float);
        };
    }
}

// Note: The `RayTCurrent()` intrinsic should translate to
// either `gl_HitTNV` (for hit shaders) or `gl_RayTmaxNV`
// (for intersection shaders). Right now we are handling this
// during code emission, for simplicity.
//
// TODO: Once the compiler supports a more refined concept
// of profiles/capabilities and overloading based on them,
// we should simply provide two overloads here, specialized
// to the appropriate Vulkan stages.
//
float RayTCurrent()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "RayTCurrent";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_RayTmaxEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_RayTmaxNV)";
    case cuda: __intrinsic_asm "optixGetRayTmax";
    case spirv:
        return spirv_asm {
            result:$$float = OpLoad builtin(RayTmaxKHR:float);
        };
    }
}

uint RayFlags()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "RayFlags";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_IncomingRayFlagsEXT)";
    case GL_NV_ray_tracing: __intrinsic_asm "(gl_IncomingRayFlagsNV)";
    case cuda: __intrinsic_asm "optixGetRayFlags";
    case spirv:
        return spirv_asm {
            result:$$uint = OpLoad builtin(IncomingRayFlagsKHR:uint);
        };
    }
}

// 10.4.3 - Primitive/Object Space System Values

uint InstanceIndex()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "InstanceIndex";
    case __glslRayTracing: __intrinsic_asm "(gl_InstanceID)";
    case cuda: __intrinsic_asm "optixGetInstanceIndex";
    case spirv:
        return spirv_asm {
            result:$$uint = OpLoad builtin(InstanceId:uint);
        };
    }
}

uint InstanceID()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "InstanceID";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_InstanceCustomIndexEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "(gl_InstanceCustomIndexNV)";
    case cuda: __intrinsic_asm "optixGetInstanceId";
    case spirv:
        return spirv_asm {
            result:$$uint = OpLoad builtin(InstanceCustomIndexKHR:uint);
        };
    }
}

uint PrimitiveIndex()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "PrimitiveIndex";
    case __glslRayTracing:  __intrinsic_asm "(gl_PrimitiveID)";
    case cuda: __intrinsic_asm "optixGetPrimitiveIndex";
    case spirv:
        return spirv_asm {
            result:$$uint = OpLoad builtin(PrimitiveId:uint);
        };
    }
}

float3 ObjectRayOrigin()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "ObjectRayOrigin";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_ObjectRayOriginEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "(gl_ObjectRayOriginNV)";
    case cuda: __intrinsic_asm "optixGetObjectRayOrigin";
    case spirv:
        return spirv_asm {
            result:$$float3 = OpLoad builtin(ObjectRayOriginKHR:float3);
        };
    }
}

float3 ObjectRayDirection()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "ObjectRayDirection";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_ObjectRayDirectionEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "(gl_ObjectRayDirectionNV)";
    case cuda: __intrinsic_asm "optixGetObjectRayDirection";
    case spirv:
        return spirv_asm {
            result:$$float3 = OpLoad builtin(ObjectRayDirectionKHR:float3);
        };
    }
}

// TODO: optix has an optixGetObjectToWorldTransformMatrix function that returns 12
// floats by reference.
float3x4 ObjectToWorld3x4()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "ObjectToWorld3x4";
    case GL_EXT_ray_tracing: __intrinsic_asm "transpose(gl_ObjectToWorldEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "transpose(gl_ObjectToWorldNV)";
    case spirv:
        return spirv_asm {
            %mat = OpLoad builtin(ObjectToWorldKHR:float4x3);
            result:$$float3x4 = OpTranspose %mat;
        };
    }
}

float3x4 WorldToObject3x4()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "WorldToObject3x4";
    case GL_EXT_ray_tracing: __intrinsic_asm "transpose(gl_WorldToObjectEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "transpose(gl_WorldToObjectNV)";
    case spirv:
        return spirv_asm {
            %mat = OpLoad builtin(WorldToObjectKHR:float4x3);
            result:$$float3x4 = OpTranspose %mat;
        };
    }
}

float4x3 ObjectToWorld4x3()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "ObjectToWorld4x3";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_ObjectToWorldEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "(gl_ObjectToWorldNV)";
    case spirv:
        return spirv_asm {
            result:$$float4x3 = OpLoad builtin(ObjectToWorldKHR:float4x3);
        };
    }
}

float4x3 WorldToObject4x3()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "WorldToObject4x3";
    case GL_EXT_ray_tracing: __intrinsic_asm "(gl_WorldToObjectEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "(gl_WorldToObjectNV)";
    case spirv:
        return spirv_asm {
            result:$$float4x3 = OpLoad builtin(WorldToObjectKHR:float4x3);
        };
    }
}

// NOTE!
// The name of the following functions may change when DXR supports
// a feature similar to the `GL_NV_ray_tracing_motion_blur` extension

__glsl_version(460)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
__glsl_extension(GL_EXT_ray_tracing)
float RayCurrentTime()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "RayCurrentTime";
    case glsl:  __intrinsic_asm "(gl_CurrentRayTimeNV)";
    case spirv:
        return spirv_asm {
            result:$$float = OpLoad builtin(CurrentRayTimeNV:float);
        };
    }
}

// Note: The provisional DXR spec included these unadorned
// `ObjectToWorld()` and `WorldToObject()` functions, so
// we will forward them to the new names as a convience
// for users who are porting their code.
//
// TODO: Should we provide a deprecation warning on these
// declarations, so that users can know they aren't coding
// against the final spec?
//
float3x4 ObjectToWorld() { return ObjectToWorld3x4(); }
float3x4 WorldToObject() { return WorldToObject3x4(); }

// 10.4.4 - Hit Specific System values
uint HitKind()
{
    __target_switch
    {
    case hlsl:  __intrinsic_asm "HitKind";
    case GL_EXT_ray_tracing:  __intrinsic_asm "(gl_HitKindEXT)";
    case GL_NV_ray_tracing:  __intrinsic_asm "(gl_HitKindNV)";
    case cuda:  __intrinsic_asm "optixGetHitKind";
    case spirv:
        return spirv_asm {
            result:$$uint = OpLoad builtin(HitKindKHR:uint);
        };
    }
}

// Pre-defined hit kinds (not documented explicitly)
static const uint HIT_KIND_TRIANGLE_FRONT_FACE  = 254;
static const uint HIT_KIND_TRIANGLE_BACK_FACE   = 255;

//
// Shader Model 6.4
//

// Treats `left` and `right` as 4-component vectors of `UInt8` and computes `dot(left, right) + acc`
uint dot4add_u8packed(uint left, uint right, uint acc);

// Treats `left` and `right` as 4-component vectors of `Int8` and computes `dot(left, right) + acc`
int dot4add_i8packed(uint left, uint right, int acc);

// Computes `dot(left, right) + acc`.
//
// May not produce infinities or NaNs for intermediate results that overflow the range of `half`
float dot2add(float2 left, float2 right, float acc);

//
// Shader Model 6.5
//

//
// Mesh Shaders
//

// Set the number of output vertices and primitives for a mesh shader invocation.
__glsl_extension(GL_EXT_mesh_shader)
__glsl_version(450)
void SetMeshOutputCounts(uint vertexCount, uint primitiveCount)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "SetMeshOutputCounts";
    case glsl:
        __intrinsic_asm "SetMeshOutputsEXT";
    case spirv:
        return spirv_asm
        {
            OpCapability MeshShadingEXT;
            OpExtension "SPV_EXT_mesh_shader";
            OpSetMeshOutputsEXT $vertexCount $primitiveCount;
        };
    }
}

// Specify the number of downstream mesh shader thread groups to invoke from an amplification shader,
// and provide the values for per-mesh payload parameters.
//
// This function doesn't return.
//
[KnownBuiltin("DispatchMesh")]
void DispatchMesh<P>(uint threadGroupCountX, uint threadGroupCountY, uint threadGroupCountZ, __ref P meshPayload)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "DispatchMesh";
    case glsl:
        // This intrinsic doesn't take into account writing meshPayload. That
        // is dealt with separately by 'legalizeDispatchMeshPayloadForGLSL'.
        __intrinsic_asm "EmitMeshTasksEXT($0, $1, $2)";
    case spirv:
        return spirv_asm
        {
            OpCapability MeshShadingEXT;
            OpExtension "SPV_EXT_mesh_shader";
            OpEmitMeshTasksEXT $threadGroupCountX $threadGroupCountY $threadGroupCountZ &meshPayload;
            // OpEmitMeshTasksExt is a terminator, so we need to start a new
            // block to hold whatever comes after this intrinsic
            %_ = OpLabel
        };
    }
}

//
// "Sampler feedback" types `FeedbackTexture2D` and `FeedbackTexture2DArray`.
//

// https://microsoft.github.io/DirectX-Specs/d3d/SamplerFeedback.html

// The docs describe these as 'types' but their syntax makes them seem enum like, and enum is a simpler way to implement them
// But slang enums are always 'enum class like', so I use an empty struct type here

[sealed]
[builtin]
interface __BuiltinSamplerFeedbackType {};

[sealed]
__magic_type(FeedbackType, $(int(FeedbackType::Kind::MinMip)))
__target_intrinsic(hlsl, SAMPLER_FEEDBACK_MIN_MIP)
struct SAMPLER_FEEDBACK_MIN_MIP : __BuiltinSamplerFeedbackType {};

[sealed]
__magic_type(FeedbackType, $(int(FeedbackType::Kind::MipRegionUsed)))
__target_intrinsic(hlsl, SAMPLER_FEEDBACK_MIP_REGION_USED)
struct SAMPLER_FEEDBACK_MIP_REGION_USED : __BuiltinSamplerFeedbackType {};

// All of these objects are write-only resources that point to a special kind of unordered access view meant for sampler feedback.
__generic<T:__BuiltinSamplerFeedbackType>
extension __TextureImpl<T,__Shape2D, 0, 0, 0, $(kStdlibResourceAccessFeedback), 0, 0, 0>
{
    // With Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    void WriteSamplerFeedback<S>(Texture2D<S> tex, SamplerState samp, float2 location, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackBias<S>(Texture2D<S> tex, SamplerState samp, float2 location, float bias, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    void WriteSamplerFeedbackGrad<S>(Texture2D<S> tex, SamplerState samp, float2 location, float2 ddx, float2 ddy, float clamp);

    // Level

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    void WriteSamplerFeedbackLevel<S>(Texture2D<S> tex, SamplerState samp, float2 location, float lod);

    // Without Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3)")
    void WriteSamplerFeedback<S>(Texture2D<S> tex, SamplerState samp, float2 location);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    void WriteSamplerFeedbackBias<S>(Texture2D<S> tex, SamplerState samp, float2 location, float bias);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackGrad<S>(Texture2D<S> tex, SamplerState samp, float2 location, float2 ddx, float2 ddy);
};

__generic<T:__BuiltinSamplerFeedbackType>
extension __TextureImpl<T,__Shape2D, 1, 0, 0, $(kStdlibResourceAccessFeedback), 0, 0, 0>
{
    // With Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    void WriteSamplerFeedback<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackBias<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float bias, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    void WriteSamplerFeedbackGrad<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float3 ddx, float3 ddy, float clamp);

    // Level

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    void WriteSamplerFeedbackLevel<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float lod);

    // Without Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3)")
    void WriteSamplerFeedback<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    void WriteSamplerFeedbackBias<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float bias);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackGrad<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float3 ddx, float3 ddy);
};

//
// DXR 1.1 and `TraceRayInline` support
//

// Get the index of the geometry that was hit in an intersection, any-hit, or closest-hit shader
__target_intrinsic(GL_EXT_ray_tracing, "(gl_GeometryIndexEXT)")
uint GeometryIndex()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "GeometryIndex";
    case glsl: __intrinsic_asm "(gl_GeometryIndexEXT)";
    case spirv: return spirv_asm {
            result:$$uint = OpLoad builtin(RayGeometryIndexKHR:uint);
        };
    }
}

// Status of whether a (closest) hit has been committed in a `RayQuery`.
typedef uint COMMITTED_STATUS;

// No hit committed.
static const COMMITTED_STATUS COMMITTED_NOTHING = 0;

// Closest hit is a triangle.
//
// This could be an opaque triangle hit found by the fixed-function
// traversal and intersection implementation, or a non-opaque
// triangle hit committed by user code with `RayQuery.CommitNonOpaqueTriangleHit`
//
static const COMMITTED_STATUS COMMITTED_TRIANGLE_HIT = 1;

// Closest hit is a procedural primitive.
//
// A procedural hit primitive is committed using `RayQuery.CommitProceduralPrimitiveHit`.
static const COMMITTED_STATUS COMMITTED_PROCEDURAL_PRIMITIVE_HIT = 2;

// Type of candidate hit that a `RayQuery` is pausing at.
//
// A `RayQuery` can automatically commit hits with opaque triangles,
// but yields to user code for other hits to allow them to be
// dismissed or committed.
//
typedef uint CANDIDATE_TYPE;

// Candidate hit is a non-opaque triangle.
static const CANDIDATE_TYPE CANDIDATE_NON_OPAQUE_TRIANGLE = 0;

// Candidate hit is a procedural primitive.
static const CANDIDATE_TYPE CANDIDATE_PROCEDURAL_PRIMITIVE = 1;

// Handle to state of an in-progress ray-tracing query.
//
// The ray query is effectively a coroutine that user shader
// code can resume to continue tracing the ray, and which yields
// back to the user code at interesting events along the ray.
//
// Note: The treatment of the `RayQuery` type in Slang does not
// perfectly match its semantics in vanilla HLSL in some corner
// cases. Specifically, a `RayQuery` in vanilla HLSL is an
// opaque handle to mutable storage, and assigning a `RayQuery`
// or passing one as a parameter will only copy the *handle*,
// potentially resulting in aliasing of the underlying mutable
// storage.
//
// In contrast, Slang considers a `RayQuery` to own its mutable
// state, and (because the API does not support cloning of queries),
// `RayQuery` values are non-copyable (aka "move-only").
//
// The main place where this arises as a consideration is when
// passing a `RayQuery` down into a function that will perform
// mutating operations on it (e.g., `TraceRay` or `Proceed`):
//
//      void myFunc( inout RayQuery<FLAGS> q )
//      {
//          q.Proceed();
//      }
//
// In Slang, a parameter like `q` above should be declared `inout`.
// HLSL does not care about whether `q` is declared `inout` or not.
//
__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
[__NonCopyableType]
__intrinsic_type($(kIROp_RayQueryType))
struct RayQuery <let rayFlagsGeneric : RAY_FLAG = RAY_FLAG_NONE>
{
    // Create a new ray query, initialized to its default state.
    //
    __intrinsic_op($(kIROp_AllocateOpaqueHandle))
    __init();

    
    __target_intrinsic(glsl, "rayQueryInitializeEXT($0, $1, $2, $3, $4, $5, $6, $7)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [mutating]
    void __rayQueryInitializeEXT(
        RaytracingAccelerationStructure accelerationStructure,
        RAY_FLAG                        rayFlags,
        uint                            instanceInclusionMask,
        float3                          origin,
        float                           tMin,
        float3                          direction,
        float                           tMax)
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "rayQueryInitializeEXT($0, $1, $2, $3, $4, $5, $6, $7)";
        case spirv:
            spirv_asm {
                OpRayQueryInitializeKHR &this $accelerationStructure $rayFlags $instanceInclusionMask $origin $tMin $direction $tMax;
            };
        }
    }

    // Initialize a ray-tracing query.
    //
    // This method may be called on a "fresh" ray query, or
    // on one that is already tracing a ray. In the latter
    // case any state related to the ray previously being
    // traced is overwritten.
    //
    // The `rayFlags` here will be bitwise ORed with
    // the `rayFlags` passed as a generic argument to
    // `RayQuery` to get the effective ray flags, which
    // must obey any API-imposed restrictions.
    //
    [__unsafeForceInlineEarly]
    [mutating]
    void TraceRayInline(
        RaytracingAccelerationStructure accelerationStructure,
        RAY_FLAG                        rayFlags,
        uint                            instanceInclusionMask,
        RayDesc                         ray)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".TraceRayInline";
        case glsl:
        case spirv:
            __rayQueryInitializeEXT(
                accelerationStructure,
                rayFlags | rayFlagsGeneric,
                instanceInclusionMask,
                ray.Origin,
                ray.TMin,
                ray.Direction,
                ray.TMax);
        }
    }
    
    // Resume the ray query coroutine.
    //
    // If the coroutine suspends because of encountering
    // a candidate hit that cannot be resolved with fixed-funciton
    // logic, this function returns `true`, and the `Candidate*()`
    // functions should be used by application code to resolve
    // the candidate hit (by either committing or ignoring it).
    //
    // If the coroutine terminates because traversal is
    // complete (or has been aborted), this function returns
    // `false`, and application code should use the `Committed*()`
    // functions to appropriately handle the closest hit (it any)
    // that was found.
    //
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [mutating]
    bool Proceed()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Proceed";
        case glsl: __intrinsic_asm "rayQueryProceedEXT";
        case spirv: return spirv_asm
            {
                result:$$bool = OpRayQueryProceedKHR &this
            };
        }
    }

    // Causes the ray query to terminate.
    //
    // This function cases the ray query to act as if
    // traversal has terminated, so that subsequent
    // `Proceed()` calls will return `false`.
    //
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [mutating]
    void Abort()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Abort";
        case glsl: __intrinsic_asm "rayQueryTerminateEXT";
        case spirv: spirv_asm { OpRayQueryTerminateKHR &this };
        }
    }

    // Commit the current non-opaque triangle hit.
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    [mutating]
    void CommitNonOpaqueTriangleHit()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CommitNonOpaqueTriangleHit";
        case glsl: __intrinsic_asm "rayQueryConfirmIntersectionEXT";
        case spirv: spirv_asm { OpRayQueryConfirmIntersectionKHR &this };
        }
    }

    // Commit the current procedural primitive hit, with hit time `t`.
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    [mutating]
    void CommitProceduralPrimitiveHit(float t)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CommitProceduralPrimitiveHit";
        case glsl: __intrinsic_asm "rayQueryGenerateIntersectionEXT";
        case spirv: spirv_asm { OpRayQueryGenerateIntersectionKHR &this $t };
        }
    }

    // Get the type of candidate hit being considered.
    //
    // The ray query coroutine will suspend when it encounters
    // a hit that cannot be resolved with fixed-function logic
    // (either a non-opaque triangle or a procedural primitive).
    // In either of those cases, `CandidateType()` will return
    // the kind of candidate hit that must be resolved by
    // user code.
    //
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    CANDIDATE_TYPE CandidateType()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CandidateType";
        case glsl: __intrinsic_asm "rayQueryGetIntersectionTypeEXT($0, false)";
        case spirv:
            uint RayQueryCandidateIntersectionKHR = 0;
            return spirv_asm {
                result:$$CANDIDATE_TYPE = OpRayQueryGetIntersectionTypeKHR &this $RayQueryCandidateIntersectionKHR;
            };
        }
    }

    // Get the status of the committed (closest) hit, if any.
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    COMMITTED_STATUS CommittedStatus()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CommittedStatus";
        case glsl: __intrinsic_asm "rayQueryGetIntersectionTypeEXT($0, true)";
        case spirv:
            uint RayQueryCommittedIntersectionKHR = 1;
            return spirv_asm
            {
                result:$$COMMITTED_STATUS = OpRayQueryGetIntersectionTypeKHR &this $RayQueryCommittedIntersectionKHR;
            };
        }
    }

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    bool CandidateProceduralPrimitiveNonOpaque()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CandidateProceduralPrimitiveNonOpaque";
        case glsl: __intrinsic_asm "(!rayQueryGetIntersectionCandidateAABBOpaqueEXT($0, false))";
        case spirv:
            uint iCandidateOrCommitted = 0;
            return spirv_asm
            {
                %rr:$$bool = OpRayQueryGetIntersectionCandidateAABBOpaqueKHR &this;
                result:$$bool = OpLogicalNot %rr;
            };
        }
    }

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    float CandidateTriangleRayT()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CandidateTriangleRayT";
        case glsl: __intrinsic_asm "rayQueryGetIntersectionTEXT($0, false)";
        case spirv:
            uint iCandidateOrCommitted = 0;
            return spirv_asm
            {
                result:$$float = OpRayQueryGetIntersectionTKHR &this $iCandidateOrCommitted;
            };
        }
    }
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    float CommittedRayT()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".CommittedRayT";
        case glsl: __intrinsic_asm "rayQueryGetIntersectionTEXT($0, true)";
        case spirv:
            uint iCandidateOrCommitted = 1;
            return spirv_asm
            {
                result:$$float = OpRayQueryGetIntersectionTKHR &this $iCandidateOrCommitted;
            };
        }
    }

${{{{
    const char* kCandidateCommitted[] = {"Candidate", "Committed"};

    // Access Candidate and Committed Matrices.
    for (uint32_t candidateOrCommitted = 0; candidateOrCommitted < 2; candidateOrCommitted++)
    {
        auto ccName = kCandidateCommitted[candidateOrCommitted];
        auto ccTF = candidateOrCommitted == 0 ? "false" : "true";
}}}}

    // CandidateObjectToWorld3x4, CandidateWorldToObject4x3
    // CommittedObjectToWorld3x4, CommittedObjectToWorld4x3
    ${{{{
    const char* kRayQueryMatrixNames[] = {"ObjectToWorld", "WorldToObject"};
    for (auto matName : kRayQueryMatrixNames) {
    }}}}

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    float3x4 $(ccName)$(matName)3x4()
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "transpose(rayQueryGetIntersection$(matName)EXT($0, $(ccTF)))";
        case hlsl: __intrinsic_asm ".$(ccName)$(matName)3x4";
        case spirv:
            uint iCandidateOrCommitted = $(candidateOrCommitted);
            return spirv_asm {
                %m:$$float4x3 = OpRayQueryGetIntersection$(matName)KHR &this $iCandidateOrCommitted;
                result:$$float3x4 = OpTranspose %m;
            };
        }
    }

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__readNone]
    float4x3 $(ccName)$(matName)4x3()
    {
        __target_switch
        {
        case glsl: __intrinsic_asm "rayQueryGetIntersection$(matName)EXT($0, $(ccTF))";
        case hlsl: __intrinsic_asm ".$(ccName)$(matName)4x3";
        case spirv:
            uint iCandidateOrCommitted = $(candidateOrCommitted);
            return spirv_asm {
                result:$$float4x3 = OpRayQueryGetIntersection$(matName)KHR &this $iCandidateOrCommitted;
            };
        }
    }

${{{{
    } // ObjectToWorld/WorldToObject.

    // Access Candidate and Committed properties.
    struct RayQueryMethodEntry
    {
        const char* type;
        const char* hlslName;
        const char* glslName;
    };
    const RayQueryMethodEntry rayQueryMethods[] = {
        {"uint", "InstanceIndex", "InstanceId"},
        {"uint", "InstanceID", "InstanceCustomIndex"},
        {"uint", "PrimitiveIndex", "PrimitiveIndex"},
        {"uint", "GeometryIndex", "GeometryIndex"},
        {"uint", "InstanceContributionToHitGroupIndex", "InstanceShaderBindingTableRecordOffset"},
        {"float3", "ObjectRayOrigin", "ObjectRayOrigin"},
        {"float3", "ObjectRayDirection", "ObjectRayDirection"},
        {"bool", "TriangleFrontFace", "FrontFace"},
        {"float2", "TriangleBarycentrics", "Barycentrics"},
    };
    for (auto method : rayQueryMethods) {
}}}}

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    $(method.type) $(ccName)$(method.hlslName)()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".$(ccName)$(method.hlslName)";
        case glsl: __intrinsic_asm "rayQueryGetIntersection$(method.glslName)EXT($0, $(ccTF))";
        case spirv:
            uint iCandidateOrCommitted = $(candidateOrCommitted);
            return spirv_asm {
                result:$$$(method.type) = OpRayQueryGetIntersection$(method.glslName)KHR &this $iCandidateOrCommitted;
            };
        }
    }
${{{{
    } // Candidate/Committed properties.
    } // for ("Candidate", "Committed")
}}}}

    // Access properties of the ray being traced.

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    uint RayFlags()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".RayFlags";
        case glsl: __intrinsic_asm "rayQueryGetRayFlagsEXT";
        case spirv:
            return spirv_asm {
                result:$$uint = OpRayQueryGetRayFlagsKHR &this;
            };
        }
    }

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    float3 WorldRayOrigin()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".WorldRayOrigin";
        case glsl: __intrinsic_asm "rayQueryGetWorldRayOriginEXT";
        case spirv:
            return spirv_asm {
                result:$$float3 = OpRayQueryGetWorldRayOriginKHR &this;
            };
        }
    }

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    float3 WorldRayDirection()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".WorldRayDirection";
        case glsl: __intrinsic_asm "rayQueryGetWorldRayDirectionEXT";
        case spirv:
            return spirv_asm {
                result:$$float3 = OpRayQueryGetWorldRayDirectionKHR &this;
            };
        }
    }

    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    [__NoSideEffect]
    float RayTMin()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".RayTMin";
        case glsl: __intrinsic_asm "rayQueryGetRayTMinEXT";
        case spirv:
            return spirv_asm {
                result:$$float = OpRayQueryGetRayTMinKHR &this;
            };
        }
    };
}

//
// Vulkan/SPIR-V specific features
//

struct VkSubpassInput<T>
{
    T SubpassLoad();
}

struct VkSubpassInputMS<T>
{
    T SubpassLoad(int sampleIndex);
}


///
/// Shader Execution Reordering (SER)
///
/// NOTE! This API is currently experimental and may change in the future as SER is made available
/// in different APIs and downstream compilers.
///
/// Based on the NVAPI on D3D12 only currently.
///
/// White paper on SER on NVAPI https://developer.nvidia.com/sites/default/files/akamai/gameworks/ser-whitepaper.pdf
///
/// The NVAPI headers (R520) required for this functionality to work can be found here...
///
/// https://developer.nvidia.com/rtx/path-tracing/nvapi/get-started
///
/// For VK the specification is currently in this PR
///
/// https://github.com/KhronosGroup/GLSL/pull/196/files

/// Internal helper functions

// This is a bit of a hack for GLSL HitObjectAttributes
// It relies on [ForceInline] removing the surrounding function and just inserting the *contained* `t` as a global
// The __ref should indicate the desire for the returned value to not be a copy of t, but *t*.
// In practive __ref doesn't have this effect in practice.
// 
// We need this to be able access the payload outside of a function (which is all that TraceRay for example needs)
// We access the HitObjectAttributes via this function for the desired type, and it acts *as if* it's just an access
// to the global t.
[ForceInline]
Ref<T> __hitObjectAttributes<T>()
{
    [__vulkanHitObjectAttributes]   
    static T t;
    return t;
}
[ForceInline]
Ptr<T> __allocHitObjectAttributes<T>()
{
    [__vulkanHitObjectAttributes]   
    static T t;
    return &t;
}

// Next is the custom intrinsic that will compute the hitObjectAttributes location
// for GLSL-based targets.
//
__generic<Attributes>
__intrinsic_op($(kIROp_GetVulkanRayTracingPayloadLocation))
int __hitObjectAttributesLocation(__ref Attributes attributes);

    /// Immutable data type representing a ray hit or a miss. Can be used to invoke hit or miss shading,
    /// or as a key in ReorderThread. Created by one of several methods described below. HitObject
    /// and its related functions are available in raytracing shader types only.
[__requiresNVAPI]
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GL_EXT_ray_tracing)
[__NonCopyableType]
__intrinsic_type($(kIROp_HitObjectType))
struct HitObject
{
    __intrinsic_op($(kIROp_AllocateOpaqueHandle))
    __init();

        /// Executes ray traversal (including anyhit and intersection shaders) like TraceRay, but returns the
        /// resulting hit information as a HitObject and does not trigger closesthit or miss shaders.
    [ForceInline]
    static HitObject TraceRay<payload_t>(
        RaytracingAccelerationStructure AccelerationStructure,
        uint RayFlags,
        uint InstanceInclusionMask,
        uint RayContributionToHitGroupIndex,
        uint MultiplierForGeometryContributionToHitGroupIndex,
        uint MissShaderIndex,
        RayDesc Ray,
        inout payload_t Payload)
    {
        __target_switch
        {
        case hlsl:
            {
                HitObject hitObj;
                __hlslTraceRay(
                    AccelerationStructure, 
                    RayFlags, 
                    InstanceInclusionMask, 
                    RayContributionToHitGroupIndex, 
                    MultiplierForGeometryContributionToHitGroupIndex, 
                    MissShaderIndex, 
                    Ray, 
                    Payload,
                    hitObj);
                return hitObj;
            }
        case glsl:
            {
                [__vulkanRayPayload]
                static payload_t p;

                // Save the payload
                p = Payload;

                __glslTraceRay(
                    __return_val,
                    AccelerationStructure,
                    RayFlags,                                           // Assumes D3D/VK have some RayFlags values
                    InstanceInclusionMask,                              // cullMask
                    RayContributionToHitGroupIndex,                     // sbtRecordOffset
                    MultiplierForGeometryContributionToHitGroupIndex,   // sbtRecordStride
                    MissShaderIndex,
                    Ray.Origin,
                    Ray.TMin,
                    Ray.Direction, 
                    Ray.TMax,
                    __rayPayloadLocation(p));
        
                // Write the payload out
                Payload = p;
            }
        case spirv:
            {
                [__vulkanRayPayload]
                static payload_t p;

                // Save the payload
                p = Payload;

                let origin = Ray.Origin;
                let direction = Ray.Direction;
                let tmin = Ray.TMin;
                let tmax = Ray.TMax;
                spirv_asm {
                    OpHitObjectTraceRayNV
                        /**/ &__return_val
                        /**/ $AccelerationStructure
                        /**/ $RayFlags
                        /**/ $InstanceInclusionMask
                        /**/ $RayContributionToHitGroupIndex
                        /**/ $MultiplierForGeometryContributionToHitGroupIndex
                        /**/ $MissShaderIndex
                        /**/ $origin
                        /**/ $tmin
                        /**/ $direction
                        /**/ $tmax
                        /**/ &p;
                };

                // Write the payload out
                Payload = p;
            }
        }
    }

        /// Executes motion ray traversal (including anyhit and intersection shaders) like TraceRay, but returns the
        /// resulting hit information as a HitObject and does not trigger closesthit or miss shaders.
    [ForceInline]
    static HitObject TraceMotionRay<payload_t>( 
        RaytracingAccelerationStructure AccelerationStructure, 
        uint RayFlags, 
        uint InstanceInclusionMask, 
        uint RayContributionToHitGroupIndex, 
        uint MultiplierForGeometryContributionToHitGroupIndex, 
        uint MissShaderIndex, 
        RayDesc Ray,
        float CurrentTime,
        inout payload_t Payload)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm "TraceMotionRay";
        case glsl:
            {
                [__vulkanRayPayload]
                static payload_t p;

                // Save the payload
                p = Payload;

                __glslTraceMotionRay(
                    __return_val,
                    AccelerationStructure,
                    RayFlags,                                           // Assumes D3D/VK have some RayFlags values
                    InstanceInclusionMask,                              // cullMask
                    RayContributionToHitGroupIndex,                     // sbtRecordOffset
                    MultiplierForGeometryContributionToHitGroupIndex,   // sbtRecordStride
                    MissShaderIndex,
                    Ray.Origin,
                    Ray.TMin,
                    Ray.Direction, 
                    Ray.TMax,
                    CurrentTime,
                    __rayPayloadLocation(p));
        
                // Write the payload out
                Payload = p;
            }
        case spirv:
            {
                [__vulkanRayPayload]
                static payload_t p;

                // Save the payload
                p = Payload;

                let origin = Ray.Origin;
                let direction = Ray.Direction;
                let tmin = Ray.TMin;
                let tmax = Ray.TMax;
                spirv_asm {
                    OpCapability RayTracingMotionBlurNV;
                    OpExtension "SPV_NV_ray_tracing_motion_blur";
                    OpHitObjectTraceRayMotionNV
                        /**/ &__return_val
                        /**/ $AccelerationStructure
                        /**/ $RayFlags
                        /**/ $InstanceInclusionMask
                        /**/ $RayContributionToHitGroupIndex
                        /**/ $MultiplierForGeometryContributionToHitGroupIndex
                        /**/ $MissShaderIndex
                        /**/ $origin
                        /**/ $tmin
                        /**/ $direction
                        /**/ $tmax
                        /**/ $CurrentTime
                        /**/ &p;
                };
        
                // Write the payload out
                Payload = p;
            }
        }
        
    }

        /// Creates a HitObject representing a hit based on values explicitly passed as arguments, without
        /// tracing a ray. The primitive specified by AccelerationStructure, InstanceIndex, GeometryIndex,
        /// and PrimitiveIndex must exist. The shader table index is computed using the formula used with
        /// TraceRay. The computed index must reference a valid hit group record in the shader table. The
        /// Attributes parameter must either be an attribute struct, such as
        /// BuiltInTriangleIntersectionAttributes, or another HitObject to copy the attributes from.
    [ForceInline]
    static HitObject MakeHit<attr_t>(
        RaytracingAccelerationStructure AccelerationStructure,
        uint InstanceIndex,
        uint GeometryIndex,
        uint PrimitiveIndex,
        uint HitKind,
        uint RayContributionToHitGroupIndex,
        uint MultiplierForGeometryContributionToHitGroupIndex,
        RayDesc Ray,
        attr_t attributes)
    {
        __target_switch
        {
        case hlsl:
            HitObject hitObj;
            __hlslMakeHit(
                AccelerationStructure, 
                InstanceIndex,
                GeometryIndex,
                PrimitiveIndex,
                HitKind,
                RayContributionToHitGroupIndex,
                MultiplierForGeometryContributionToHitGroupIndex,
                Ray,
                attributes,
                hitObj);
            return hitObj;
        case glsl:
            {
                // Save the attributes
                __ref attr_t attr = __hitObjectAttributes<attr_t>();

                attr = attributes;

                __glslMakeHit(
                    __return_val,
                    AccelerationStructure,
                    InstanceIndex,
                    PrimitiveIndex,
                    GeometryIndex,
                    HitKind,
                    RayContributionToHitGroupIndex,                         /// sbtRecordOffset?
                    MultiplierForGeometryContributionToHitGroupIndex,       /// sbtRecordStride?
                    Ray.Origin,
                    Ray.TMin,
                    Ray.Direction, 
                    Ray.TMax,
                    __hitObjectAttributesLocation(__hitObjectAttributes<attr_t>()));
            }
        case spirv:
            {
                // Save the attributes
                Ptr<attr_t> attr = __allocHitObjectAttributes<attr_t>();

                *attr = attributes;

                let origin = Ray.Origin;
                let direction = Ray.Direction;
                let tmin = Ray.TMin;
                let tmax = Ray.TMax;
                spirv_asm {
                    OpHitObjectRecordHitNV
                        /**/ &__return_val
                        /**/ $AccelerationStructure
                        /**/ $InstanceIndex
                        /**/ $PrimitiveIndex
                        /**/ $GeometryIndex
                        /**/ $HitKind
                        /**/ $RayContributionToHitGroupIndex
                        /**/ $MultiplierForGeometryContributionToHitGroupIndex
                        /**/ $origin
                        /**/ $tmin
                        /**/ $direction
                        /**/ $tmax
                        /**/ $attr;
                };
            }
        }
    }

        /// See MakeHit but handles Motion 
        /// Currently only supported on VK
    [ForceInline]
    static HitObject MakeMotionHit<attr_t>( 
        RaytracingAccelerationStructure AccelerationStructure, 
        uint InstanceIndex, 
        uint GeometryIndex, 
        uint PrimitiveIndex, 
        uint HitKind, 
        uint RayContributionToHitGroupIndex, 
        uint MultiplierForGeometryContributionToHitGroupIndex, 
        RayDesc Ray,
        float CurrentTime,
        attr_t attributes)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm "MakeMotionHit";
        case glsl:
        {
            // Save the attributes
            __ref attr_t attr = __hitObjectAttributes<attr_t>();

            attr = attributes;

            __glslMakeMotionHit(
                __return_val,
                AccelerationStructure,
                InstanceIndex,
                PrimitiveIndex,
                GeometryIndex,
                HitKind,
                RayContributionToHitGroupIndex,                         /// sbtRecordOffset?
                MultiplierForGeometryContributionToHitGroupIndex,       /// sbtRecordStride?
                Ray.Origin,
                Ray.TMin,
                Ray.Direction, 
                Ray.TMax,
                CurrentTime,
                __hitObjectAttributesLocation(__hitObjectAttributes<attr_t>()));
        }
        case spirv:
        {
            // Save the attributes
            Ptr<attr_t> attr = __allocHitObjectAttributes<attr_t>();

            *attr = attributes;

            let origin = Ray.Origin;
            let direction = Ray.Direction;
            let tmin = Ray.TMin;
            let tmax = Ray.TMax;
            spirv_asm {
                OpCapability RayTracingMotionBlurNV;
                OpExtension "SPV_NV_ray_tracing_motion_blur";
                OpHitObjectRecordHitMotionNV
                    /**/ &__return_val
                    /**/ $AccelerationStructure
                    /**/ $InstanceIndex
                    /**/ $PrimitiveIndex
                    /**/ $GeometryIndex
                    /**/ $HitKind
                    /**/ $RayContributionToHitGroupIndex
                    /**/ $MultiplierForGeometryContributionToHitGroupIndex
                    /**/ $origin
                    /**/ $tmin
                    /**/ $direction
                    /**/ $tmax
                    /**/ $CurrentTime
                    /**/ $attr;
            };
        }
        }
    }

        /// Creates a HitObject representing a hit based on values explicitly passed as arguments, without
        /// tracing a ray. The primitive specified by AccelerationStructure, InstanceIndex, GeometryIndex,
        /// and PrimitiveIndex must exist. The shader table index is explicitly provided as an argument
        /// instead of being computed from the indexing formula used in TraceRay. The provided index must
        /// reference a valid hit group record in the shader table. The Attributes parameter must either be an
        /// attribute struct, such as BuiltInTriangleIntersectionAttributes, or another HitObject to copy the
        /// attributes from.
    [ForceInline]
    static HitObject MakeHit<attr_t>(
        uint HitGroupRecordIndex,
        RaytracingAccelerationStructure AccelerationStructure,
        uint InstanceIndex,
        uint GeometryIndex,
        uint PrimitiveIndex,
        uint HitKind,
        RayDesc Ray,
        attr_t attributes)
    {
        __target_switch
        {
        case hlsl:
            HitObject hitObj;
            __hlslMakeHitWithRecordIndex(
                HitGroupRecordIndex, 
                AccelerationStructure, 
                InstanceIndex,
                GeometryIndex,
                PrimitiveIndex,
                HitKind,
                Ray,
                attributes,
                hitObj);
            return hitObj;
        case glsl:
        {
            // Save the attributes
            __ref attr_t attr = __hitObjectAttributes<attr_t>();
            attr = attributes;

            __glslMakeHitWithIndex(
                __return_val,
                AccelerationStructure, 
                InstanceIndex,              ///? Same as instanceid ?
                GeometryIndex, 
                PrimitiveIndex,
                HitKind,                    /// Assuming HitKinds are compatible
                HitGroupRecordIndex,        /// sbtRecordIndex
                Ray.Origin,
                Ray.TMin,
                Ray.Direction, 
                Ray.TMax,
                __hitObjectAttributesLocation(__hitObjectAttributes<attr_t>()));
        }
        case spirv:
        {
            // Save the attributes
            Ptr<attr_t> attr = __allocHitObjectAttributes<attr_t>();
            *attr = attributes;
            let origin = Ray.Origin;
            let direction = Ray.Direction;
            let tmin = Ray.TMin;
            let tmax = Ray.TMax;
            spirv_asm {
                OpHitObjectRecordHitWithIndexNV
                    /**/ &__return_val
                    /**/ $AccelerationStructure
                    /**/ $InstanceIndex
                    /**/ $PrimitiveIndex
                    /**/ $GeometryIndex
                    /**/ $HitKind
                    /**/ $HitGroupRecordIndex
                    /**/ $origin
                    /**/ $tmin
                    /**/ $direction
                    /**/ $tmax
                    /**/ $attr;
            };
        }
        }
    }
        /// See MakeHit but handles Motion 
        /// Currently only supported on VK

    [ForceInline]
    static HitObject MakeMotionHit<attr_t>( 
        uint HitGroupRecordIndex, 
        RaytracingAccelerationStructure AccelerationStructure, 
        uint InstanceIndex, 
        uint GeometryIndex, 
        uint PrimitiveIndex, 
        uint HitKind, 
        RayDesc Ray, 
        float CurrentTime,
        attr_t attributes)
    {
        __target_switch
        {
        case glsl:
        {
            // Save the attributes
            __ref attr_t attr = __hitObjectAttributes<attr_t>();
            attr = attributes;

            __glslMakeMotionHitWithIndex(
                __return_val,
                AccelerationStructure, 
                InstanceIndex,              ///? Same as instanceid ?
                GeometryIndex, 
                PrimitiveIndex,
                HitKind,                    /// Assuming HitKinds are compatible
                HitGroupRecordIndex,        /// sbtRecordIndex
                Ray.Origin,
                Ray.TMin,
                Ray.Direction, 
                Ray.TMax,
                CurrentTime,
                __hitObjectAttributesLocation(__hitObjectAttributes<attr_t>()));
        }
        case spirv:
        {
            // Save the attributes
            Ptr<attr_t> attr = __allocHitObjectAttributes<attr_t>();
            *attr = attributes;
            let origin = Ray.Origin;
            let direction = Ray.Direction;
            let tmin = Ray.TMin;
            let tmax = Ray.TMax;
            spirv_asm {
                OpCapability RayTracingMotionBlurNV;
                OpExtension "SPV_NV_ray_tracing_motion_blur";
                OpHitObjectRecordHitWithIndexMotionNV
                    /**/ &__return_val
                    /**/ $AccelerationStructure
                    /**/ $InstanceIndex
                    /**/ $PrimitiveIndex
                    /**/ $GeometryIndex
                    /**/ $HitKind
                    /**/ $HitGroupRecordIndex
                    /**/ $origin
                    /**/ $tmin
                    /**/ $direction
                    /**/ $tmax
                    /**/ $CurrentTime
                    /**/ $attr;
            };
        }
        }
    }

        /// Creates a HitObject representing a miss based on values explicitly passed as arguments, without
        /// tracing a ray. The provided shader table index must reference a valid miss record in the shader
        /// table.
    [__requiresNVAPI]
    [ForceInline]
    static HitObject MakeMiss( 
        uint MissShaderIndex, 
        RayDesc Ray)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm "($2=NvMakeMiss($0,$1))";
        case glsl:
            __glslMakeMiss(__return_val, MissShaderIndex, Ray.Origin, Ray.TMin, Ray.Direction, Ray.TMax);
        case spirv:
            {
                let origin = Ray.Origin;
                let direction = Ray.Direction;
                let tmin = Ray.TMin;
                let tmax = Ray.TMax;
                spirv_asm {
                    OpHitObjectRecordMissNV
                        /**/ &__return_val
                        /**/ $MissShaderIndex
                        /**/ $origin
                        /**/ $tmin
                        /**/ $direction
                        /**/ $tmax;
                };
            }
        }
    }

        /// See MakeMiss but handles Motion 
        /// Currently only supported on VK
    [ForceInline]
    __specialized_for_target(glsl)
    static HitObject MakeMotionMiss( 
        uint MissShaderIndex, 
        RayDesc Ray,
        float CurrentTime)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm "($3=NvMakeMotionMiss($0,$1,$2))";
        case glsl:
            __glslMakeMotionMiss(__return_val, MissShaderIndex, Ray.Origin, Ray.TMin, Ray.Direction, Ray.TMax, CurrentTime);
        case spirv:
            {
                let origin = Ray.Origin;
                let direction = Ray.Direction;
                let tmin = Ray.TMin;
                let tmax = Ray.TMax;
                spirv_asm {
                    OpCapability RayTracingMotionBlurNV;
                    OpExtension "SPV_NV_ray_tracing_motion_blur";
                    OpHitObjectRecordMissMotionNV
                        /**/ &__return_val
                        /**/ $MissShaderIndex
                        /**/ $origin
                        /**/ $tmin
                        /**/ $direction
                        /**/ $tmax
                        /**/ $CurrentTime;
                };
            }
        }
    }

        /// Creates a HitObject representing “NOP” (no operation) which is neither a hit nor a miss. Invoking a
        /// NOP hit object using HitObject::Invoke has no effect. Reordering by hit objects using
        /// ReorderThread will group NOP hit objects together. This can be useful in some reordering
        /// scenarios where future control flow for some threads is known to process neither a hit nor a
        /// miss.
    [__requiresNVAPI]
    [ForceInline]
    static HitObject MakeNop()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm "($0 = NvMakeNop())";
        case glsl:
            __glslMakeNop(__return_val);
        case spirv:
            spirv_asm {
                OpHitObjectRecordEmptyNV
                    /**/ &__return_val;
            };
        }
    }

        /// Invokes closesthit or miss shading for the specified hit object. In case of a NOP HitObject, no
        /// shader is invoked.
    [__requiresNVAPI]
    [ForceInline]
    static void Invoke<payload_t>(
        RaytracingAccelerationStructure AccelerationStructure,
        HitObject HitOrMiss,
        inout payload_t Payload)
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm "NvInvokeHitObject";
        case glsl:
            {
                [__vulkanRayPayload]
                static payload_t p;

                // Save the payload
                p = Payload;

                __glslInvoke(HitOrMiss, __rayPayloadLocation(p));

                // Write payload result
                Payload = p;
            }
        case spirv:
            {
                [__vulkanRayPayload]
                static payload_t p;

                // Save the payload
                p = Payload;

                spirv_asm {
                    OpHitObjectExecuteShaderNV
                        /**/ &HitOrMiss
                        /**/ &p;
                };

                // Write payload result
                Payload = p;
            }
        }
    }

        /// Returns true if the HitObject encodes a miss, otherwise returns false.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    bool IsMiss()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".IsMiss";
        case glsl: __intrinsic_asm "hitObjectIsMissNV($0)";
        case spirv: return spirv_asm {
                result:$$bool = OpHitObjectIsMissNV &this;
            };
        }
    }

        /// Returns true if the HitObject encodes a hit, otherwise returns false.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    bool IsHit()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".IsHit";
        case glsl: __intrinsic_asm "hitObjectIsHitNV($0)";
        case spirv: return spirv_asm {
                result:$$bool = OpHitObjectIsHitNV &this;
            };
        }
    }

        /// Returns true if the HitObject encodes a nop, otherwise returns false.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    bool IsNop()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".IsNop";
        case glsl: __intrinsic_asm "hitObjectIsEmptyNV($0)";
        case spirv: return spirv_asm {
                result:$$bool = OpHitObjectIsEmptyNV &this;
            };
        }
    }

        /// Queries ray properties from HitObject. Valid if the hit object represents a hit or a miss.
    [__requiresNVAPI]
    [ForceInline]
    __target_intrinsic(hlsl)
    RayDesc GetRayDesc()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".GetRayDesc";
        case glsl:
            {
                RayDesc ray = { __glslGetRayWorldOrigin(), __glslGetTMin(), __glslGetRayWorldDirection(), __glslGetTMax() };
                return ray;
            }
        case spirv:
            return spirv_asm {
                %origin:$$float3 = OpHitObjectGetWorldRayOriginNV &this;
                %tmin:$$float = OpHitObjectGetRayTMinNV &this;
                %direction:$$float3 = OpHitObjectGetWorldRayDirectionNV &this;
                %tmax:$$float = OpHitObjectGetRayTMaxNV &this;
                result:$$RayDesc = OpCompositeConstruct %origin %tmin %direction %tmax;
            };
        }
    }

        /// Queries shader table index from HitObject. Valid if the hit object represents a hit or a miss.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    uint GetShaderTableIndex()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetShaderTableIndex";
        case glsl: __intrinsic_asm "hitObjectGetShaderBindingTableRecordIndexNV($0)";
        case spirv: return spirv_asm {
                result:$$uint = OpHitObjectGetShaderBindingTableRecordIndexNV &this;
            };
        }
    }

        /// Returns the instance index of a hit. Valid if the hit object represents a hit.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    uint GetInstanceIndex()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetInstanceIndex";
        case glsl: __intrinsic_asm "hitObjectGetInstanceIdNV($0)";
        case spirv: return spirv_asm {
                result:$$uint = OpHitObjectGetInstanceIdNV &this;
            };
        }
    }

        /// Returns the instance ID of a hit. Valid if the hit object represents a hit.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    uint GetInstanceID()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetInstanceID";
        case glsl: __intrinsic_asm "hitObjectGetInstanceCustomIndexNV($0)";
        case spirv: return spirv_asm {
                result:$$uint = OpHitObjectGetInstanceCustomIndexNV &this;
            };
        }
    }

        /// Returns the geometry index of a hit. Valid if the hit object represents a hit.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    uint GetGeometryIndex()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetGeometryIndex";
        case glsl: __intrinsic_asm "hitObjectGetGeometryIndexNV($0)";
        case spirv: return spirv_asm {
                result:$$uint = OpHitObjectGetGeometryIndexNV &this;
            };
        }
    }

        /// Returns the primitive index of a hit. Valid if the hit object represents a hit.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    uint GetPrimitiveIndex()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetPrimitiveIndex";
        case glsl: __intrinsic_asm "hitObjectGetPrimitiveIndexNV($0)";
        case spirv: return spirv_asm {
                result:$$uint = OpHitObjectGetPrimitiveIndexNV &this;
            };
        }
    }

        /// Returns the hit kind. Valid if the hit object represents a hit.
    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    uint GetHitKind()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetHitKind";
        case glsl: __intrinsic_asm "hitObjectGetHitKindNV($0)";
        case spirv: return spirv_asm {
                result:$$uint = OpHitObjectGetHitKindNV &this;
            };
        }
    }

    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    float4x3 GetWorldToObject()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetWorldToObject";
        case glsl: __intrinsic_asm "hitObjectGetWorldToObjectNV($0)";
        case spirv: return spirv_asm {
                result:$$float4x3 = OpHitObjectGetWorldToObjectNV &this;
            };
        }
    }

    [__requiresNVAPI]
    [ForceInline]
    __glsl_extension(GL_EXT_ray_tracing)
    float4x3 GetObjectToWorld()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".GetObjectToWorld";
        case glsl: __intrinsic_asm "hitObjectGetObjectToWorldNV($0)";
        case spirv: return spirv_asm {
                result:$$float4x3 = OpHitObjectGetObjectToWorldNV &this;
            };
        }
    }

        /// Returns the attributes of a hit. Valid if the hit object represents a hit or a miss.
    [ForceInline]
    attr_t GetAttributes<attr_t>()
    {
        __target_switch
        {
        case hlsl:
            {
                attr_t v;
                __hlslGetAttributesFromHitObject(v);
                return v;
            }
        case glsl:
            {
                // Work out the location
                int attributeLocation = __hitObjectAttributesLocation(__hitObjectAttributes<attr_t>());

                // Load the attributes from the location
                __glslGetAttributes(attributeLocation);

                // Return the attributes
                return __hitObjectAttributes<attr_t>();
            }
        case spirv:
            {
                Ptr<attr_t> attr = __allocHitObjectAttributes<attr_t>();
                spirv_asm {
                    OpHitObjectGetAttributesNV &this $attr;
                };
                return *attr;
            }
        }
    }
        /// Loads a root constant from the local root table referenced by the hit object. Valid if the hit object
        /// represents a hit or a miss. RootConstantOffsetInBytes must be a multiple of 4.
    __target_intrinsic(hlsl)
    [__requiresNVAPI]
    uint LoadLocalRootTableConstant(uint RootConstantOffsetInBytes);

    /// 
    /// !!!! Internal NVAPI HLSL impl. Not part of interface! !!!!!!!!!!!!
    /// 

    __target_intrinsic(hlsl, "NvGetAttributesFromHitObject($0, $1)")
    [__requiresNVAPI]
    void __hlslGetAttributesFromHitObject<T>(out T t);

    __target_intrinsic(hlsl, "NvMakeHitWithRecordIndex")
    [__requiresNVAPI]
    static void __hlslMakeHitWithRecordIndex<attr_t>(
        uint HitGroupRecordIndex, 
        RaytracingAccelerationStructure AccelerationStructure, 
        uint InstanceIndex, 
        uint GeometryIndex, 
        uint PrimitiveIndex, 
        uint HitKind, 
        RayDesc Ray, 
        attr_t attributes, 
        out HitObject hitObj);

    __target_intrinsic(hlsl, "NvMakeHit")
    [__requiresNVAPI]
    static void __hlslMakeHit<attr_t>(RaytracingAccelerationStructure AccelerationStructure, 
        uint InstanceIndex, 
        uint GeometryIndex, 
        uint PrimitiveIndex, 
        uint HitKind, 
        uint RayContributionToHitGroupIndex, 
        uint MultiplierForGeometryContributionToHitGroupIndex, 
        RayDesc Ray, 
        attr_t attributes, 
        out HitObject hitObj);

    __target_intrinsic(hlsl, "NvTraceRayHitObject")
    [__requiresNVAPI]
    static void __hlslTraceRay<payload_t>( 
        RaytracingAccelerationStructure AccelerationStructure, 
        uint RayFlags, 
        uint InstanceInclusionMask, 
        uint RayContributionToHitGroupIndex, 
        uint MultiplierForGeometryContributionToHitGroupIndex, 
        uint MissShaderIndex, 
        RayDesc Ray, 
        inout payload_t Payload,
        out HitObject hitObj);

    /// 
    /// !!!! Internal GLSL GL_NV_shader_invocation_reorder impl. Not part of interface! !!!!!!!!!!!!
    /// 

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_version(460)
    __target_intrinsic(glsl, "hitObjectRecordMissNV")
    static void __glslMakeMiss(
        out HitObject hitObj,
        uint MissShaderIndex,
        float3 Origin,
        float TMin,
        float3 Direction,
        float TMax);

    // "void hitObjectRecordMissNV(hitObjectNV, uint, vec3, float, vec3, float);"
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_ray_tracing_motion_blur)
    __glsl_version(460)
    __target_intrinsic(glsl, "hitObjectRecordMissMotionNV")
    static void __glslMakeMotionMiss(
        out HitObject hitObj,
        uint MissShaderIndex,
        float3 Origin,
        float TMin,
        float3 Direction,
        float TMax, 
        float CurrentTime);

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_EXT_ray_tracing)
    __target_intrinsic(glsl, "hitObjectRecordEmptyNV")
    static void __glslMakeNop(out HitObject hitObj);

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectGetObjectRayDirectionNV($0)")
    float3 __glslGetRayDirection();

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectGetWorldRayDirectionNV($0)")
    float3 __glslGetRayWorldDirection();

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectGetWorldRayOriginNV($0)")
    float3 __glslGetRayWorldOrigin();

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectGetRayTMaxNV($0)")
    float __glslGetTMax();

    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectGetRayTMinNV($0)")
    float __glslGetTMin();

    // "void hitObjectRecordHitWithIndexNV(hitObjectNV, accelerationStructureEXT,int,int,int,uint,uint,vec3,float,vec3,float,int);"
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_version(460)
    __target_intrinsic(glsl, "hitObjectRecordHitWithIndexNV")
    static void __glslMakeHitWithIndex(
        out HitObject hitObj,
        RaytracingAccelerationStructure accelerationStructure,
        int instanceid,
        int primitiveid,
        int geometryindex,
        uint hitKind,
        uint sbtRecordIndex,
        float3 origin,
        float Tmin,
        float3 direction,
        float Tmax,
        int attributeLocation);

    //  "void hitObjectRecordHitWithIndexMotionNV(hitObjectNV, accelerationStructureEXT,int,int,int,uint,uint,vec3,float,vec3,float,float,int);"
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_ray_tracing_motion_blur)
    __target_intrinsic(glsl, "hitObjectRecordHitWithIndexMotionNV")
    static void __glslMakeMotionHitWithIndex(
        out HitObject hitObj,
        RaytracingAccelerationStructure accelerationStructure,
        int instanceid,
        int primitiveid,
        int geometryindex,
        uint hitKind,
        uint sbtRecordIndex,
        float3 origin,
        float Tmin,
        float3 direction,
        float Tmax,
        float CurrentTime,
        int attributeLocation);

    // "void hitObjectRecordHitNV(hitObjectNV,accelerationStructureEXT,int,int,int,uint,uint,uint,vec3,float,vec3,float,int);"
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectRecordHitNV")
    static void __glslMakeHit(
        out HitObject hitObj,
        RaytracingAccelerationStructure accelerationStructure,
        int instanceid,
        int primitiveid,
        int geometryindex,
        uint hitKind,
        uint sbtRecordOffset,
        uint sbtRecordStride,
        float3 origin,
        float Tmin,
        float3 direction,
        float Tmax,
        int attributeLocation);

        // "void hitObjectRecordHitMotionNV(hitObjectNV,accelerationStructureEXT,int,int,int,uint,uint,uint,vec3,float,vec3,float,float,int);"
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_NV_ray_tracing_motion_blur)
    __target_intrinsic(glsl, "hitObjectRecordHitMotionNV")
    static void __glslMakeMotionHit(
        out HitObject hitObj,
        RaytracingAccelerationStructure accelerationStructure,
        int instanceid,
        int primitiveid,
        int geometryindex,
        uint hitKind,
        uint sbtRecordOffset,
        uint sbtRecordStride,
        float3 origin,
        float Tmin,
        float3 direction,
        float Tmax,
        float CurrentTime,
        int attributeLocation);

    
    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectGetAttributesNV($0, $1)")
    void __glslGetAttributes(int attributeLocation);

    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectTraceRayNV")
    static void __glslTraceRay(
        out HitObject hitObject,
        RaytracingAccelerationStructure accelerationStructure,
        uint rayFlags,
        uint cullMask,
        uint sbtRecordOffset,
        uint sbtRecordStride,
        uint missIndex,
        float3 origin,
        float Tmin,
        float3 direction,
        float Tmax,
        int payload);

    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __glsl_extension(GL_NV_ray_tracing_motion_blur)
    __target_intrinsic(glsl, "hitObjectTraceRayMotionNV")
    static void __glslTraceMotionRay(
        out HitObject hitObject,
        RaytracingAccelerationStructure accelerationStructure,
        uint rayFlags,
        uint cullMask,
        uint sbtRecordOffset,
        uint sbtRecordStride,
        uint missIndex,
        float3 origin,
        float Tmin,
        float3 direction,
        float Tmax,
        float currentTime,
        int payload);

    __glsl_extension(GL_EXT_ray_tracing)
    __glsl_extension(GL_NV_shader_invocation_reorder)
    __target_intrinsic(glsl, "hitObjectExecuteShaderNV")
    static void __glslInvoke(
        HitObject hitObj,
        int payload);
};

    /// Reorders threads based on a coherence hint value. NumCoherenceHintBits indicates how many of
    /// the least significant bits of CoherenceHint should be considered during reordering (max: 16).
    /// Applications should set this to the lowest value required to represent all possible values in
    /// CoherenceHint. For best performance, all threads should provide the same value for
    /// NumCoherenceHintBits.
    /// Where possible, reordering will also attempt to retain locality in the thread’s launch indices
    /// (DispatchRaysIndex in DXR).
[__requiresNVAPI]
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GL_EXT_ray_tracing)
void ReorderThread( uint CoherenceHint, uint NumCoherenceHintBitsFromLSB )
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "NvReorderThread";
    case glsl: __intrinsic_asm "reorderThreadNV";
    case spirv:
        spirv_asm {
            OpCapability ShaderInvocationReorderNV;
            OpExtension "SPV_NV_shader_invocation_reorder";
            OpReorderThreadWithHintNV $CoherenceHint $NumCoherenceHintBitsFromLSB;
        };
    }
}

    /// Reorders threads based on a hit object, optionally extended by a coherence hint value. Coherence
    /// hints behave as described in the generic variant of ReorderThread. The maximum number of
    /// coherence hint bits in this variant of ReorderThread is 8. If no coherence hint is desired, set
    /// NumCoherenceHitBits to zero.
    /// Reordering will consider information in the HitObject and coherence hint with the following
    /// priority:
    ///
    /// 1. Shader ID stored in the HitObject
    /// 2. Coherence hint, with the most significant hint bit having highest priority
    /// 3. Spatial information stored in the HitObject
    ///
    /// That is, ReorderThread will first attempt to group threads whose HitObject references the
    /// same shader ID. (Miss shaders and NOP HitObjects are grouped separately). Within each of these
    /// groups, it will attempt to order threads by the value of their coherence hints. And within ranges
    /// of equal coherence hints, it will attempt to maximize locality in 3D space of the ray hit (if any).
[__requiresNVAPI]
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GL_EXT_ray_tracing)
void ReorderThread( HitObject HitOrMiss, uint CoherenceHint, uint NumCoherenceHintBitsFromLSB )
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "NvReorderThread";
    case glsl: __intrinsic_asm "reorderThreadNV";
    case spirv:
        spirv_asm {
            OpReorderThreadWithHitObjectNV &HitOrMiss $CoherenceHint $NumCoherenceHintBitsFromLSB;
        };
    }
}

    /// Is equivalent to
    /// ```
    /// void ReorderThread( HitObject HitOrMiss, uint CoherenceHint, uint NumCoherenceHintBitsFromLSB );
    /// ```
    /// With CoherenceHint and NumCoherenceHintBitsFromLSB as 0, meaning they are ignored.
[__requiresNVAPI]
__glsl_extension(GL_NV_shader_invocation_reorder)
void ReorderThread( HitObject HitOrMiss )
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "NvReorderThread";
    case glsl: __intrinsic_asm "reorderThreadNV";
    case spirv:
        spirv_asm {
            OpReorderThreadWithHitObjectNV &HitOrMiss;
        };
    }
}


///
/// DebugBreak support 
///
/// There doesn't appear to be an equivalent for debugBreak for HLSL

__target_intrinsic(hlsl, "/* debugBreak() not currently supported for HLSL */")
__target_intrinsic(cuda,"__brkpt()")
__target_intrinsic(cpp, "SLANG_BREAKPOINT(0)")
void debugBreak();

__specialized_for_target(glsl)
[[vk::spirv_instruction(1, "NonSemantic.DebugBreak")]]
void debugBreak();

// 
// Realtime Clock support
//

// https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_realtime_clock.txt

[__requiresNVAPI]
__glsl_extension(GL_EXT_shader_realtime_clock)
uint getRealtimeClockLow()
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "NvGetSpecial( NV_SPECIALOP_GLOBAL_TIMER_LO)";
    case glsl:
        return getRealtimeClock().x;
    case cuda:
        __intrinsic_asm "clock";
    case spirv:
        return getRealtimeClock().x;
    }
}

__target_intrinsic(cuda, "clock64")
int64_t __cudaGetRealtimeClock();

[__requiresNVAPI]
__glsl_extension(GL_EXT_shader_realtime_clock)
uint2 getRealtimeClock()
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "uint2(NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO), NvGetSpecial( NV_SPECIALOP_GLOBAL_TIMER_HI))";
    case glsl:
        __intrinsic_asm "clockRealtime2x32EXT()";
    case cuda:
        int64_t ticks = __cudaGetRealtimeClock();
        return uint2(uint(ticks), uint(uint64_t(ticks) >> 32));
    case spirv:
        return spirv_asm
        {
            OpCapability ShaderClockKHR;
            OpExtension "SPV_KHR_shader_clock";
            result : $$uint2 = OpReadClockKHR Device
        };
    }
}

// 
// CUDA specific 
// 

__target_intrinsic(cuda, "(threadIdx)")
[__readNone]
uint3 cudaThreadIdx();

__target_intrinsic(cuda, "(blockIdx)")
[__readNone]
uint3 cudaBlockIdx();

__target_intrinsic(cuda, "(blockDim)")
[__readNone]
uint3 cudaBlockDim();

//
// Workgroup cooperation
//

//
// `saturated_cooperation(c, f, s, u)` will call `f(s, u)` if not all lanes in the
// workgroup are currently executing. however if all lanes are saturated, then
// for each unique `s` across all the active lanes `c(s, u)` is called. The
// return value is the one corresponding to the input `s` from this lane.
//
// Adjacent calls to saturated_cooperation are subject to fusion, i.e.
//      saturated_cooperation(c1, f1, s, u1);
//      saturated_cooperation(c2, f2, s, u2);
// will be transformed to:
//      saturated_cooperation(c1c2, f1f2, s, u1u2);
// where
//      c1c2 is a function which calls c1(s, u1) and then c2(s, u2);
//      f1f2 is a function which calls f1(s, u1) and then f2(s, u2);
//
// When the input differs, calls are fused
//      saturated_cooperation(c1, f1, s1, u1);
//      saturated_cooperation(c2, f2, s2, u2);
// will be transformed to:
//      saturated_cooperation(c1c2, f1f2, s1s2, u1u2);
// where
//      s1s2 is a tuple of s1 and s2
//      c1c2 is a function which calls c1(s1, u1) and then c2(s2, u2);
//      f1f2 is a function which calls f1(s1, u1) and then f2(s2, u2);
// Note that in this case, we will make a call to c1c2 for every unique pair
// s1s2 across all lanes
//
// (This fusion takes place in the fuse-satcoop pass, and as such any changes to
// the signature or behavior of this function should be adjusted for there).
//
[KnownBuiltin("saturated_cooperation")]
func saturated_cooperation<A : __BuiltinType, B, C>(
    cooperate : functype (A, B) -> C,
    fallback : functype (A, B) -> C,
    A input,
    B otherArg)
    -> C
{
    return saturated_cooperation_using(cooperate, fallback, __WaveMatchBuitin<A>, __WaveReadLaneAtBuiltin<A>, input, otherArg);
}

// These two functions are a temporary (circa May 2023) workaround to the fact
// that we can't deduce which overload to pass to saturated_cooperation_using
// in the call above
[__unsafeForceInlineEarly]
func __WaveMatchBuitin<T : __BuiltinType>(T t) -> uint4
{
    return WaveMatch(t);
}
[__unsafeForceInlineEarly]
func __WaveReadLaneAtBuiltin<T : __BuiltinType>(T t, int i) -> T
{
    return WaveReadLaneAt(t, i);
}

//
// saturated_cooperation, but you're able to specify manually the functions:
//
// waveMatch: a function to return a mask of lanes with the same input as this one
// broadcast: a function which returns the value passed into it on the specified lane
//
[KnownBuiltin("saturated_cooperation_using")]
func saturated_cooperation_using<A, B, C>(
    cooperate : functype (A, B) -> C,
    fallback : functype (A, B) -> C,
    waveMatch : functype (A) -> uint4,
    broadcast : functype (A, int) -> A,
    A input,
    B otherArg)
    -> C
{
    const bool isWaveSaturated = WaveActiveCountBits(true) == WaveGetLaneCount();
    if(isWaveSaturated)
    {
        let lanesWithSameInput = waveMatch(input).x;
        // Keep least significant lane in our set
        let ourRepresentative = lanesWithSameInput & -lanesWithSameInput;
        // The representative lanes for all lanes
        var allRepresentatives = WaveActiveBitOr(ourRepresentative);

        C ret;

        // Iterate over set bits in mask from low to high.
        // In each iteration the lowest bit is cleared.
        while(bool(allRepresentatives))
        {
            // Broadcast input across warp.
            let laneIdx = firstbitlow(allRepresentatives);
            let uniformInput = broadcast(input, int(laneIdx));

            // All lanes perform some cooperative computation with dynamic
            // uniform input
            C c = cooperate(uniformInput, otherArg);

            // Update our return value until it
            if(bool(allRepresentatives & ourRepresentative))
                ret = c;

            // Clear the lowest bit
            allRepresentatives &= allRepresentatives - 1;
        }

        return ret;
    }
    else
    {
        return fallback(input, otherArg);
    }
}


${{{{
//
// Texture Footprint Queries
//
// This section introduces the types and methods related
// to the `GL_NV_shader_texture_footprint` GLSL extension,
// and the matching NVAPI operations.
//
// Footprint queries are allowed on both 2D and 3D textures,
// and are structurally similar for the two, so we will
// use a meta-loop to deduplicate the code for the two
// cases.
//
static const struct FootprintTextureShape
{
    int rank;
    char const* suffix;
    char const* texelIndexType;
    char const* sampleCoordsType;
} kFootprintTextureShapes[] =
{
    { 2, "2D", "uint2", "float2" },
    { 3, "3D", "uint3", "float3" },
};
for(auto shape : kFootprintTextureShapes)
{
    auto ND = shape.suffix;
    auto anchorType = shape.texelIndexType;
    auto offsetType = shape.texelIndexType;
    auto coordsType = shape.sampleCoordsType;

// A footprint query yields a data structure
// that describes blocks of texels that
// conservatively cover the data that might
// be fetched in the query.
//
// A given sampling operation might access two
// mip levels of a texture when, e.g., trilinear
// filtering is on. A footprint query may ask for
// a footprint in either the coarse or fine level
// of the pair.
//
// We first define a `struct` type that closely maps
// to how a footprint is defined for each of the
// implementations we support, and then wrap that
// in a derived `struct` that includes the extra
// data that is returned by the GLSL API via the
// function reuslt.
//
}}}}

__glsl_version(450)
__glsl_extension(GL_NV_shader_texture_footprint)
__target_intrinsic(glsl, gl_TextureFootprint$(ND)NV)
__target_intrinsic(hlsl, uint4)
struct __TextureFootprintData$(ND)
{
    typealias Anchor        = $(anchorType);
    typealias Offset        = $(offsetType);
    typealias Mask          = uint2;
    typealias LOD           = uint;
    typealias Granularity   = uint;

    property anchor : Anchor
    {
        [__NoSideEffect]
        [__requiresNVAPI]
        __target_intrinsic(hlsl, NvFootprintExtractAnchorTileLoc$(ND))
        __target_intrinsic(glsl, "$0.anchor")
        get;
    }

    property offset : Offset
    {
        [__NoSideEffect]
        [__requiresNVAPI]
        __target_intrinsic(hlsl, NvFootprintExtractOffset$(ND))
        __target_intrinsic(glsl, "$0.offset")
        get;
    }

    property mask : Mask
    {
        [__NoSideEffect]
        [__requiresNVAPI]
        __target_intrinsic(hlsl, NvFootprintExtractBitmask)
        __target_intrinsic(glsl, "$0.mask")
        get;
    }

    property lod : LOD
    {
        [__NoSideEffect]
        [__requiresNVAPI]
        __target_intrinsic(hlsl, NvFootprintExtractLOD)
        __target_intrinsic(glsl, "$0.lod")
        get;
    }

    property granularity : Granularity
    {
        [__NoSideEffect]
        [__requiresNVAPI]
        __target_intrinsic(hlsl, NvFootprintExtractReturnGran)
        __target_intrinsic(glsl, "$0.granularity")
        get;
    }
}

struct TextureFootprint$(ND) : __TextureFootprintData$(ND)
{
    bool _isSingleLevel;

    property isSingleLevel : bool
    {
        [__NoSideEffect]
        get
        {
            return _isSingleLevel;
        }
    }
}

${
// The NVAPI operations are defined to take the space/register
// indices of their texture and sampler parameters, rather than
// taking the texture/sampler objects directly.
//
// In order to support this approach, we need intrinsics that
// can magically fetch the binding information for a resource.
//
// TODO: These operations are kind of *screaming* for us to
// have a built-in `interface` that all of the opaque resource
// types conform to, so that we can define builtins that work
// for any resource type.
}

__intrinsic_op($(kIROp_GetRegisterSpace)) uint __getRegisterSpace<T>(Texture2D<T> texture);
__intrinsic_op($(kIROp_GetRegisterSpace)) uint __getRegisterSpace<T>(Texture3D<T> texture);
__intrinsic_op($(kIROp_GetRegisterSpace)) uint __getRegisterSpace(SamplerState sampler);

__intrinsic_op($(kIROp_GetRegisterIndex)) uint __getRegisterIndex<T>(Texture2D<T> texture);
__intrinsic_op($(kIROp_GetRegisterIndex)) uint __getRegisterIndex<T>(Texture3D<T> texture);
__intrinsic_op($(kIROp_GetRegisterIndex)) uint __getRegisterIndex(SamplerState sampler);

${
// We define the new operations via an `extension`
// on the relevant texture type(s), rather than
// further clutter the original type declarations.
}

__generic<T>
extension Texture$(ND)<T>
{
${
// We introduce a few convenience type aliases here,
// which both keep our declarations simpler and easier
// to understand, but which might *also* be useful to
// users of the stdlib, so that they can write things
// like `Texture2D.Footprint`, and also have auto-complete
// help them find such members.
//
// TODO: The `Coords` type really ought to be something
// defined on the base texture types, rather than via
// this `extension`.
}
    typealias Coords = $(coordsType);
    typealias Footprint = TextureFootprint$(ND);
    typealias __FootprintData = __TextureFootprintData$(ND);
    typealias FootprintGranularity = Footprint.Granularity;

${
// For the GLSL extension, the choice between the
// coarse and fine level is modeled as a `bool`
// parameter to the query operation(s). We define
// the GLSL functions here as intrinsics, so that
// we can refer to them later in the definitions
// of our stdlib operaitons.
//
// Note: despite the GLSL extension defining the `granularity`
// member of the query result as having type `uint`, the
// function signatures all take `int` parameters for the
// granularity instead.
//
}

    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __target_intrinsic(glsl,
        "textureFootprintNV($p, $*2)")
    bool __queryFootprintGLSL(
            SamplerState    sampler,
            Coords          coords,
            int             granularity,
            bool            useCoarseLevel,
            out __FootprintData footprint);

    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __target_intrinsic(glsl,
        "textureFootprintNV($p, $*2)")
    bool __queryFootprintGLSL(
            SamplerState    sampler,
            Coords          coords,
            int             granularity,
            bool            useCoarseLevel,
            out __FootprintData footprint,
            float           bias);

    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __glsl_extension(GL_ARB_sparse_texture_clamp)
    __target_intrinsic(glsl,
        "textureFootprintClampNV($p, $*2)")
    bool __queryFootprintClampGLSL(
            SamplerState    sampler,
            Coords          coords,
            float           lodClamp,
            int             granularity,
            bool            useCoarseLevel,
            out __FootprintData footprint);

    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __glsl_extension(GL_ARB_sparse_texture_clamp)
    __target_intrinsic(glsl,
        "textureFootprintClampNV($p, $*2)")
    bool __queryFootprintClampGLSL(
            SamplerState    sampler,
            Coords          coords,
            float           lodClamp,
            int             granularity,
            bool            useCoarseLevel,
            out __FootprintData footprint,
            float           bias);

    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __target_intrinsic(glsl,
        "textureFootprintLodNV($p, $*2)")
    [__requiresNVAPI]
    bool __queryFootprintLodGLSL(
            SamplerState            sampler,
            Coords                  coords,
            float                   lod,
            int                     granularity,
            bool                    useCoarseLevel,
            out __FootprintData         footprint);


${{{
    // Texture sampling with gradient is only available for 2D textures.
    if(shape.rank == 2) {
}}}
    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __target_intrinsic(glsl,
        "textureFootprintGradNV($p, $*2)")
    [__requiresNVAPI]
    bool __queryFootprintGradGLSL(
            SamplerState    sampler,
            Coords          coords,
            Coords          dx,
            Coords          dy,
            int             granularity,
            bool            useCoarseLevel,
            out __FootprintData footprint);

    [__NoSideEffect]
    __glsl_version(450)
    __glsl_extension(GL_NV_shader_texture_footprint)
    __glsl_extension(GL_ARB_sparse_texture_clamp)
    __target_intrinsic(glsl,
        "textureFootprintGradClampNV($p, $*2)")
    bool __queryFootprintGradClampGLSL(
            SamplerState    sampler,
            Coords          coords,
            Coords          dx,
            Coords          dy,
            float           lodClamp,
            int             granularity,
            bool            useCoarseLevel,
            out __FootprintData footprint);
${{{
    }
}}}


${{{{
// The NVAPI texture query operations encode the choice
// between coarse and fine levels as part of the function
// name, and so we are forced to match this convention
// if we want to provide a more portable API.
//
// TODO: We could conceivably define the functions to use
// a parameter for the coarse/fine choice, which is required
// to be `constexpr` for the HLSL/NVAPI target.
//
static const struct LevelChoice
{
char const* name;
char const* isCoarseVal;
} kLevelChoices[] =
{
    { "Coarse", "true" },
    { "Fine", "false" },
};
for(auto levelChoice : kLevelChoices)
{
    auto CoarseOrFine = levelChoice.name;
    auto isCoarseVal = levelChoice.isCoarseVal;

// We now go ahead and define the intrinsics provided by NVAPI,
// which have a very different signature from the GLSL ones.
//
// Note: the NVAPI functions also support an optional texel
// offset parameter. For now we are not including overloads
// with that parameter, since they have no equivalent in
// the GLSL extension.
//
}}}}

    [__NoSideEffect]
    [__requiresNVAPI]
    __target_intrinsic(hlsl,
        "NvFootprint$(CoarseOrFine)($0, $1, $2, $3, NV_EXTN_TEXTURE_$(ND), $*4)")
    static __FootprintData __queryFootprint$(CoarseOrFine)NVAPI(
        uint                    textureSpace,
        uint                    textureIndex,
        uint                    samplerSpace,
        uint                    samplerIndex,
        float3                  coords,
        FootprintGranularity    granularity, 
        out uint                isSingleLod);

    [__NoSideEffect]
    [__requiresNVAPI]
    __target_intrinsic(hlsl,
        "NvFootprint$(CoarseOrFine)Bias($0, $1, $2, $3, NV_EXTN_TEXTURE_$(ND), $*4)")
    static __FootprintData __queryFootprint$(CoarseOrFine)BiasNVAPI(
        uint                    textureSpace,
        uint                    textureIndex,
        uint                    samplerSpace,
        uint                    samplerIndex,
        float3                  coords,
        FootprintGranularity    granularity,
        float                   lodBias, 
        out uint                isSingleLod);

    [__NoSideEffect]
    [__requiresNVAPI]
    __target_intrinsic(hlsl,
        "NvFootprint$(CoarseOrFine)Level($0, $1, $2, $3, NV_EXTN_TEXTURE_$(ND), $*4)")
    static __FootprintData __queryFootprint$(CoarseOrFine)LevelNVAPI(
        uint                    textureSpace,
        uint                    textureIndex,
        uint                    samplerSpace,
        uint                    samplerIndex,
        float3                  coords,
        FootprintGranularity    granularity,
        float                   lod, 
        out uint                isSingleLod);

${{{
    // Texture sampling with gradient is only available for 2D textures.
    if(shape.rank == 2) {
}}}
    [__NoSideEffect]
    [__requiresNVAPI]
    __target_intrinsic(hlsl,
        "NvFootprint$(CoarseOrFine)Grad($0, $1, $2, $3, NV_EXTN_TEXTURE_$(ND), $*4)")
    static __FootprintData __queryFootprint$(CoarseOrFine)GradNVAPI(
        uint                    textureSpace,
        uint                    textureIndex,
        uint                    samplerSpace,
        uint                    samplerIndex,
        float3                  coords,
        FootprintGranularity    granularity,
        float3                  dx,
        float3                  dy, 
        out uint                isSingleLod);
${{{
    }
}}}

${
// We now define the portable operations that will be officially
// supported by the standard library. For each operation, we
// need to provide both a version that maps to the GLSL extension,
// and a version that uses the NVAPI functions.
//
// Some function variations are only available with one extension
// or the other, so we try our best to only define them where
// each is available.
}

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.Sample(sampler, coords);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintGLSL(sampler, coords, granularity, $(isCoarseVal), footprint);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.Sample(sampler, coords);
    ///
    [__NoSideEffect]
    __specialized_for_target(hlsl)
    Footprint queryFootprint$(CoarseOrFine)(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords)
    {
        uint isSingleLod = 0;
        Footprint footprint =  {__queryFootprint$(CoarseOrFine)NVAPI(
            __getRegisterSpace(this), __getRegisterIndex(this),
            __getRegisterSpace(sampler), __getRegisterIndex(sampler),
            __asFloat3(coords), granularity, /* out */isSingleLod), false};
        footprint._isSingleLevel = (isSingleLod != 0);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleBias(sampler, coords, lodBias);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)Bias(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            float                   lodBias)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintGLSL(sampler, coords, granularity, $(isCoarseVal), footprint, lodBias);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleBias(sampler, coords, lodBias);
    ///
    [__NoSideEffect]
    __specialized_for_target(hlsl)
    Footprint queryFootprint$(CoarseOrFine)Bias(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            float                   lodBias)
    {
        uint isSingleLod = 0;
        Footprint footprint = {__queryFootprint$(CoarseOrFine)BiasNVAPI(
            __getRegisterSpace(this), __getRegisterIndex(this),
            __getRegisterSpace(sampler), __getRegisterIndex(sampler),
            __asFloat3(coords), granularity, lodBias, /* out */isSingleLod), false}; 
            
        footprint._isSingleLevel = (isSingleLod != 0);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleClamp(sampler, coords, lodClamp);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)Clamp(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            float                   lodClamp)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintClampGLSL(sampler, coords, lodClamp, granularity, $(isCoarseVal), footprint);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleBiasClamp(sampler, coords, lodBias, lodClamp);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)BiasClamp(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            float                   lodBias,
            float                   lodClamp)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintClampGLSL(sampler, coords, lodClamp, granularity, $(isCoarseVal), footprint, lodBias);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleLevel(sampler, coords, lod);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)Level(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            float                   lod)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintLodGLSL(sampler, coords, lod, granularity, $(isCoarseVal), footprint);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleLevel(sampler, coords, lod);
    ///
    [__NoSideEffect]
    __specialized_for_target(hlsl)
    Footprint queryFootprint$(CoarseOrFine)Level(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            float                   lod)
    {
        uint isSingleLod = 0;
        Footprint footprint = {__queryFootprint$(CoarseOrFine)LevelNVAPI(
            __getRegisterSpace(this), __getRegisterIndex(this),
            __getRegisterSpace(sampler), __getRegisterIndex(sampler),
            __asFloat3(coords), granularity, lod, /* out */isSingleLod), false};
            
        footprint._isSingleLevel = (isSingleLod != 0);
        return footprint;
    }


${{{
    // Texture sampling with gradient is only available for 2D textures.
    if(shape.rank == 2) {
}}}

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleGrad(sampler, coords, dx, dy);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)Grad(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            Coords                  dx,
            Coords                  dy)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintGradGLSL(sampler, coords, dx, dy, granularity, $(isCoarseVal), footprint);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleGrad(sampler, coords, dx, dy);
    ///
    [__NoSideEffect]
    __specialized_for_target(hlsl)
    Footprint queryFootprint$(CoarseOrFine)Grad(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            Coords                  dx,
            Coords                  dy)
    {
        uint isSingleLod = 0;
        Footprint footprint = {__queryFootprint$(CoarseOrFine)GradNVAPI(
            __getRegisterSpace(this), __getRegisterIndex(this),
            __getRegisterSpace(sampler), __getRegisterIndex(sampler),
            __asFloat3(coords), granularity, __asFloat3(dx), __asFloat3(dy), /* out */isSingleLod), false};

        footprint._isSingleLevel = (isSingleLod != 0);
        return footprint;
    }

    /// Query the footprint that would be accessed by a texture sampling operation.
    ///
    /// This operation queries the footprint that would be accessed
    /// by a comparable call to:
    ///
    ///     t.SampleGradClamp(sampler, coords, dx, dy, lodClamp);
    ///
    [__NoSideEffect]
    __specialized_for_target(glsl)
    Footprint queryFootprint$(CoarseOrFine)GradClamp(
            FootprintGranularity    granularity,
            SamplerState            sampler,
            Coords                  coords,
            Coords                  dx,
            Coords                  dy,
            float                   lodClamp)
    {
        Footprint footprint;
        footprint._isSingleLevel = __queryFootprintGradClampGLSL(sampler, coords, dx, dy, lodClamp, granularity, $(isCoarseVal), footprint);
        return footprint;
    }

${{{
    } // if(shape.rank == 2)
}}}

${{{{
}
}}}}

} // extension

${{{{
}
}}}}

// Buffer Pointer

__generic<T, let Alignment : int = 16>
__intrinsic_type($(kIROp_HLSLConstBufferPointerType))
__glsl_extension(GL_EXT_buffer_reference)
__magic_type(ConstBufferPointerType)
struct ConstBufferPointer
{
    __glsl_version(450)
    __glsl_extension(GL_EXT_buffer_reference)
    __target_intrinsic(glsl, "$0._data")
    [__NoSideEffect]
    T get();
}
