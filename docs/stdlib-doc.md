--------------------------------------------------------------------------------
# __BuiltinArithmeticType.init

## Signature 

```
__BuiltinArithmeticType.init(int value);
```

## Target Availability

HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# __BuiltinFloatingPointType.init

## Signature 

```
__BuiltinFloatingPointType.init(float value);
```

## Target Availability

HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# __BuiltinFloatingPointType.getPi

## Description

Get the value of the mathematical constant pi in this type.

## Signature 

```
__BuiltinFloatingPointType.This __BuiltinFloatingPointType.getPi();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# struct ConstantBuffer<T>

## Generic Parameters

* _T_ 


--------------------------------------------------------------------------------
# struct TextureBuffer<T>

## Generic Parameters

* _T_ 


--------------------------------------------------------------------------------
# struct ParameterBlock<T>

## Generic Parameters

* _T_ 


--------------------------------------------------------------------------------
# struct SamplerState


--------------------------------------------------------------------------------
# struct SamplerComparisonState


--------------------------------------------------------------------------------
# struct SamplerTexture1D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture1D<T>.init

## Signature 

```
SamplerTexture1D<T>.init(
    Texture1D            t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture1D     t,
    vector<float,1>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture1D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture1D<T>.init

## Signature 

```
SamplerRWTexture1D<T>.init(
    RWTexture1D          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture1D   t,
    vector<float,1>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture1D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture1D<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture1D<T>.init(
    RasterizerOrderedTexture1D t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture1D t,
    vector<float,1>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture1DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture1DMS<T>.init

## Signature 

```
SamplerTexture1DMS<T>.init(
    Texture1DMS          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture1DMS   t,
    vector<float,1>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture1DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture1DMS<T>.init

## Signature 

```
SamplerRWTexture1DMS<T>.init(
    RWTexture1DMS        t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture1DMS t,
    vector<float,1>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture1DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture1DMS<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture1DMS<T>.init(
    RasterizerOrderedTexture1DMS t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture1DMS t,
    vector<float,1>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture1DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture1DArray<T>.init

## Signature 

```
SamplerTexture1DArray<T>.init(
    Texture1DArray       t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture1DArray t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture1DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture1DArray<T>.init

## Signature 

```
SamplerRWTexture1DArray<T>.init(
    RWTexture1DArray     t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture1DArray t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture1DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture1DArray<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture1DArray<T>.init(
    RasterizerOrderedTexture1DArray t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture1DArray t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture1DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture1DMSArray<T>.init

## Signature 

```
SamplerTexture1DMSArray<T>.init(
    Texture1DMSArray     t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture1DMSArray t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture1DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture1DMSArray<T>.init

## Signature 

```
SamplerRWTexture1DMSArray<T>.init(
    RWTexture1DMSArray   t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture1DMSArray t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture1DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture1DMSArray<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture1DMSArray<T>.init(
    RasterizerOrderedTexture1DMSArray t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture1DMSArray t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture2D<T>.init

## Signature 

```
SamplerTexture2D<T>.init(
    Texture2D            t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture2D     t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture2D<T>.init

## Signature 

```
SamplerRWTexture2D<T>.init(
    RWTexture2D          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture2D   t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture2D<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture2D<T>.init(
    RasterizerOrderedTexture2D t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture2D t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture2DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture2DMS<T>.init

## Signature 

```
SamplerTexture2DMS<T>.init(
    Texture2DMS          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture2DMS   t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture2DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture2DMS<T>.init

## Signature 

```
SamplerRWTexture2DMS<T>.init(
    RWTexture2DMS        t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture2DMS t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture2DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture2DMS<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture2DMS<T>.init(
    RasterizerOrderedTexture2DMS t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture2DMS t,
    vector<float,2>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture2DArray<T>.init

## Signature 

```
SamplerTexture2DArray<T>.init(
    Texture2DArray       t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture2DArray t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture2DArray<T>.init

## Signature 

```
SamplerRWTexture2DArray<T>.init(
    RWTexture2DArray     t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture2DArray t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture2DArray<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture2DArray<T>.init(
    RasterizerOrderedTexture2DArray t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture2DArray t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture2DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture2DMSArray<T>.init

## Signature 

```
SamplerTexture2DMSArray<T>.init(
    Texture2DMSArray     t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture2DMSArray t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture2DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture2DMSArray<T>.init

## Signature 

```
SamplerRWTexture2DMSArray<T>.init(
    RWTexture2DMSArray   t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture2DMSArray t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture2DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture2DMSArray<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture2DMSArray<T>.init(
    RasterizerOrderedTexture2DMSArray t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture2DMSArray t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture3D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture3D<T>.init

## Signature 

```
SamplerTexture3D<T>.init(
    Texture3D            t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture3D     t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture3D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture3D<T>.init

## Signature 

```
SamplerRWTexture3D<T>.init(
    RWTexture3D          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture3D   t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture3D<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture3D<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture3D<T>.init(
    RasterizerOrderedTexture3D t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture3D t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTexture3DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTexture3DMS<T>.init

## Signature 

```
SamplerTexture3DMS<T>.init(
    Texture3DMS          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTexture3DMS   t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRWTexture3DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRWTexture3DMS<T>.init

## Signature 

```
SamplerRWTexture3DMS<T>.init(
    RWTexture3DMS        t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRWTexture3DMS t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTexture3DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTexture3DMS<T>.init

## Signature 

```
SamplerRasterizerOrderedTexture3DMS<T>.init(
    RasterizerOrderedTexture3DMS t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTexture3DMS t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTextureCube<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTextureCube<T>.init

## Signature 

```
SamplerTextureCube<T>.init(
    TextureCube          t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTextureCube   t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTextureCube<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTextureCube<T>.init

## Signature 

```
SamplerRasterizerOrderedTextureCube<T>.init(
    RasterizerOrderedTextureCube t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTextureCube t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTextureCubeMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTextureCubeMS<T>.init

## Signature 

```
SamplerTextureCubeMS<T>.init(
    TextureCubeMS        t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTextureCubeMS t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTextureCubeMS<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTextureCubeMS<T>.init

## Signature 

```
SamplerRasterizerOrderedTextureCubeMS<T>.init(
    RasterizerOrderedTextureCubeMS t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTextureCubeMS t,
    vector<float,3>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTextureCubeArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTextureCubeArray<T>.init

## Signature 

```
SamplerTextureCubeArray<T>.init(
    TextureCubeArray     t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTextureCubeArray t,
    vector<float,4>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTextureCubeArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTextureCubeArray<T>.init

## Signature 

```
SamplerRasterizerOrderedTextureCubeArray<T>.init(
    RasterizerOrderedTextureCubeArray t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTextureCubeArray t,
    vector<float,4>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerTextureCubeMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerTextureCubeMSArray<T>.init

## Signature 

```
SamplerTextureCubeMSArray<T>.init(
    TextureCubeMSArray   t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerTextureCubeMSArray t,
    vector<float,4>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct SamplerRasterizerOrderedTextureCubeMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _init_ 


--------------------------------------------------------------------------------
# SamplerRasterizerOrderedTextureCubeMSArray<T>.init

## Signature 

```
SamplerRasterizerOrderedTextureCubeMSArray<T>.init(
    RasterizerOrderedTextureCubeMSArray t,
    SamplerState         s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 
* _s_ 

--------------------------------------------------------------------------------
# texture<T>

## Signature 

```
T texture<T>(
    SamplerRasterizerOrderedTextureCubeMSArray t,
    vector<float,4>      location);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _t_ 
* _location_ 

--------------------------------------------------------------------------------
# struct Texture1D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# Texture1D<T>.CalculateLevelOfDetail

## Signature 

```
float Texture1D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture1D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float Texture1D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture1D<T>.GetDimensions

## Signature 

```
void Texture1D<T>.GetDimensions(out uint width);
void Texture1D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             numberOfLevels);
void Texture1D<T>.GetDimensions(out float width);
void Texture1D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture1D<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture1D<T>.Load(vector<int,2> location);
/// See Target Availability 2
T Texture1D<T>.Load(
    vector<int,2>        location,
    vector<int,1>        offset);
/// See Target Availability 3
T Texture1D<T>.Load(
    vector<int,2>        location,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture1D<T>.subscript

## Signature 

```
T Texture1D<T>.subscript(uint location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# Texture1D<T>.Sample

## Signature 

```
/// See Target Availability 1
T Texture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location);
/// See Target Availability 2
T Texture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 3
T Texture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    float                clamp);
T Texture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture1D<T>.SampleBias

## Signature 

```
T Texture1D<T>.SampleBias(
    SamplerState         s,
    vector<float,1>      location,
    float                bias);
T Texture1D<T>.SampleBias(
    SamplerState         s,
    vector<float,1>      location,
    float                bias,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture1D<T>.SampleCmp

## Signature 

```
float Texture1D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue);
float Texture1D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture1D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float Texture1D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue);
/// See Target Availability 2
float Texture1D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture1D<T>.SampleGrad

## Signature 

```
T Texture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY);
T Texture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset);
T Texture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# Texture1D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T Texture1D<T>.SampleLevel(
    SamplerState         s,
    vector<float,1>      location,
    float                level);
/// See Target Availability 2
T Texture1D<T>.SampleLevel(
    SamplerState         s,
    vector<float,1>      location,
    float                level,
    vector<int,1>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension Texture1D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RWTexture1D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RWTexture1D<T>.CalculateLevelOfDetail

## Signature 

```
float RWTexture1D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RWTexture1D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.GetDimensions

## Signature 

```
void RWTexture1D<T>.GetDimensions(out uint width);
void RWTexture1D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             numberOfLevels);
void RWTexture1D<T>.GetDimensions(out float width);
void RWTexture1D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture1D<T>.Load(vector<int,1> location);
/// See Target Availability 2
T RWTexture1D<T>.Load(
    vector<int,1>        location,
    vector<int,1>        offset);
/// See Target Availability 3
T RWTexture1D<T>.Load(
    vector<int,1>        location,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.subscript

## Signature 

```
T RWTexture1D<T>.subscript(uint location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.Sample

## Signature 

```
/// See Target Availability 1
T RWTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location);
/// See Target Availability 2
T RWTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 3
T RWTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    float                clamp);
T RWTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.SampleBias

## Signature 

```
T RWTexture1D<T>.SampleBias(
    SamplerState         s,
    vector<float,1>      location,
    float                bias);
T RWTexture1D<T>.SampleBias(
    SamplerState         s,
    vector<float,1>      location,
    float                bias,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.SampleCmp

## Signature 

```
float RWTexture1D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue);
float RWTexture1D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RWTexture1D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue);
/// See Target Availability 2
float RWTexture1D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.SampleGrad

## Signature 

```
T RWTexture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY);
T RWTexture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset);
T RWTexture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RWTexture1D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RWTexture1D<T>.SampleLevel(
    SamplerState         s,
    vector<float,1>      location,
    float                level);
/// See Target Availability 2
T RWTexture1D<T>.SampleLevel(
    SamplerState         s,
    vector<float,1>      location,
    float                level,
    vector<int,1>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RWTexture1D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture1D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTexture1D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTexture1D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture1D<T>.GetDimensions(out uint width);
void RasterizerOrderedTexture1D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             numberOfLevels);
void RasterizerOrderedTexture1D<T>.GetDimensions(out float width);
void RasterizerOrderedTexture1D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1D<T>.Load(vector<int,1> location);
T RasterizerOrderedTexture1D<T>.Load(
    vector<int,1>        location,
    vector<int,1>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture1D<T>.Load(
    vector<int,1>        location,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.subscript

## Signature 

```
T RasterizerOrderedTexture1D<T>.subscript(uint location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location);
/// See Target Availability 2
T RasterizerOrderedTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 3
T RasterizerOrderedTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    float                clamp);
T RasterizerOrderedTexture1D<T>.Sample(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.SampleBias

## Signature 

```
T RasterizerOrderedTexture1D<T>.SampleBias(
    SamplerState         s,
    vector<float,1>      location,
    float                bias);
T RasterizerOrderedTexture1D<T>.SampleBias(
    SamplerState         s,
    vector<float,1>      location,
    float                bias,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.SampleCmp

## Signature 

```
float RasterizerOrderedTexture1D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue);
float RasterizerOrderedTexture1D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RasterizerOrderedTexture1D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue);
/// See Target Availability 2
float RasterizerOrderedTexture1D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,1>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTexture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY);
T RasterizerOrderedTexture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset);
T RasterizerOrderedTexture1D<T>.SampleGrad(
    SamplerState         s,
    vector<float,1>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1D<T>.SampleLevel(
    SamplerState         s,
    vector<float,1>      location,
    float                level);
/// See Target Availability 2
T RasterizerOrderedTexture1D<T>.SampleLevel(
    SamplerState         s,
    vector<float,1>      location,
    float                level,
    vector<int,1>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<T,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<float,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<int,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.Gather(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherRed(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherGreen(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherBlue(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location);
vector<uint,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1D.GatherAlpha(
    SamplerState         s,
    vector<float,1>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct Texture1DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# Texture1DMS<T>.GetDimensions

## Signature 

```
void Texture1DMS<T>.GetDimensions(
    out uint             width,
    out uint             sampleCount);
void Texture1DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             sampleCount,
    out uint             numberOfLevels);
void Texture1DMS<T>.GetDimensions(
    out float            width,
    out float            sampleCount);
void Texture1DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture1DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> Texture1DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# Texture1DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex);
T Texture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex,
    vector<int,1>        offset);
/// See Target Availability 2
T Texture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture1DMS<T>.subscript

## Signature 

```
T Texture1DMS<T>.subscript(uint location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RWTexture1DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RWTexture1DMS<T>.GetDimensions

## Signature 

```
void RWTexture1DMS<T>.GetDimensions(
    out uint             width,
    out uint             sampleCount);
void RWTexture1DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RWTexture1DMS<T>.GetDimensions(
    out float            width,
    out float            sampleCount);
void RWTexture1DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture1DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RWTexture1DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RWTexture1DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex);
T RWTexture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex,
    vector<int,1>        offset);
/// See Target Availability 2
T RWTexture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture1DMS<T>.subscript

## Signature 

```
T RWTexture1DMS<T>.subscript(uint location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture1DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMS<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture1DMS<T>.GetDimensions(
    out uint             width,
    out uint             sampleCount);
void RasterizerOrderedTexture1DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTexture1DMS<T>.GetDimensions(
    out float            width,
    out float            sampleCount);
void RasterizerOrderedTexture1DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTexture1DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex);
T RasterizerOrderedTexture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex,
    vector<int,1>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture1DMS<T>.Load(
    vector<int,1>        location,
    int                  sampleIndex,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMS<T>.subscript

## Signature 

```
T RasterizerOrderedTexture1DMS<T>.subscript(uint location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct Texture1DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# Texture1DArray<T>.CalculateLevelOfDetail

## Signature 

```
float Texture1DArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float Texture1DArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.GetDimensions

## Signature 

```
void Texture1DArray<T>.GetDimensions(
    out uint             width,
    out uint             elements);
void Texture1DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             elements,
    out uint             numberOfLevels);
void Texture1DArray<T>.GetDimensions(
    out float            width,
    out float            elements);
void Texture1DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture1DArray<T>.Load(vector<int,3> location);
T Texture1DArray<T>.Load(
    vector<int,3>        location,
    vector<int,1>        offset);
/// See Target Availability 2
T Texture1DArray<T>.Load(
    vector<int,3>        location,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.subscript

## Signature 

```
T Texture1DArray<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T Texture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
T Texture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 3
T Texture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    float                clamp);
T Texture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.SampleBias

## Signature 

```
T Texture1DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias);
T Texture1DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.SampleCmp

## Signature 

```
float Texture1DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
float Texture1DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float Texture1DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
/// See Target Availability 2
float Texture1DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.SampleGrad

## Signature 

```
T Texture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY);
T Texture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset);
T Texture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# Texture1DArray<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T Texture1DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level);
/// See Target Availability 2
T Texture1DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level,
    vector<int,1>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension Texture1DArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> Texture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RWTexture1DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RWTexture1DArray<T>.CalculateLevelOfDetail

## Signature 

```
float RWTexture1DArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RWTexture1DArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.GetDimensions

## Signature 

```
void RWTexture1DArray<T>.GetDimensions(
    out uint             width,
    out uint             elements);
void RWTexture1DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             elements,
    out uint             numberOfLevels);
void RWTexture1DArray<T>.GetDimensions(
    out float            width,
    out float            elements);
void RWTexture1DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture1DArray<T>.Load(vector<int,2> location);
/// See Target Availability 2
T RWTexture1DArray<T>.Load(
    vector<int,2>        location,
    vector<int,1>        offset);
/// See Target Availability 3
T RWTexture1DArray<T>.Load(
    vector<int,2>        location,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.subscript

## Signature 

```
T RWTexture1DArray<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T RWTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
T RWTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 3
T RWTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    float                clamp);
T RWTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.SampleBias

## Signature 

```
T RWTexture1DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias);
T RWTexture1DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.SampleCmp

## Signature 

```
float RWTexture1DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
float RWTexture1DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RWTexture1DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
/// See Target Availability 2
float RWTexture1DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.SampleGrad

## Signature 

```
T RWTexture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY);
T RWTexture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset);
T RWTexture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RWTexture1DArray<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RWTexture1DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level);
/// See Target Availability 2
T RWTexture1DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level,
    vector<int,1>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RWTexture1DArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture1DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTexture1DArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTexture1DArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,1>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture1DArray<T>.GetDimensions(
    out uint             width,
    out uint             elements);
void RasterizerOrderedTexture1DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             elements,
    out uint             numberOfLevels);
void RasterizerOrderedTexture1DArray<T>.GetDimensions(
    out float            width,
    out float            elements);
void RasterizerOrderedTexture1DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1DArray<T>.Load(vector<int,2> location);
T RasterizerOrderedTexture1DArray<T>.Load(
    vector<int,2>        location,
    vector<int,1>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture1DArray<T>.Load(
    vector<int,2>        location,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.subscript

## Signature 

```
T RasterizerOrderedTexture1DArray<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
T RasterizerOrderedTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 3
T RasterizerOrderedTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    float                clamp);
T RasterizerOrderedTexture1DArray<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.SampleBias

## Signature 

```
T RasterizerOrderedTexture1DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias);
T RasterizerOrderedTexture1DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.SampleCmp

## Signature 

```
float RasterizerOrderedTexture1DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
float RasterizerOrderedTexture1DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RasterizerOrderedTexture1DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
/// See Target Availability 2
float RasterizerOrderedTexture1DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,1>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTexture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY);
T RasterizerOrderedTexture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset);
T RasterizerOrderedTexture1DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,1>      gradX,
    vector<float,1>      gradY,
    vector<int,1>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level);
/// See Target Availability 2
T RasterizerOrderedTexture1DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level,
    vector<int,1>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1DArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<T,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<float,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<int,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture1DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
vector<uint,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture1DArray.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,1>        offset1,
    vector<int,1>        offset2,
    vector<int,1>        offset3,
    vector<int,1>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct Texture1DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# Texture1DMSArray<T>.GetDimensions

## Signature 

```
void Texture1DMSArray<T>.GetDimensions(
    out uint             width,
    out uint             elements,
    out uint             sampleCount);
void Texture1DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void Texture1DMSArray<T>.GetDimensions(
    out float            width,
    out float            elements,
    out float            sampleCount);
void Texture1DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture1DMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> Texture1DMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# Texture1DMSArray<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex);
T Texture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,1>        offset);
/// See Target Availability 2
T Texture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture1DMSArray<T>.subscript

## Signature 

```
T Texture1DMSArray<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RWTexture1DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RWTexture1DMSArray<T>.GetDimensions

## Signature 

```
void RWTexture1DMSArray<T>.GetDimensions(
    out uint             width,
    out uint             elements,
    out uint             sampleCount);
void RWTexture1DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RWTexture1DMSArray<T>.GetDimensions(
    out float            width,
    out float            elements,
    out float            sampleCount);
void RWTexture1DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture1DMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> RWTexture1DMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RWTexture1DMSArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex);
T RWTexture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,1>        offset);
/// See Target Availability 2
T RWTexture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture1DMSArray<T>.subscript

## Signature 

```
T RWTexture1DMSArray<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture1DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMSArray<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture1DMSArray<T>.GetDimensions(
    out uint             width,
    out uint             elements,
    out uint             sampleCount);
void RasterizerOrderedTexture1DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTexture1DMSArray<T>.GetDimensions(
    out float            width,
    out float            elements,
    out float            sampleCount);
void RasterizerOrderedTexture1DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTexture1DMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMSArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex);
T RasterizerOrderedTexture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,1>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture1DMSArray<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,1>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture1DMSArray<T>.subscript

## Signature 

```
T RasterizerOrderedTexture1DMSArray<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct Texture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# Texture2D<T>.CalculateLevelOfDetail

## Signature 

```
float Texture2D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture2D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float Texture2D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture2D<T>.GetDimensions

## Signature 

```
void Texture2D<T>.GetDimensions(
    out uint             width,
    out uint             height);
void Texture2D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             numberOfLevels);
void Texture2D<T>.GetDimensions(
    out float            width,
    out float            height);
void Texture2D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture2D<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture2D<T>.Load(vector<int,3> location);
T Texture2D<T>.Load(
    vector<int,3>        location,
    vector<int,2>        offset);
/// See Target Availability 2
T Texture2D<T>.Load(
    vector<int,3>        location,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture2D<T>.subscript

## Signature 

```
T Texture2D<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# Texture2D<T>.Sample

## Signature 

```
/// See Target Availability 1
T Texture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
T Texture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
T Texture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    float                clamp);
T Texture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture2D<T>.SampleBias

## Signature 

```
T Texture2D<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias);
T Texture2D<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture2D<T>.SampleCmp

## Signature 

```
float Texture2D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
float Texture2D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture2D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float Texture2D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
/// See Target Availability 2
float Texture2D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture2D<T>.SampleGrad

## Signature 

```
T Texture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY);
T Texture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset);
T Texture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# Texture2D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T Texture2D<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level);
/// See Target Availability 2
T Texture2D<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level,
    vector<int,2>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension Texture2D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RWTexture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RWTexture2D<T>.CalculateLevelOfDetail

## Signature 

```
float RWTexture2D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RWTexture2D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.GetDimensions

## Signature 

```
void RWTexture2D<T>.GetDimensions(
    out uint             width,
    out uint             height);
void RWTexture2D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             numberOfLevels);
void RWTexture2D<T>.GetDimensions(
    out float            width,
    out float            height);
void RWTexture2D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture2D<T>.Load(vector<int,2> location);
/// See Target Availability 2
T RWTexture2D<T>.Load(
    vector<int,2>        location,
    vector<int,2>        offset);
/// See Target Availability 3
T RWTexture2D<T>.Load(
    vector<int,2>        location,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.subscript

## Signature 

```
T RWTexture2D<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.Sample

## Signature 

```
/// See Target Availability 1
T RWTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
T RWTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
T RWTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    float                clamp);
T RWTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.SampleBias

## Signature 

```
T RWTexture2D<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias);
T RWTexture2D<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.SampleCmp

## Signature 

```
float RWTexture2D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
float RWTexture2D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RWTexture2D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
/// See Target Availability 2
float RWTexture2D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.SampleGrad

## Signature 

```
T RWTexture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY);
T RWTexture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset);
T RWTexture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RWTexture2D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RWTexture2D<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level);
/// See Target Availability 2
T RWTexture2D<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level,
    vector<int,2>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RWTexture2D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTexture2D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTexture2D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture2D<T>.GetDimensions(
    out uint             width,
    out uint             height);
void RasterizerOrderedTexture2D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             numberOfLevels);
void RasterizerOrderedTexture2D<T>.GetDimensions(
    out float            width,
    out float            height);
void RasterizerOrderedTexture2D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2D<T>.Load(vector<int,2> location);
T RasterizerOrderedTexture2D<T>.Load(
    vector<int,2>        location,
    vector<int,2>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture2D<T>.Load(
    vector<int,2>        location,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.subscript

## Signature 

```
T RasterizerOrderedTexture2D<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
T RasterizerOrderedTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
T RasterizerOrderedTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    float                clamp);
T RasterizerOrderedTexture2D<T>.Sample(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.SampleBias

## Signature 

```
T RasterizerOrderedTexture2D<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias);
T RasterizerOrderedTexture2D<T>.SampleBias(
    SamplerState         s,
    vector<float,2>      location,
    float                bias,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.SampleCmp

## Signature 

```
float RasterizerOrderedTexture2D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
float RasterizerOrderedTexture2D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RasterizerOrderedTexture2D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue);
/// See Target Availability 2
float RasterizerOrderedTexture2D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,2>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTexture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY);
T RasterizerOrderedTexture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset);
T RasterizerOrderedTexture2D<T>.SampleGrad(
    SamplerState         s,
    vector<float,2>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2D<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level);
/// See Target Availability 2
T RasterizerOrderedTexture2D<T>.SampleLevel(
    SamplerState         s,
    vector<float,2>      location,
    float                level,
    vector<int,2>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.Gather(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherRed(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherGreen(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherBlue(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2D.GatherAlpha(
    SamplerState         s,
    vector<float,2>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct Texture2DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# Texture2DMS<T>.GetDimensions

## Signature 

```
void Texture2DMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             sampleCount);
void Texture2DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             sampleCount,
    out uint             numberOfLevels);
void Texture2DMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            sampleCount);
void Texture2DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture2DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> Texture2DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# Texture2DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex);
T Texture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,2>        offset);
/// See Target Availability 2
T Texture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture2DMS<T>.subscript

## Signature 

```
T Texture2DMS<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RWTexture2DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RWTexture2DMS<T>.GetDimensions

## Signature 

```
void RWTexture2DMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             sampleCount);
void RWTexture2DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RWTexture2DMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            sampleCount);
void RWTexture2DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture2DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RWTexture2DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RWTexture2DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex);
T RWTexture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,2>        offset);
/// See Target Availability 2
T RWTexture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture2DMS<T>.subscript

## Signature 

```
T RWTexture2DMS<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture2DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMS<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture2DMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             sampleCount);
void RasterizerOrderedTexture2DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTexture2DMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            sampleCount);
void RasterizerOrderedTexture2DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTexture2DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex);
T RasterizerOrderedTexture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,2>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture2DMS<T>.Load(
    vector<int,2>        location,
    int                  sampleIndex,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMS<T>.subscript

## Signature 

```
T RasterizerOrderedTexture2DMS<T>.subscript(vector<uint,2> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct Texture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# Texture2DArray<T>.CalculateLevelOfDetail

## Signature 

```
float Texture2DArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float Texture2DArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.GetDimensions

## Signature 

```
void Texture2DArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements);
void Texture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             numberOfLevels);
void Texture2DArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements);
void Texture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture2DArray<T>.Load(vector<int,4> location);
T Texture2DArray<T>.Load(
    vector<int,4>        location,
    vector<int,2>        offset);
/// See Target Availability 2
T Texture2DArray<T>.Load(
    vector<int,4>        location,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.subscript

## Signature 

```
T Texture2DArray<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T Texture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T Texture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
T Texture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    float                clamp);
T Texture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.SampleBias

## Signature 

```
T Texture2DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
T Texture2DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.SampleCmp

## Signature 

```
float Texture2DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
float Texture2DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float Texture2DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
/// See Target Availability 2
float Texture2DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.SampleGrad

## Signature 

```
T Texture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY);
T Texture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset);
T Texture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# Texture2DArray<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T Texture2DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
/// See Target Availability 2
T Texture2DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level,
    vector<int,2>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension Texture2DArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> Texture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RWTexture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RWTexture2DArray<T>.CalculateLevelOfDetail

## Signature 

```
float RWTexture2DArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RWTexture2DArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.GetDimensions

## Signature 

```
void RWTexture2DArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements);
void RWTexture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             numberOfLevels);
void RWTexture2DArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements);
void RWTexture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture2DArray<T>.Load(vector<int,3> location);
/// See Target Availability 2
T RWTexture2DArray<T>.Load(
    vector<int,3>        location,
    vector<int,2>        offset);
/// See Target Availability 3
T RWTexture2DArray<T>.Load(
    vector<int,3>        location,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.subscript

## Signature 

```
T RWTexture2DArray<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T RWTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T RWTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
T RWTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    float                clamp);
T RWTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.SampleBias

## Signature 

```
T RWTexture2DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
T RWTexture2DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.SampleCmp

## Signature 

```
float RWTexture2DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
float RWTexture2DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RWTexture2DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
/// See Target Availability 2
float RWTexture2DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.SampleGrad

## Signature 

```
T RWTexture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY);
T RWTexture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset);
T RWTexture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RWTexture2DArray<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RWTexture2DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
/// See Target Availability 2
T RWTexture2DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level,
    vector<int,2>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RWTexture2DArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RWTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTexture2DArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTexture2DArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,2>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture2DArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements);
void RasterizerOrderedTexture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             numberOfLevels);
void RasterizerOrderedTexture2DArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements);
void RasterizerOrderedTexture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2DArray<T>.Load(vector<int,3> location);
T RasterizerOrderedTexture2DArray<T>.Load(
    vector<int,3>        location,
    vector<int,2>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture2DArray<T>.Load(
    vector<int,3>        location,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.subscript

## Signature 

```
T RasterizerOrderedTexture2DArray<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T RasterizerOrderedTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
T RasterizerOrderedTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    float                clamp);
T RasterizerOrderedTexture2DArray<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.SampleBias

## Signature 

```
T RasterizerOrderedTexture2DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
T RasterizerOrderedTexture2DArray<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.SampleCmp

## Signature 

```
float RasterizerOrderedTexture2DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
float RasterizerOrderedTexture2DArray<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RasterizerOrderedTexture2DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
/// See Target Availability 2
float RasterizerOrderedTexture2DArray<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,2>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTexture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY);
T RasterizerOrderedTexture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset);
T RasterizerOrderedTexture2DArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,2>      gradX,
    vector<float,2>      gradY,
    vector<int,2>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
/// See Target Availability 2
T RasterizerOrderedTexture2DArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level,
    vector<int,2>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2DArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<T,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<float,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<int,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture2DArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset,
    out uint             status);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4);
/// See Target Availability 3
vector<uint,4> RasterizerOrderedTexture2DArray.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,2>        offset1,
    vector<int,2>        offset2,
    vector<int,2>        offset3,
    vector<int,2>        offset4,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct Texture2DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# Texture2DMSArray<T>.GetDimensions

## Signature 

```
void Texture2DMSArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount);
void Texture2DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void Texture2DMSArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount);
void Texture2DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture2DMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> Texture2DMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# Texture2DMSArray<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T Texture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,2>        offset);
/// See Target Availability 2
T Texture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture2DMSArray<T>.subscript

## Signature 

```
T Texture2DMSArray<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RWTexture2DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RWTexture2DMSArray<T>.GetDimensions

## Signature 

```
void RWTexture2DMSArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount);
void RWTexture2DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RWTexture2DMSArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount);
void RWTexture2DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture2DMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> RWTexture2DMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RWTexture2DMSArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T RWTexture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,2>        offset);
/// See Target Availability 2
T RWTexture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture2DMSArray<T>.subscript

## Signature 

```
T RWTexture2DMSArray<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture2DMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMSArray<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture2DMSArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount);
void RasterizerOrderedTexture2DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTexture2DMSArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount);
void RasterizerOrderedTexture2DMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTexture2DMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMSArray<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T RasterizerOrderedTexture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,2>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture2DMSArray<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,2>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture2DMSArray<T>.subscript

## Signature 

```
T RasterizerOrderedTexture2DMSArray<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct Texture3D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# Texture3D<T>.CalculateLevelOfDetail

## Signature 

```
float Texture3D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture3D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float Texture3D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# Texture3D<T>.GetDimensions

## Signature 

```
void Texture3D<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             depth);
void Texture3D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             numberOfLevels);
void Texture3D<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            depth);
void Texture3D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            depth,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _depth_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture3D<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture3D<T>.Load(vector<int,4> location);
T Texture3D<T>.Load(
    vector<int,4>        location,
    vector<int,3>        offset);
/// See Target Availability 2
T Texture3D<T>.Load(
    vector<int,4>        location,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture3D<T>.subscript

## Signature 

```
T Texture3D<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# Texture3D<T>.Sample

## Signature 

```
/// See Target Availability 1
T Texture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T Texture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 3
T Texture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    float                clamp);
T Texture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture3D<T>.SampleBias

## Signature 

```
T Texture3D<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
T Texture3D<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias,
    vector<int,3>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture3D<T>.SampleCmp

## Signature 

```
float Texture3D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
float Texture3D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,3>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture3D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float Texture3D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
/// See Target Availability 2
float Texture3D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,3>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# Texture3D<T>.SampleGrad

## Signature 

```
T Texture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
T Texture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY,
    vector<int,3>        offset);
T Texture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY,
    vector<int,3>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# Texture3D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T Texture3D<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
/// See Target Availability 2
T Texture3D<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level,
    vector<int,3>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension Texture3D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension Texture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# Texture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> Texture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# Texture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> Texture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RWTexture3D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RWTexture3D<T>.CalculateLevelOfDetail

## Signature 

```
float RWTexture3D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RWTexture3D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.GetDimensions

## Signature 

```
void RWTexture3D<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             depth);
void RWTexture3D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             numberOfLevels);
void RWTexture3D<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            depth);
void RWTexture3D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            depth,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _depth_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture3D<T>.Load(vector<int,3> location);
/// See Target Availability 2
T RWTexture3D<T>.Load(
    vector<int,3>        location,
    vector<int,3>        offset);
/// See Target Availability 3
T RWTexture3D<T>.Load(
    vector<int,3>        location,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.subscript

## Signature 

```
T RWTexture3D<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.Sample

## Signature 

```
/// See Target Availability 1
T RWTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T RWTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 3
T RWTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    float                clamp);
T RWTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.SampleBias

## Signature 

```
T RWTexture3D<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
T RWTexture3D<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias,
    vector<int,3>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.SampleCmp

## Signature 

```
float RWTexture3D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
float RWTexture3D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,3>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RWTexture3D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
/// See Target Availability 2
float RWTexture3D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,3>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.SampleGrad

## Signature 

```
T RWTexture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
T RWTexture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY,
    vector<int,3>        offset);
T RWTexture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY,
    vector<int,3>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RWTexture3D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RWTexture3D<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
/// See Target Availability 2
T RWTexture3D<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level,
    vector<int,3>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RWTexture3D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RWTexture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RWTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RWTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RWTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture3D<T>

## Generic Parameters

* _T_ 

## Methods

* _CalculateLevelOfDetail_ 
* _GetDimensions_ 
* _subscript_ 
* _SampleCmp_ 
* _Load_ 
* _SampleBias_ 
* _CalculateLevelOfDetailUnclamped_ 
* _SampleGrad_ 
* _Sample_ 
* _SampleCmpLevelZero_ 
* _SampleLevel_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTexture3D<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTexture3D<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture3D<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             depth);
void RasterizerOrderedTexture3D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             numberOfLevels);
void RasterizerOrderedTexture3D<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            depth);
void RasterizerOrderedTexture3D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            depth,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _depth_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture3D<T>.Load(vector<int,3> location);
T RasterizerOrderedTexture3D<T>.Load(
    vector<int,3>        location,
    vector<int,3>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture3D<T>.Load(
    vector<int,3>        location,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.subscript

## Signature 

```
T RasterizerOrderedTexture3D<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T RasterizerOrderedTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 3
T RasterizerOrderedTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    float                clamp);
T RasterizerOrderedTexture3D<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL
* _3_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.SampleBias

## Signature 

```
T RasterizerOrderedTexture3D<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
T RasterizerOrderedTexture3D<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias,
    vector<int,3>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.SampleCmp

## Signature 

```
float RasterizerOrderedTexture3D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
float RasterizerOrderedTexture3D<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,3>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.SampleCmpLevelZero

## Signature 

```
/// See Target Availability 1
float RasterizerOrderedTexture3D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
/// See Target Availability 2
float RasterizerOrderedTexture3D<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue,
    vector<int,3>        offset);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 
* _offset_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTexture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
T RasterizerOrderedTexture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY,
    vector<int,3>        offset);
T RasterizerOrderedTexture3D<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY,
    vector<int,3>        offset,
    float                lodClamp);
```

## Requirements

GLSL GL_ARB_sparse_texture_clamp

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 
* _offset_ 
* _lodClamp_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D<T>.SampleLevel

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture3D<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
/// See Target Availability 2
T RasterizerOrderedTexture3D<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level,
    vector<int,3>        offset);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 
* _offset_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture3D

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTexture3D

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3D.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTexture3D.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct Texture3DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# Texture3DMS<T>.GetDimensions

## Signature 

```
void Texture3DMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             sampleCount);
void Texture3DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             sampleCount,
    out uint             numberOfLevels);
void Texture3DMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            depth,
    out float            sampleCount);
void Texture3DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            depth,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _depth_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# Texture3DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> Texture3DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# Texture3DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T Texture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T Texture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset);
/// See Target Availability 2
T Texture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# Texture3DMS<T>.subscript

## Signature 

```
T Texture3DMS<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RWTexture3DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RWTexture3DMS<T>.GetDimensions

## Signature 

```
void RWTexture3DMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             sampleCount);
void RWTexture3DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RWTexture3DMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            depth,
    out float            sampleCount);
void RWTexture3DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            depth,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _depth_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RWTexture3DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RWTexture3DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RWTexture3DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RWTexture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T RWTexture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset);
/// See Target Availability 2
T RWTexture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RWTexture3DMS<T>.subscript

## Signature 

```
T RWTexture3DMS<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTexture3DMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTexture3DMS<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTexture3DMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             sampleCount);
void RasterizerOrderedTexture3DMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             depth,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTexture3DMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            depth,
    out float            sampleCount);
void RasterizerOrderedTexture3DMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            depth,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _depth_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3DMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTexture3DMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3DMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTexture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T RasterizerOrderedTexture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset);
/// See Target Availability 2
T RasterizerOrderedTexture3DMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTexture3DMS<T>.subscript

## Signature 

```
T RasterizerOrderedTexture3DMS<T>.subscript(vector<uint,3> location);
```

## Target Availability

HLSL

## Parameters

* _location_ 

--------------------------------------------------------------------------------
# struct TextureCube<T>

## Generic Parameters

* _T_ 

## Methods

* _Sample_ 
* _SampleLevel_ 
* _SampleCmp_ 
* _SampleCmpLevelZero_ 
* _CalculateLevelOfDetail_ 
* _CalculateLevelOfDetailUnclamped_ 
* _GetDimensions_ 
* _Load_ 
* _SampleGrad_ 
* _SampleBias_ 


--------------------------------------------------------------------------------
# TextureCube<T>.CalculateLevelOfDetail

## Signature 

```
float TextureCube<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# TextureCube<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float TextureCube<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# TextureCube<T>.GetDimensions

## Signature 

```
void TextureCube<T>.GetDimensions(
    out uint             width,
    out uint             height);
void TextureCube<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             numberOfLevels);
void TextureCube<T>.GetDimensions(
    out float            width,
    out float            height);
void TextureCube<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# TextureCube<T>.Load

## Signature 

```
/// See Target Availability 1
T TextureCube<T>.Load(vector<int,4> location);
T TextureCube<T>.Load(
    vector<int,4>        location,
    vector<int,3>        offset);
/// See Target Availability 2
T TextureCube<T>.Load(
    vector<int,4>        location,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# TextureCube<T>.Sample

## Signature 

```
/// See Target Availability 1
T TextureCube<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T TextureCube<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    float                clamp);
T TextureCube<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# TextureCube<T>.SampleBias

## Signature 

```
T TextureCube<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 

--------------------------------------------------------------------------------
# TextureCube<T>.SampleCmp

## Signature 

```
float TextureCube<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 

--------------------------------------------------------------------------------
# TextureCube<T>.SampleCmpLevelZero

## Signature 

```
float TextureCube<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 

--------------------------------------------------------------------------------
# TextureCube<T>.SampleGrad

## Signature 

```
T TextureCube<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 

--------------------------------------------------------------------------------
# TextureCube<T>.SampleLevel

## Signature 

```
T TextureCube<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 

--------------------------------------------------------------------------------
# extension TextureCube

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension TextureCube

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension TextureCube

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension TextureCube

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTextureCube<T>

## Generic Parameters

* _T_ 

## Methods

* _Sample_ 
* _SampleLevel_ 
* _SampleCmp_ 
* _SampleCmpLevelZero_ 
* _CalculateLevelOfDetail_ 
* _CalculateLevelOfDetailUnclamped_ 
* _GetDimensions_ 
* _Load_ 
* _SampleGrad_ 
* _SampleBias_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTextureCube<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTextureCube<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTextureCube<T>.GetDimensions(
    out uint             width,
    out uint             height);
void RasterizerOrderedTextureCube<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             numberOfLevels);
void RasterizerOrderedTextureCube<T>.GetDimensions(
    out float            width,
    out float            height);
void RasterizerOrderedTextureCube<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTextureCube<T>.Load(vector<int,3> location);
T RasterizerOrderedTextureCube<T>.Load(
    vector<int,3>        location,
    vector<int,3>        offset);
/// See Target Availability 2
T RasterizerOrderedTextureCube<T>.Load(
    vector<int,3>        location,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTextureCube<T>.Sample(
    SamplerState         s,
    vector<float,3>      location);
/// See Target Availability 2
T RasterizerOrderedTextureCube<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    float                clamp);
T RasterizerOrderedTextureCube<T>.Sample(
    SamplerState         s,
    vector<float,3>      location,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.SampleBias

## Signature 

```
T RasterizerOrderedTextureCube<T>.SampleBias(
    SamplerState         s,
    vector<float,3>      location,
    float                bias);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.SampleCmp

## Signature 

```
float RasterizerOrderedTextureCube<T>.SampleCmp(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.SampleCmpLevelZero

## Signature 

```
float RasterizerOrderedTextureCube<T>.SampleCmpLevelZero(
    SamplerComparisonState s,
    vector<float,3>      location,
    float                compareValue);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _compareValue_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTextureCube<T>.SampleGrad(
    SamplerState         s,
    vector<float,3>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube<T>.SampleLevel

## Signature 

```
T RasterizerOrderedTextureCube<T>.SampleLevel(
    SamplerState         s,
    vector<float,3>      location,
    float                level);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCube

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<T,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCube

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<float,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCube

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<int,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCube

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.Gather(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherRed(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherGreen(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherBlue(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCube.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location);
vector<uint,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCube.GatherAlpha(
    SamplerState         s,
    vector<float,3>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct TextureCubeMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# TextureCubeMS<T>.GetDimensions

## Signature 

```
void TextureCubeMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             sampleCount);
void TextureCubeMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             sampleCount,
    out uint             numberOfLevels);
void TextureCubeMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            sampleCount);
void TextureCubeMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# TextureCubeMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> TextureCubeMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# TextureCubeMS<T>.Load

## Signature 

```
/// See Target Availability 1
T TextureCubeMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T TextureCubeMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset);
/// See Target Availability 2
T TextureCubeMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTextureCubeMS<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeMS<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTextureCubeMS<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             sampleCount);
void RasterizerOrderedTextureCubeMS<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTextureCubeMS<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            sampleCount);
void RasterizerOrderedTextureCubeMS<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeMS<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTextureCubeMS<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeMS<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTextureCubeMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex);
T RasterizerOrderedTextureCubeMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset);
/// See Target Availability 2
T RasterizerOrderedTextureCubeMS<T>.Load(
    vector<int,3>        location,
    int                  sampleIndex,
    vector<int,3>        offset,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _sampleIndex_ 
* _offset_ 
* _status_ 

--------------------------------------------------------------------------------
# struct TextureCubeArray<T>

## Generic Parameters

* _T_ 

## Methods

* _Sample_ 
* _SampleLevel_ 
* _CalculateLevelOfDetail_ 
* _CalculateLevelOfDetailUnclamped_ 
* _GetDimensions_ 
* _SampleGrad_ 
* _SampleBias_ 


--------------------------------------------------------------------------------
# TextureCubeArray<T>.CalculateLevelOfDetail

## Signature 

```
float TextureCubeArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# TextureCubeArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float TextureCubeArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# TextureCubeArray<T>.GetDimensions

## Signature 

```
void TextureCubeArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements);
void TextureCubeArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             numberOfLevels);
void TextureCubeArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements);
void TextureCubeArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# TextureCubeArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T TextureCubeArray<T>.Sample(
    SamplerState         s,
    vector<float,4>      location);
/// See Target Availability 2
T TextureCubeArray<T>.Sample(
    SamplerState         s,
    vector<float,4>      location,
    float                clamp);
T TextureCubeArray<T>.Sample(
    SamplerState         s,
    vector<float,4>      location,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# TextureCubeArray<T>.SampleBias

## Signature 

```
T TextureCubeArray<T>.SampleBias(
    SamplerState         s,
    vector<float,4>      location,
    float                bias);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 

--------------------------------------------------------------------------------
# TextureCubeArray<T>.SampleGrad

## Signature 

```
T TextureCubeArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,4>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 

--------------------------------------------------------------------------------
# TextureCubeArray<T>.SampleLevel

## Signature 

```
T TextureCubeArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,4>      location,
    float                level);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 

--------------------------------------------------------------------------------
# extension TextureCubeArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension TextureCubeArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension TextureCubeArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension TextureCubeArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# TextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# TextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> TextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTextureCubeArray<T>

## Generic Parameters

* _T_ 

## Methods

* _Sample_ 
* _SampleLevel_ 
* _CalculateLevelOfDetail_ 
* _CalculateLevelOfDetailUnclamped_ 
* _GetDimensions_ 
* _SampleGrad_ 
* _SampleBias_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.CalculateLevelOfDetail

## Signature 

```
float RasterizerOrderedTextureCubeArray<T>.CalculateLevelOfDetail(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.CalculateLevelOfDetailUnclamped

## Signature 

```
float RasterizerOrderedTextureCubeArray<T>.CalculateLevelOfDetailUnclamped(
    SamplerState         s,
    vector<float,3>      location);
```

## Target Availability

HLSL

## Parameters

* _s_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTextureCubeArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements);
void RasterizerOrderedTextureCubeArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             numberOfLevels);
void RasterizerOrderedTextureCubeArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements);
void RasterizerOrderedTextureCubeArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.Sample

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedTextureCubeArray<T>.Sample(
    SamplerState         s,
    vector<float,4>      location);
/// See Target Availability 2
T RasterizerOrderedTextureCubeArray<T>.Sample(
    SamplerState         s,
    vector<float,4>      location,
    float                clamp);
T RasterizerOrderedTextureCubeArray<T>.Sample(
    SamplerState         s,
    vector<float,4>      location,
    float                clamp,
    out uint             status);
```

## Target Availability

* _1_ CUDA, GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _clamp_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.SampleBias

## Signature 

```
T RasterizerOrderedTextureCubeArray<T>.SampleBias(
    SamplerState         s,
    vector<float,4>      location,
    float                bias);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _bias_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.SampleGrad

## Signature 

```
T RasterizerOrderedTextureCubeArray<T>.SampleGrad(
    SamplerState         s,
    vector<float,4>      location,
    vector<float,3>      gradX,
    vector<float,3>      gradY);
```

## Target Availability

GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _gradX_ 
* _gradY_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray<T>.SampleLevel

## Signature 

```
T RasterizerOrderedTextureCubeArray<T>.SampleLevel(
    SamplerState         s,
    vector<float,4>      location,
    float                level);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _s_ 
* _location_ 
* _level_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCubeArray

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<T,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<T,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<T,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCubeArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<float,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<float,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<float,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCubeArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<int,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<int,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<int,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# extension RasterizerOrderedTextureCubeArray

## Methods

* _Gather_ 
* _GatherAlpha_ 
* _GatherRed_ 
* _GatherGreen_ 
* _GatherBlue_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.Gather

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.Gather(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherRed

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherRed(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherGreen

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherGreen(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherBlue

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherBlue(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeArray.GatherAlpha

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location);
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset,
    out uint             status);
/// See Target Availability 1
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedTextureCubeArray.GatherAlpha(
    SamplerState         s,
    vector<float,4>      location,
    vector<int,3>        offset1,
    vector<int,3>        offset2,
    vector<int,3>        offset3,
    vector<int,3>        offset4,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _s_ 
* _location_ 
* _offset_ 
* _status_ 
* _offset1_ 
* _offset2_ 
* _offset3_ 
* _offset4_ 

--------------------------------------------------------------------------------
# struct TextureCubeMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# TextureCubeMSArray<T>.GetDimensions

## Signature 

```
void TextureCubeMSArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount);
void TextureCubeMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void TextureCubeMSArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount);
void TextureCubeMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# TextureCubeMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> TextureCubeMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedTextureCubeMSArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetSamplePosition_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeMSArray<T>.GetDimensions

## Signature 

```
void RasterizerOrderedTextureCubeMSArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount);
void RasterizerOrderedTextureCubeMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             sampleCount,
    out uint             numberOfLevels);
void RasterizerOrderedTextureCubeMSArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount);
void RasterizerOrderedTextureCubeMSArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            sampleCount,
    out float            numberOfLevels);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _sampleCount_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# RasterizerOrderedTextureCubeMSArray<T>.GetSamplePosition

## Signature 

```
vector<float,2> RasterizerOrderedTextureCubeMSArray<T>.GetSamplePosition(int s);
```

## Target Availability

HLSL

## Parameters

* _s_ 

--------------------------------------------------------------------------------
# bit_cast<T, U>

## Signature 

```
T bit_cast<T, U>(U value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _U_ 
* _value_ 

--------------------------------------------------------------------------------
# getStringHash

## Description

Given a string returns an integer hash of that string.

## Signature 

```
int getStringHash(String string);
```

## Target Availability

HLSL

## Parameters

* _string_ 

--------------------------------------------------------------------------------
# beginInvocationInterlock

## Signature 

```
void beginInvocationInterlock();
```

## Requirements

GLSL GL_ARB_fragment_shader_interlock, GLSL420

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# endInvocationInterlock

## Signature 

```
void endInvocationInterlock();
```

## Requirements

GLSL GL_ARB_fragment_shader_interlock, GLSL420

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# enum _AttributeTargets

## Values 

* _Struct_ 
* _Var_ 
* _Function_ 

--------------------------------------------------------------------------------
# struct AppendStructuredBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _Append_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# AppendStructuredBuffer<T>.Append

## Signature 

```
void AppendStructuredBuffer<T>.Append(T value);
```

## Target Availability

HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# AppendStructuredBuffer<T>.GetDimensions

## Signature 

```
void AppendStructuredBuffer<T>.GetDimensions(
    out uint             numStructs,
    out uint             stride);
```

## Target Availability

HLSL

## Parameters

* _numStructs_ 
* _stride_ 

--------------------------------------------------------------------------------
# struct ByteAddressBuffer

## Methods

* _Load3_ 
* _Load4_ 
* _GetDimensions_ 
* _Load2_ 


--------------------------------------------------------------------------------
# ByteAddressBuffer.GetDimensions

## Signature 

```
void ByteAddressBuffer.GetDimensions(out uint dim);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dim_ 

--------------------------------------------------------------------------------
# ByteAddressBuffer.Load2

## Signature 

```
/// See Target Availability 1
vector<uint,2> ByteAddressBuffer.Load2(int location);
/// See Target Availability 2
vector<uint,2> ByteAddressBuffer.Load2(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# ByteAddressBuffer.Load3

## Signature 

```
/// See Target Availability 1
vector<uint,3> ByteAddressBuffer.Load3(int location);
/// See Target Availability 2
vector<uint,3> ByteAddressBuffer.Load3(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# ByteAddressBuffer.Load4

## Signature 

```
/// See Target Availability 1
vector<uint,4> ByteAddressBuffer.Load4(int location);
/// See Target Availability 2
vector<uint,4> ByteAddressBuffer.Load4(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# ByteAddressBuffer.Load<T>

## Signature 

```
T ByteAddressBuffer.Load<T>(int location);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _location_ 

--------------------------------------------------------------------------------
# struct StructuredBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# StructuredBuffer<T>.GetDimensions

## Signature 

```
void StructuredBuffer<T>.GetDimensions(
    out uint             numStructs,
    out uint             stride);
```

## Target Availability

GLSL, HLSL

## Parameters

* _numStructs_ 
* _stride_ 

--------------------------------------------------------------------------------
# StructuredBuffer<T>.Load

## Signature 

```
/// See Target Availability 1
T StructuredBuffer<T>.Load(int location);
/// See Target Availability 2
T StructuredBuffer<T>.Load(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# StructuredBuffer<T>.subscript

## Signature 

```
T StructuredBuffer<T>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct ConsumeStructuredBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _Consume_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# ConsumeStructuredBuffer<T>.Consume

## Signature 

```
T ConsumeStructuredBuffer<T>.Consume();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# ConsumeStructuredBuffer<T>.GetDimensions

## Signature 

```
void ConsumeStructuredBuffer<T>.GetDimensions(
    out uint             numStructs,
    out uint             stride);
```

## Target Availability

HLSL

## Parameters

* _numStructs_ 
* _stride_ 

--------------------------------------------------------------------------------
# struct InputPatch<T, N:int>

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _subscript_ 


--------------------------------------------------------------------------------
# InputPatch<T, N:int>.subscript

## Signature 

```
T InputPatch<T, N:int>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct OutputPatch<T, N:int>

## Generic Parameters

* _T_ 
* _N_ 

## Methods

* _subscript_ 


--------------------------------------------------------------------------------
# OutputPatch<T, N:int>.subscript

## Signature 

```
T OutputPatch<T, N:int>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct RWByteAddressBuffer

## Methods

* _InterlockedXor_ 
* _Load4_ 
* _InterlockedAdd_ 
* _GetDimensions_ 
* _InterlockedOr_ 
* _InterlockedAndU64_ 
* _InterlockedAddF32_ 
* _InterlockedExchange_ 
* _InterlockedCompareExchangeU64_ 
* _InterlockedOrU64_ 
* _InterlockedMinU64_ 
* _InterlockedAnd_ 
* _Store3_ 
* _InterlockedMin_ 
* _Store4_ 
* _Store2_ 
* _InterlockedMaxU64_ 
* _InterlockedCompareExchange_ 
* _InterlockedExchangeU64_ 
* _Load2_ 
* _InterlockedMax_ 
* _InterlockedXorU64_ 
* _InterlockedAddI64_ 
* _Load3_ 
* _InterlockedCompareStore_ 


--------------------------------------------------------------------------------
# RWByteAddressBuffer.GetDimensions

## Signature 

```
void RWByteAddressBuffer.GetDimensions(out uint dim);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dim_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Load2

## Signature 

```
/// See Target Availability 1
vector<uint,2> RWByteAddressBuffer.Load2(int location);
/// See Target Availability 2
vector<uint,2> RWByteAddressBuffer.Load2(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Load3

## Signature 

```
/// See Target Availability 1
vector<uint,3> RWByteAddressBuffer.Load3(int location);
/// See Target Availability 2
vector<uint,3> RWByteAddressBuffer.Load3(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Load4

## Signature 

```
/// See Target Availability 1
vector<uint,4> RWByteAddressBuffer.Load4(int location);
/// See Target Availability 2
vector<uint,4> RWByteAddressBuffer.Load4(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Load<T>

## Signature 

```
T RWByteAddressBuffer.Load<T>(int location);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _location_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedAddF32

## Signature 

```
void RWByteAddressBuffer.InterlockedAddF32(
    uint                 byteAddress,
    float                valueToAdd,
    out float            originalValue);
void RWByteAddressBuffer.InterlockedAddF32(
    uint                 byteAddress,
    float                valueToAdd);
```

## Requirements

CUDA SM 2.0, NVAPI

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _valueToAdd_ 
* _originalValue_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedAddI64

## Signature 

```
void RWByteAddressBuffer.InterlockedAddI64(
    uint                 byteAddress,
    int64_t              valueToAdd,
    out int64_t          originalValue);
void RWByteAddressBuffer.InterlockedAddI64(
    uint                 byteAddress,
    int64_t              valueToAdd);
```

## Requirements

CUDA SM 6.0

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _valueToAdd_ 
* _originalValue_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedCompareExchangeU64

## Signature 

```
void RWByteAddressBuffer.InterlockedCompareExchangeU64(
    uint                 byteAddress,
    uint64_t             compareValue,
    uint64_t             value,
    out uint64_t         outOriginalValue);
```

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _compareValue_ 
* _value_ 
* _outOriginalValue_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedMaxU64

## Signature 

```
uint64_t RWByteAddressBuffer.InterlockedMaxU64(
    uint                 byteAddress,
    uint64_t             value);
```

## Requirements

CUDA SM 3.5

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedMinU64

## Signature 

```
uint64_t RWByteAddressBuffer.InterlockedMinU64(
    uint                 byteAddress,
    uint64_t             value);
```

## Requirements

CUDA SM 3.5

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedAndU64

## Signature 

```
uint64_t RWByteAddressBuffer.InterlockedAndU64(
    uint                 byteAddress,
    uint64_t             value);
```

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedOrU64

## Signature 

```
uint64_t RWByteAddressBuffer.InterlockedOrU64(
    uint                 byteAddress,
    uint64_t             value);
```

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedXorU64

## Signature 

```
uint64_t RWByteAddressBuffer.InterlockedXorU64(
    uint                 byteAddress,
    uint64_t             value);
```

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedExchangeU64

## Signature 

```
uint64_t RWByteAddressBuffer.InterlockedExchangeU64(
    uint                 byteAddress,
    uint64_t             value);
```

## Target Availability

CUDA, HLSL

## Parameters

* _byteAddress_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedAdd

## Signature 

```
void RWByteAddressBuffer.InterlockedAdd(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RWByteAddressBuffer.InterlockedAdd(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedAnd

## Signature 

```
void RWByteAddressBuffer.InterlockedAnd(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RWByteAddressBuffer.InterlockedAnd(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedCompareExchange

## Signature 

```
void RWByteAddressBuffer.InterlockedCompareExchange(
    uint                 dest,
    uint                 compare_value,
    uint                 value,
    out uint             original_value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _compare_value_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedCompareStore

## Signature 

```
void RWByteAddressBuffer.InterlockedCompareStore(
    uint                 dest,
    uint                 compare_value,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _compare_value_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedExchange

## Signature 

```
void RWByteAddressBuffer.InterlockedExchange(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedMax

## Signature 

```
void RWByteAddressBuffer.InterlockedMax(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RWByteAddressBuffer.InterlockedMax(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedMin

## Signature 

```
void RWByteAddressBuffer.InterlockedMin(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RWByteAddressBuffer.InterlockedMin(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedOr

## Signature 

```
void RWByteAddressBuffer.InterlockedOr(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RWByteAddressBuffer.InterlockedOr(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.InterlockedXor

## Signature 

```
void RWByteAddressBuffer.InterlockedXor(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RWByteAddressBuffer.InterlockedXor(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Store2

## Signature 

```
void RWByteAddressBuffer.Store2(
    uint                 address,
    vector<uint,2>       value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _address_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Store3

## Signature 

```
void RWByteAddressBuffer.Store3(
    uint                 address,
    vector<uint,3>       value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _address_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Store4

## Signature 

```
void RWByteAddressBuffer.Store4(
    uint                 address,
    vector<uint,4>       value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _address_ 
* _value_ 

--------------------------------------------------------------------------------
# RWByteAddressBuffer.Store<T>

## Signature 

```
void RWByteAddressBuffer.Store<T>(
    int                  offset,
    T                    value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _offset_ 
* _value_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedByteAddressBuffer

## Methods

* _Load4_ 
* _InterlockedAdd_ 
* _InterlockedOr_ 
* _GetDimensions_ 
* _InterlockedMax_ 
* _InterlockedAnd_ 
* _InterlockedMin_ 
* _Store2_ 
* _InterlockedExchange_ 
* _Load3_ 
* _InterlockedCompareStore_ 
* _InterlockedXor_ 
* _InterlockedCompareExchange_ 
* _Load2_ 
* _Store3_ 
* _Store4_ 


--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.GetDimensions

## Signature 

```
void RasterizerOrderedByteAddressBuffer.GetDimensions(out uint dim);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dim_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Load2

## Signature 

```
/// See Target Availability 1
vector<uint,2> RasterizerOrderedByteAddressBuffer.Load2(int location);
/// See Target Availability 2
vector<uint,2> RasterizerOrderedByteAddressBuffer.Load2(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Load3

## Signature 

```
/// See Target Availability 1
vector<uint,3> RasterizerOrderedByteAddressBuffer.Load3(int location);
/// See Target Availability 2
vector<uint,3> RasterizerOrderedByteAddressBuffer.Load3(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Load4

## Signature 

```
/// See Target Availability 1
vector<uint,4> RasterizerOrderedByteAddressBuffer.Load4(int location);
/// See Target Availability 2
vector<uint,4> RasterizerOrderedByteAddressBuffer.Load4(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Load<T>

## Signature 

```
T RasterizerOrderedByteAddressBuffer.Load<T>(int location);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _location_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedAdd

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedAdd(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RasterizerOrderedByteAddressBuffer.InterlockedAdd(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedAnd

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedAnd(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RasterizerOrderedByteAddressBuffer.InterlockedAnd(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedCompareExchange

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedCompareExchange(
    uint                 dest,
    uint                 compare_value,
    uint                 value,
    out uint             original_value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _compare_value_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedCompareStore

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedCompareStore(
    uint                 dest,
    uint                 compare_value,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _compare_value_ 
* _value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedExchange

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedExchange(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedMax

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedMax(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RasterizerOrderedByteAddressBuffer.InterlockedMax(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedMin

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedMin(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RasterizerOrderedByteAddressBuffer.InterlockedMin(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedOr

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedOr(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RasterizerOrderedByteAddressBuffer.InterlockedOr(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.InterlockedXor

## Signature 

```
void RasterizerOrderedByteAddressBuffer.InterlockedXor(
    uint                 dest,
    uint                 value,
    out uint             original_value);
void RasterizerOrderedByteAddressBuffer.InterlockedXor(
    uint                 dest,
    uint                 value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Store2

## Signature 

```
void RasterizerOrderedByteAddressBuffer.Store2(
    uint                 address,
    vector<uint,2>       value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _address_ 
* _value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Store3

## Signature 

```
void RasterizerOrderedByteAddressBuffer.Store3(
    uint                 address,
    vector<uint,3>       value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _address_ 
* _value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Store4

## Signature 

```
void RasterizerOrderedByteAddressBuffer.Store4(
    uint                 address,
    vector<uint,4>       value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _address_ 
* _value_ 

--------------------------------------------------------------------------------
# RasterizerOrderedByteAddressBuffer.Store<T>

## Signature 

```
void RasterizerOrderedByteAddressBuffer.Store<T>(
    int                  offset,
    T                    value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _offset_ 
* _value_ 

--------------------------------------------------------------------------------
# struct RWStructuredBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _subscript_ 
* _DecrementCounter_ 
* _Load_ 
* _GetDimensions_ 
* _IncrementCounter_ 


--------------------------------------------------------------------------------
# RWStructuredBuffer<T>.DecrementCounter

## Signature 

```
uint RWStructuredBuffer<T>.DecrementCounter();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# RWStructuredBuffer<T>.GetDimensions

## Signature 

```
void RWStructuredBuffer<T>.GetDimensions(
    out uint             numStructs,
    out uint             stride);
```

## Target Availability

GLSL, HLSL

## Parameters

* _numStructs_ 
* _stride_ 

--------------------------------------------------------------------------------
# RWStructuredBuffer<T>.IncrementCounter

## Signature 

```
uint RWStructuredBuffer<T>.IncrementCounter();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# RWStructuredBuffer<T>.Load

## Signature 

```
/// See Target Availability 1
T RWStructuredBuffer<T>.Load(int location);
/// See Target Availability 2
T RWStructuredBuffer<T>.Load(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RWStructuredBuffer<T>.subscript

## Signature 

```
T RWStructuredBuffer<T>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedStructuredBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _subscript_ 
* _DecrementCounter_ 
* _Load_ 
* _GetDimensions_ 
* _IncrementCounter_ 


--------------------------------------------------------------------------------
# RasterizerOrderedStructuredBuffer<T>.DecrementCounter

## Signature 

```
uint RasterizerOrderedStructuredBuffer<T>.DecrementCounter();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# RasterizerOrderedStructuredBuffer<T>.GetDimensions

## Signature 

```
void RasterizerOrderedStructuredBuffer<T>.GetDimensions(
    out uint             numStructs,
    out uint             stride);
```

## Target Availability

GLSL, HLSL

## Parameters

* _numStructs_ 
* _stride_ 

--------------------------------------------------------------------------------
# RasterizerOrderedStructuredBuffer<T>.IncrementCounter

## Signature 

```
uint RasterizerOrderedStructuredBuffer<T>.IncrementCounter();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# RasterizerOrderedStructuredBuffer<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedStructuredBuffer<T>.Load(int location);
/// See Target Availability 2
T RasterizerOrderedStructuredBuffer<T>.Load(
    int                  location,
    out uint             status);
```

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedStructuredBuffer<T>.subscript

## Signature 

```
T RasterizerOrderedStructuredBuffer<T>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct PointStream<T>

## Generic Parameters

* _T_ 

## Methods

* _Append_ 
* _RestartStrip_ 


--------------------------------------------------------------------------------
# PointStream<T>.Append

## Signature 

```
void PointStream<T>.Append(T value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# PointStream<T>.RestartStrip

## Signature 

```
void PointStream<T>.RestartStrip();
```

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# struct LineStream<T>

## Generic Parameters

* _T_ 

## Methods

* _Append_ 
* _RestartStrip_ 


--------------------------------------------------------------------------------
# LineStream<T>.Append

## Signature 

```
void LineStream<T>.Append(T value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# LineStream<T>.RestartStrip

## Signature 

```
void LineStream<T>.RestartStrip();
```

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# struct TriangleStream<T>

## Generic Parameters

* _T_ 

## Methods

* _Append_ 
* _RestartStrip_ 


--------------------------------------------------------------------------------
# TriangleStream<T>.Append

## Signature 

```
void TriangleStream<T>.Append(T value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# TriangleStream<T>.RestartStrip

## Signature 

```
void TriangleStream<T>.RestartStrip();
```

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# abort

## Signature 

```
void abort();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# abs<T>

## Signature 

```
T abs<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# abs<T, N:int>

## Signature 

```
vector<T,N> abs<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# abs<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> abs<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# acos<T>

## Signature 

```
T acos<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# acos<T, N:int>

## Signature 

```
vector<T,N> acos<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# acos<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> acos<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# all<T>

## Signature 

```
bool all<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# all<T, N:int>

## Signature 

```
bool all<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# all<T, N:int, M:int>

## Signature 

```
bool all<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# AllMemoryBarrier

## Signature 

```
void AllMemoryBarrier();
```

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# AllMemoryBarrierWithGroupSync

## Signature 

```
void AllMemoryBarrierWithGroupSync();
```

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# any<T>

## Signature 

```
bool any<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# any<T, N:int>

## Signature 

```
bool any<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# any<T, N:int, M:int>

## Signature 

```
bool any<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asdouble

## Signature 

```
double asdouble(
    uint                 lowbits,
    uint                 highbits);
```

## Requirements

GLSL GL_ARB_gpu_shader5

## Target Availability

GLSL, HLSL

## Parameters

* _lowbits_ 
* _highbits_ 

--------------------------------------------------------------------------------
# asfloat<N:int>

## Signature 

```
vector<float,N> asfloat<N:int>(vector<int,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asfloat<N:int>

## Signature 

```
vector<float,N> asfloat<N:int>(vector<uint,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asfloat<N:int, M:int>

## Signature 

```
matrix<float,N,M> asfloat<N:int, M:int>(matrix<int,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asfloat<N:int, M:int>

## Signature 

```
matrix<float,N,M> asfloat<N:int, M:int>(matrix<uint,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asfloat<N:int>

## Signature 

```
vector<float,N> asfloat<N:int>(vector<float,N> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asfloat<N:int, M:int>

## Signature 

```
matrix<float,N,M> asfloat<N:int, M:int>(matrix<float,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asin<T>

## Signature 

```
T asin<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# asin<T, N:int>

## Signature 

```
vector<T,N> asin<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asin<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> asin<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asint<N:int>

## Signature 

```
vector<int,N> asint<N:int>(vector<float,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asint<N:int>

## Signature 

```
vector<int,N> asint<N:int>(vector<uint,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asint<N:int, M:int>

## Signature 

```
matrix<int,N,M> asint<N:int, M:int>(matrix<float,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asint<N:int, M:int>

## Signature 

```
matrix<int,N,M> asint<N:int, M:int>(matrix<uint,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asint<N:int>

## Signature 

```
vector<int,N> asint<N:int>(vector<int,N> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asint<N:int, M:int>

## Signature 

```
matrix<int,N,M> asint<N:int, M:int>(matrix<int,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asuint<N:int>

## Signature 

```
vector<uint,N> asuint<N:int>(vector<float,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asuint<N:int>

## Signature 

```
vector<uint,N> asuint<N:int>(vector<int,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asuint<N:int, M:int>

## Signature 

```
matrix<uint,N,M> asuint<N:int, M:int>(matrix<float,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asuint<N:int, M:int>

## Signature 

```
matrix<uint,N,M> asuint<N:int, M:int>(matrix<int,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asuint<N:int>

## Signature 

```
vector<uint,N> asuint<N:int>(vector<uint,N> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# asuint<N:int, M:int>

## Signature 

```
matrix<uint,N,M> asuint<N:int, M:int>(matrix<uint,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# asfloat16<N:int>

## Signature 

```
vector<half,N> asfloat16<N:int>(vector<half,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asfloat16<R:int, C:int>

## Signature 

```
matrix<half,R,C> asfloat16<R:int, C:int>(matrix<half,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asint16<N:int>

## Signature 

```
vector<int16_t,N> asint16<N:int>(vector<int16_t,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asint16<R:int, C:int>

## Signature 

```
matrix<int16_t,R,C> asint16<R:int, C:int>(matrix<int16_t,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asuint16<N:int>

## Signature 

```
vector<uint16_t,N> asuint16<N:int>(vector<uint16_t,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asuint16<R:int, C:int>

## Signature 

```
matrix<uint16_t,R,C> asuint16<R:int, C:int>(matrix<uint16_t,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asint16<N:int>

## Signature 

```
vector<int16_t,N> asint16<N:int>(vector<uint16_t,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asint16<R:int, C:int>

## Signature 

```
matrix<int16_t,R,C> asint16<R:int, C:int>(matrix<uint16_t,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asuint16<N:int>

## Signature 

```
vector<uint16_t,N> asuint16<N:int>(vector<int16_t,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asuint16<R:int, C:int>

## Signature 

```
matrix<uint16_t,R,C> asuint16<R:int, C:int>(matrix<int16_t,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asuint16<N:int>

## Signature 

```
vector<uint16_t,N> asuint16<N:int>(vector<half,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asuint16<R:int, C:int>

## Signature 

```
matrix<uint16_t,R,C> asuint16<R:int, C:int>(matrix<half,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asfloat16<N:int>

## Signature 

```
vector<half,N> asfloat16<N:int>(vector<uint16_t,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asfloat16<R:int, C:int>

## Signature 

```
matrix<half,R,C> asfloat16<R:int, C:int>(matrix<uint16_t,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asint16<N:int>

## Signature 

```
vector<int16_t,N> asint16<N:int>(vector<half,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asint16<R:int, C:int>

## Signature 

```
matrix<int16_t,R,C> asint16<R:int, C:int>(matrix<half,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# asfloat16<N:int>

## Signature 

```
vector<half,N> asfloat16<N:int>(vector<int16_t,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# asfloat16<R:int, C:int>

## Signature 

```
matrix<half,R,C> asfloat16<R:int, C:int>(matrix<int16_t,R,C> value);
```

## Target Availability

HLSL

## Parameters

* _R_ 
* _C_ 
* _value_ 

--------------------------------------------------------------------------------
# atan<T>

## Signature 

```
T atan<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# atan<T, N:int>

## Signature 

```
vector<T,N> atan<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# atan<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> atan<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# atan2<T>

## Signature 

```
T atan2<T>(
    T                    y,
    T                    x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _y_ 
* _x_ 

--------------------------------------------------------------------------------
# atan2<T, N:int>

## Signature 

```
vector<T,N> atan2<T, N:int>(
    vector<T,N>          y,
    vector<T,N>          x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _y_ 
* _x_ 

--------------------------------------------------------------------------------
# atan2<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> atan2<T, N:int, M:int>(
    matrix<T,N,M>        y,
    matrix<T,N,M>        x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _y_ 
* _x_ 

--------------------------------------------------------------------------------
# ceil<T>

## Signature 

```
T ceil<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ceil<T, N:int>

## Signature 

```
vector<T,N> ceil<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ceil<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ceil<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# CheckAccessFullyMapped

## Signature 

```
bool CheckAccessFullyMapped(uint status);
```

## Target Availability

HLSL

## Parameters

* _status_ 

--------------------------------------------------------------------------------
# clamp<T>

## Signature 

```
T clamp<T>(
    T                    x,
    T                    minBound,
    T                    maxBound);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _minBound_ 
* _maxBound_ 

--------------------------------------------------------------------------------
# clamp<T, N:int>

## Signature 

```
vector<T,N> clamp<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          minBound,
    vector<T,N>          maxBound);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _minBound_ 
* _maxBound_ 

--------------------------------------------------------------------------------
# clamp<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> clamp<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        minBound,
    matrix<T,N,M>        maxBound);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _minBound_ 
* _maxBound_ 

--------------------------------------------------------------------------------
# clip<T>

## Signature 

```
void clip<T>(T x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# clip<T, N:int>

## Signature 

```
void clip<T, N:int>(vector<T,N> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# clip<T, N:int, M:int>

## Signature 

```
void clip<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# cos<T>

## Signature 

```
T cos<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# cos<T, N:int>

## Signature 

```
vector<T,N> cos<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# cos<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> cos<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# cosh<T>

## Signature 

```
T cosh<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# cosh<T, N:int>

## Signature 

```
vector<T,N> cosh<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# cosh<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> cosh<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# countbits

## Signature 

```
uint countbits(uint value);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# cross<T>

## Signature 

```
vector<T,3> cross<T>(
    vector<T,3>          left,
    vector<T,3>          right);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _left_ 
* _right_ 

--------------------------------------------------------------------------------
# D3DCOLORtoUBYTE4

## Signature 

```
vector<int,4> D3DCOLORtoUBYTE4(vector<float,4> color);
```

## Target Availability

HLSL

## Parameters

* _color_ 

--------------------------------------------------------------------------------
# ddx<T>

## Signature 

```
T ddx<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx<T, N:int>

## Signature 

```
vector<T,N> ddx<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ddx<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx_coarse<T>

## Signature 

```
T ddx_coarse<T>(T x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx_coarse<T, N:int>

## Signature 

```
vector<T,N> ddx_coarse<T, N:int>(vector<T,N> x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx_coarse<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ddx_coarse<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx_fine<T>

## Signature 

```
T ddx_fine<T>(T x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx_fine<T, N:int>

## Signature 

```
vector<T,N> ddx_fine<T, N:int>(vector<T,N> x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ddx_fine<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ddx_fine<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy<T>

## Signature 

```
T ddy<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy<T, N:int>

## Signature 

```
vector<T,N> ddy<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ddy<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy_coarse<T>

## Signature 

```
T ddy_coarse<T>(T x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy_coarse<T, N:int>

## Signature 

```
vector<T,N> ddy_coarse<T, N:int>(vector<T,N> x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy_coarse<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ddy_coarse<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy_fine<T>

## Signature 

```
T ddy_fine<T>(T x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy_fine<T, N:int>

## Signature 

```
vector<T,N> ddy_fine<T, N:int>(vector<T,N> x);
```

## Requirements

GLSL GL_ARB_derivative_control

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# ddy_fine<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ddy_fine<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# degrees<T>

## Signature 

```
T degrees<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# degrees<T, N:int>

## Signature 

```
vector<T,N> degrees<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# degrees<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> degrees<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# determinant<T, N:int>

## Signature 

```
T determinant<T, N:int>(matrix<T,N,N> m);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _m_ 

--------------------------------------------------------------------------------
# DeviceMemoryBarrier

## Signature 

```
void DeviceMemoryBarrier();
```

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# DeviceMemoryBarrierWithGroupSync

## Signature 

```
void DeviceMemoryBarrierWithGroupSync();
```

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# distance<T, N:int>

## Signature 

```
T distance<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# dot<T, N:int>

## Signature 

```
T dot<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# dst<T>

## Signature 

```
vector<T,4> dst<T>(
    vector<T,4>          x,
    vector<T,4>          y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# EvaluateAttributeAtCentroid<T>

## Signature 

```
T EvaluateAttributeAtCentroid<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# EvaluateAttributeAtCentroid<T, N:int>

## Signature 

```
vector<T,N> EvaluateAttributeAtCentroid<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# EvaluateAttributeAtCentroid<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> EvaluateAttributeAtCentroid<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# EvaluateAttributeAtSample<T>

## Signature 

```
T EvaluateAttributeAtSample<T>(
    T                    x,
    uint                 sampleindex);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _sampleindex_ 

--------------------------------------------------------------------------------
# EvaluateAttributeAtSample<T, N:int>

## Signature 

```
vector<T,N> EvaluateAttributeAtSample<T, N:int>(
    vector<T,N>          x,
    uint                 sampleindex);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _sampleindex_ 

--------------------------------------------------------------------------------
# EvaluateAttributeAtSample<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> EvaluateAttributeAtSample<T, N:int, M:int>(
    matrix<T,N,M>        x,
    uint                 sampleindex);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _sampleindex_ 

--------------------------------------------------------------------------------
# EvaluateAttributeSnapped<T>

## Signature 

```
T EvaluateAttributeSnapped<T>(
    T                    x,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _offset_ 

--------------------------------------------------------------------------------
# EvaluateAttributeSnapped<T, N:int>

## Signature 

```
vector<T,N> EvaluateAttributeSnapped<T, N:int>(
    vector<T,N>          x,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _offset_ 

--------------------------------------------------------------------------------
# EvaluateAttributeSnapped<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> EvaluateAttributeSnapped<T, N:int, M:int>(
    matrix<T,N,M>        x,
    vector<int,2>        offset);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _offset_ 

--------------------------------------------------------------------------------
# exp<T>

## Signature 

```
T exp<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# exp<T, N:int>

## Signature 

```
vector<T,N> exp<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# exp<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> exp<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# exp2<T>

## Signature 

```
T exp2<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# exp2<T, N:int>

## Signature 

```
vector<T,N> exp2<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# exp2<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> exp2<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# f16tof32<N:int>

## Signature 

```
vector<float,N> f16tof32<N:int>(vector<uint,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# f32tof16<N:int>

## Signature 

```
vector<uint,N> f32tof16<N:int>(vector<float,N> value);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# faceforward<T, N:int>

## Signature 

```
vector<T,N> faceforward<T, N:int>(
    vector<T,N>          n,
    vector<T,N>          i,
    vector<T,N>          ng);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _n_ 
* _i_ 
* _ng_ 

--------------------------------------------------------------------------------
# firstbithigh<N:int>

## Signature 

```
vector<int,N> firstbithigh<N:int>(vector<int,N> value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# firstbithigh<N:int>

## Signature 

```
vector<uint,N> firstbithigh<N:int>(vector<uint,N> value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# firstbitlow<N:int>

## Signature 

```
vector<int,N> firstbitlow<N:int>(vector<int,N> value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# firstbitlow<N:int>

## Signature 

```
vector<uint,N> firstbitlow<N:int>(vector<uint,N> value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# floor<T>

## Signature 

```
T floor<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# floor<T, N:int>

## Signature 

```
vector<T,N> floor<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# floor<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> floor<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# fma<N:int>

## Signature 

```
vector<double,N> fma<N:int>(
    vector<double,N>     a,
    vector<double,N>     b,
    vector<double,N>     c);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _a_ 
* _b_ 
* _c_ 

--------------------------------------------------------------------------------
# fma<N:int, M:int>

## Signature 

```
matrix<double,N,M> fma<N:int, M:int>(
    matrix<double,N,M>   a,
    matrix<double,N,M>   b,
    matrix<double,N,M>   c);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _M_ 
* _a_ 
* _b_ 
* _c_ 

--------------------------------------------------------------------------------
# fmod<T>

## Signature 

```
T fmod<T>(
    T                    x,
    T                    y);
```

## Target Availability

CPP, CUDA, HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# fmod<T, N:int>

## Signature 

```
vector<T,N> fmod<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# fmod<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> fmod<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# frac<T>

## Signature 

```
T frac<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# frac<T, N:int>

## Signature 

```
vector<T,N> frac<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# frac<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> frac<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# frexp<T>

## Signature 

```
T frexp<T>(
    T                    x,
    out T                exp);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _exp_ 

--------------------------------------------------------------------------------
# frexp<T, N:int>

## Signature 

```
vector<T,N> frexp<T, N:int>(
    vector<T,N>          x,
    out vector<T,N>      exp);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _exp_ 

--------------------------------------------------------------------------------
# frexp<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> frexp<T, N:int, M:int>(
    matrix<T,N,M>        x,
    out matrix<T,N,M>    exp);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _exp_ 

--------------------------------------------------------------------------------
# fwidth<T>

## Signature 

```
T fwidth<T>(T x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# fwidth<T, N:int>

## Signature 

```
vector<T,N> fwidth<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# fwidth<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> fwidth<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# GetAttributeAtVertex<T>

## Signature 

```
T GetAttributeAtVertex<T>(
    T                    attribute,
    uint                 vertexIndex);
```

## Requirements

GLSL GL_NV_fragment_shader_barycentric, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _attribute_ 
* _vertexIndex_ 

--------------------------------------------------------------------------------
# GetAttributeAtVertex<T, N:int>

## Signature 

```
vector<T,N> GetAttributeAtVertex<T, N:int>(
    vector<T,N>          attribute,
    uint                 vertexIndex);
```

## Requirements

GLSL GL_NV_fragment_shader_barycentric, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _attribute_ 
* _vertexIndex_ 

--------------------------------------------------------------------------------
# GetAttributeAtVertex<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> GetAttributeAtVertex<T, N:int, M:int>(
    matrix<T,N,M>        attribute,
    uint                 vertexIndex);
```

## Requirements

GLSL GL_NV_fragment_shader_barycentric, GLSL450

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _attribute_ 
* _vertexIndex_ 

--------------------------------------------------------------------------------
# GetRenderTargetSampleCount

## Signature 

```
uint GetRenderTargetSampleCount();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# GetRenderTargetSamplePosition

## Signature 

```
vector<float,2> GetRenderTargetSamplePosition(int Index);
```

## Target Availability

HLSL

## Parameters

* _Index_ 

--------------------------------------------------------------------------------
# GroupMemoryBarrier

## Signature 

```
void GroupMemoryBarrier();
```

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# GroupMemoryBarrierWithGroupSync

## Signature 

```
void GroupMemoryBarrierWithGroupSync();
```

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# InterlockedAdd

## Signature 

```
void InterlockedAdd(
    int                  dest,
    int                  value);
void InterlockedAdd(
    uint                 dest,
    uint                 value);
void InterlockedAdd(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedAdd(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedAnd

## Signature 

```
void InterlockedAnd(
    int                  dest,
    int                  value);
void InterlockedAnd(
    uint                 dest,
    uint                 value);
void InterlockedAnd(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedAnd(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedCompareExchange

## Signature 

```
void InterlockedCompareExchange(
    int                  dest,
    int                  compare_value,
    int                  value,
    out int              original_value);
void InterlockedCompareExchange(
    uint                 dest,
    uint                 compare_value,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _compare_value_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedCompareStore

## Signature 

```
void InterlockedCompareStore(
    int                  dest,
    int                  compare_value,
    int                  value);
void InterlockedCompareStore(
    uint                 dest,
    uint                 compare_value,
    uint                 value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _compare_value_ 
* _value_ 

--------------------------------------------------------------------------------
# InterlockedExchange

## Signature 

```
void InterlockedExchange(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedExchange(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedMax

## Signature 

```
void InterlockedMax(
    int                  dest,
    int                  value);
void InterlockedMax(
    uint                 dest,
    uint                 value);
void InterlockedMax(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedMax(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedMin

## Signature 

```
void InterlockedMin(
    int                  dest,
    int                  value);
void InterlockedMin(
    uint                 dest,
    uint                 value);
void InterlockedMin(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedMin(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedOr

## Signature 

```
void InterlockedOr(
    int                  dest,
    int                  value);
void InterlockedOr(
    uint                 dest,
    uint                 value);
void InterlockedOr(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedOr(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# InterlockedXor

## Signature 

```
void InterlockedXor(
    int                  dest,
    int                  value);
void InterlockedXor(
    uint                 dest,
    uint                 value);
void InterlockedXor(
    int                  dest,
    int                  value,
    out int              original_value);
void InterlockedXor(
    uint                 dest,
    uint                 value,
    out uint             original_value);
```

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _dest_ 
* _value_ 
* _original_value_ 

--------------------------------------------------------------------------------
# isfinite<T>

## Signature 

```
bool isfinite<T>(T x);
```

## Target Availability

CPP, CUDA, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# isfinite<T, N:int>

## Signature 

```
vector<bool,N> isfinite<T, N:int>(vector<T,N> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# isfinite<T, N:int, M:int>

## Signature 

```
matrix<bool,N,M> isfinite<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# isinf<T>

## Signature 

```
bool isinf<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# isinf<T, N:int>

## Signature 

```
vector<bool,N> isinf<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# isinf<T, N:int, M:int>

## Signature 

```
matrix<bool,N,M> isinf<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# isnan<T>

## Signature 

```
bool isnan<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# isnan<T, N:int>

## Signature 

```
vector<bool,N> isnan<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# isnan<T, N:int, M:int>

## Signature 

```
matrix<bool,N,M> isnan<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# ldexp<T>

## Signature 

```
T ldexp<T>(
    T                    x,
    T                    exp);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 
* _exp_ 

--------------------------------------------------------------------------------
# ldexp<T, N:int>

## Signature 

```
vector<T,N> ldexp<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          exp);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _exp_ 

--------------------------------------------------------------------------------
# ldexp<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> ldexp<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        exp);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _exp_ 

--------------------------------------------------------------------------------
# length<T, N:int>

## Signature 

```
T length<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# lerp<T>

## Signature 

```
T lerp<T>(
    T                    x,
    T                    y,
    T                    s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 
* _s_ 

--------------------------------------------------------------------------------
# lerp<T, N:int>

## Signature 

```
vector<T,N> lerp<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y,
    vector<T,N>          s);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 
* _s_ 

--------------------------------------------------------------------------------
# lerp<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> lerp<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        y,
    matrix<T,N,M>        s);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 
* _s_ 

--------------------------------------------------------------------------------
# lit

## Signature 

```
vector<float,4> lit(
    float                n_dot_l,
    float                n_dot_h,
    float                m);
```

## Target Availability

HLSL

## Parameters

* _n_dot_l_ 
* _n_dot_h_ 
* _m_ 

--------------------------------------------------------------------------------
# log<T>

## Signature 

```
T log<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# log<T, N:int>

## Signature 

```
vector<T,N> log<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# log<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> log<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# log10<T>

## Signature 

```
T log10<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# log10<T, N:int>

## Signature 

```
vector<T,N> log10<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# log10<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> log10<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# log2<T>

## Signature 

```
T log2<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# log2<T, N:int>

## Signature 

```
vector<T,N> log2<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# log2<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> log2<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# mad<T>

## Signature 

```
T mad<T>(
    T                    mvalue,
    T                    avalue,
    T                    bvalue);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mvalue_ 
* _avalue_ 
* _bvalue_ 

--------------------------------------------------------------------------------
# mad<T, N:int>

## Signature 

```
vector<T,N> mad<T, N:int>(
    vector<T,N>          mvalue,
    vector<T,N>          avalue,
    vector<T,N>          bvalue);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mvalue_ 
* _avalue_ 
* _bvalue_ 

--------------------------------------------------------------------------------
# mad<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> mad<T, N:int, M:int>(
    matrix<T,N,M>        mvalue,
    matrix<T,N,M>        avalue,
    matrix<T,N,M>        bvalue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mvalue_ 
* _avalue_ 
* _bvalue_ 

--------------------------------------------------------------------------------
# max<T>

## Signature 

```
T max<T>(
    T                    x,
    T                    y);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# max<T, N:int>

## Signature 

```
vector<T,N> max<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# max<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> max<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# min<T>

## Signature 

```
T min<T>(
    T                    x,
    T                    y);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# min<T, N:int>

## Signature 

```
vector<T,N> min<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# min<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> min<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# modf<T>

## Signature 

```
T modf<T>(
    T                    x,
    out T                ip);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 
* _ip_ 

--------------------------------------------------------------------------------
# modf<T, N:int>

## Signature 

```
vector<T,N> modf<T, N:int>(
    vector<T,N>          x,
    out vector<T,N>      ip);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _ip_ 

--------------------------------------------------------------------------------
# modf<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> modf<T, N:int, M:int>(
    matrix<T,N,M>        x,
    out matrix<T,N,M>    ip);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _ip_ 

--------------------------------------------------------------------------------
# msad4

## Signature 

```
vector<uint,4> msad4(
    uint                 reference,
    vector<uint,2>       source,
    vector<uint,4>       accum);
```

## Target Availability

HLSL

## Parameters

* _reference_ 
* _source_ 
* _accum_ 

--------------------------------------------------------------------------------
# mul<T>

## Signature 

```
T mul<T>(
    T                    x,
    T                    y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# mul<T, N:int>

## Signature 

```
vector<T,N> mul<T, N:int>(
    vector<T,N>          x,
    T                    y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# mul<T, N:int>

## Signature 

```
vector<T,N> mul<T, N:int>(
    T                    x,
    vector<T,N>          y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# mul<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> mul<T, N:int, M:int>(
    matrix<T,N,M>        x,
    T                    y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# mul<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> mul<T, N:int, M:int>(
    T                    x,
    matrix<T,N,M>        y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# mul<T, N:int>

## Signature 

```
T mul<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# mul<T, N:int, M:int>

## Signature 

```
vector<T,M> mul<T, N:int, M:int>(
    vector<T,N>          left,
    matrix<T,N,M>        right);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _left_ 
* _right_ 

--------------------------------------------------------------------------------
# mul<T, N:int, M:int>

## Signature 

```
vector<T,N> mul<T, N:int, M:int>(
    matrix<T,N,M>        left,
    vector<T,M>          right);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _left_ 
* _right_ 

--------------------------------------------------------------------------------
# mul<T, R:int, N:int, C:int>

## Signature 

```
matrix<T,R,C> mul<T, R:int, N:int, C:int>(
    matrix<T,R,N>        right,
    matrix<T,N,C>        left);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _R_ 
* _N_ 
* _C_ 
* _right_ 
* _left_ 

--------------------------------------------------------------------------------
# noise<N:int>

## Signature 

```
float noise<N:int>(vector<float,N> x);
```

## Target Availability

HLSL

## Parameters

* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# NonUniformResourceIndex

## Signature 

```
uint NonUniformResourceIndex(uint index);
int NonUniformResourceIndex(int index);
```

## Requirements

GLSL GL_EXT_nonuniform_qualifier

## Target Availability

GLSL, HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# normalize<T, N:int>

## Signature 

```
vector<T,N> normalize<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# pow<T>

## Signature 

```
T pow<T>(
    T                    x,
    T                    y);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# pow<T, N:int>

## Signature 

```
vector<T,N> pow<T, N:int>(
    vector<T,N>          x,
    vector<T,N>          y);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# pow<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> pow<T, N:int, M:int>(
    matrix<T,N,M>        x,
    matrix<T,N,M>        y);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _y_ 

--------------------------------------------------------------------------------
# Process2DQuadTessFactorsAvg

## Signature 

```
void Process2DQuadTessFactorsAvg(
    in vector<float,4>   RawEdgeFactors,
    in vector<float,2>   InsideScale,
    out vector<float,4>  RoundedEdgeTessFactors,
    out vector<float,2>  RoundedInsideTessFactors,
    out vector<float,2>  UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# Process2DQuadTessFactorsMax

## Signature 

```
void Process2DQuadTessFactorsMax(
    in vector<float,4>   RawEdgeFactors,
    in vector<float,2>   InsideScale,
    out vector<float,4>  RoundedEdgeTessFactors,
    out vector<float,2>  RoundedInsideTessFactors,
    out vector<float,2>  UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# Process2DQuadTessFactorsMin

## Signature 

```
void Process2DQuadTessFactorsMin(
    in vector<float,4>   RawEdgeFactors,
    in vector<float,2>   InsideScale,
    out vector<float,4>  RoundedEdgeTessFactors,
    out vector<float,2>  RoundedInsideTessFactors,
    out vector<float,2>  UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# ProcessIsolineTessFactors

## Signature 

```
void ProcessIsolineTessFactors(
    in float             RawDetailFactor,
    in float             RawDensityFactor,
    out float            RoundedDetailFactor,
    out float            RoundedDensityFactor);
```

## Target Availability

HLSL

## Parameters

* _RawDetailFactor_ 
* _RawDensityFactor_ 
* _RoundedDetailFactor_ 
* _RoundedDensityFactor_ 

--------------------------------------------------------------------------------
# ProcessQuadTessFactorsAvg

## Signature 

```
void ProcessQuadTessFactorsAvg(
    in vector<float,4>   RawEdgeFactors,
    in float             InsideScale,
    out vector<float,4>  RoundedEdgeTessFactors,
    out vector<float,2>  RoundedInsideTessFactors,
    out vector<float,2>  UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# ProcessQuadTessFactorsMax

## Signature 

```
void ProcessQuadTessFactorsMax(
    in vector<float,4>   RawEdgeFactors,
    in float             InsideScale,
    out vector<float,4>  RoundedEdgeTessFactors,
    out vector<float,2>  RoundedInsideTessFactors,
    out vector<float,2>  UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# ProcessQuadTessFactorsMin

## Signature 

```
void ProcessQuadTessFactorsMin(
    in vector<float,4>   RawEdgeFactors,
    in float             InsideScale,
    out vector<float,4>  RoundedEdgeTessFactors,
    out vector<float,2>  RoundedInsideTessFactors,
    out vector<float,2>  UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# ProcessTriTessFactorsAvg

## Signature 

```
void ProcessTriTessFactorsAvg(
    in vector<float,3>   RawEdgeFactors,
    in float             InsideScale,
    out vector<float,3>  RoundedEdgeTessFactors,
    out float            RoundedInsideTessFactor,
    out float            UnroundedInsideTessFactor);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactor_ 
* _UnroundedInsideTessFactor_ 

--------------------------------------------------------------------------------
# ProcessTriTessFactorsMax

## Signature 

```
void ProcessTriTessFactorsMax(
    in vector<float,3>   RawEdgeFactors,
    in float             InsideScale,
    out vector<float,3>  RoundedEdgeTessFactors,
    out float            RoundedInsideTessFactor,
    out float            UnroundedInsideTessFactor);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactor_ 
* _UnroundedInsideTessFactor_ 

--------------------------------------------------------------------------------
# ProcessTriTessFactorsMin

## Signature 

```
void ProcessTriTessFactorsMin(
    in vector<float,3>   RawEdgeFactors,
    in float             InsideScale,
    out vector<float,3>  RoundedEdgeTessFactors,
    out float            RoundedInsideTessFactors,
    out float            UnroundedInsideTessFactors);
```

## Target Availability

HLSL

## Parameters

* _RawEdgeFactors_ 
* _InsideScale_ 
* _RoundedEdgeTessFactors_ 
* _RoundedInsideTessFactors_ 
* _UnroundedInsideTessFactors_ 

--------------------------------------------------------------------------------
# radians<T>

## Signature 

```
T radians<T>(T x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# radians<T, N:int>

## Signature 

```
vector<T,N> radians<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# radians<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> radians<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# rcp<T>

## Signature 

```
T rcp<T>(T x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# rcp<T, N:int>

## Signature 

```
vector<T,N> rcp<T, N:int>(vector<T,N> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# rcp<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> rcp<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# reflect<T, N:int>

## Signature 

```
vector<T,N> reflect<T, N:int>(
    vector<T,N>          i,
    vector<T,N>          n);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _i_ 
* _n_ 

--------------------------------------------------------------------------------
# refract<T, N:int>

## Signature 

```
vector<T,N> refract<T, N:int>(
    vector<T,N>          i,
    vector<T,N>          n,
    T                    eta);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _i_ 
* _n_ 
* _eta_ 

--------------------------------------------------------------------------------
# reversebits<N:int>

## Signature 

```
vector<uint,N> reversebits<N:int>(vector<uint,N> value);
```

## Target Availability

GLSL, HLSL

## Parameters

* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# round<T>

## Signature 

```
T round<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# round<T, N:int>

## Signature 

```
vector<T,N> round<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# round<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> round<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# rsqrt<T>

## Signature 

```
T rsqrt<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# rsqrt<T, N:int>

## Signature 

```
vector<T,N> rsqrt<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# rsqrt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> rsqrt<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# saturate<T>

## Signature 

```
T saturate<T>(T x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# saturate<T, N:int>

## Signature 

```
vector<T,N> saturate<T, N:int>(vector<T,N> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# saturate<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> saturate<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# sign<T>

## Signature 

```
int sign<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# sign<T, N:int>

## Signature 

```
vector<int,N> sign<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# sign<T, N:int, M:int>

## Signature 

```
matrix<int,N,M> sign<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# sin<T>

## Signature 

```
T sin<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# sin<T, N:int>

## Signature 

```
vector<T,N> sin<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# sin<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> sin<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# sincos<T>

## Signature 

```
void sincos<T>(
    T                    x,
    out T                s,
    out T                c);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _x_ 
* _s_ 
* _c_ 

--------------------------------------------------------------------------------
# sincos<T, N:int>

## Signature 

```
void sincos<T, N:int>(
    vector<T,N>          x,
    out vector<T,N>      s,
    out vector<T,N>      c);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 
* _s_ 
* _c_ 

--------------------------------------------------------------------------------
# sincos<T, N:int, M:int>

## Signature 

```
void sincos<T, N:int, M:int>(
    matrix<T,N,M>        x,
    out matrix<T,N,M>    s,
    out matrix<T,N,M>    c);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 
* _s_ 
* _c_ 

--------------------------------------------------------------------------------
# sinh<T>

## Signature 

```
T sinh<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# sinh<T, N:int>

## Signature 

```
vector<T,N> sinh<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# sinh<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> sinh<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# smoothstep<T>

## Signature 

```
T smoothstep<T>(
    T                    min,
    T                    max,
    T                    x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _min_ 
* _max_ 
* _x_ 

--------------------------------------------------------------------------------
# smoothstep<T, N:int>

## Signature 

```
vector<T,N> smoothstep<T, N:int>(
    vector<T,N>          min,
    vector<T,N>          max,
    vector<T,N>          x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _min_ 
* _max_ 
* _x_ 

--------------------------------------------------------------------------------
# smoothstep<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> smoothstep<T, N:int, M:int>(
    matrix<T,N,M>        min,
    matrix<T,N,M>        max,
    matrix<T,N,M>        x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _min_ 
* _max_ 
* _x_ 

--------------------------------------------------------------------------------
# sqrt<T>

## Signature 

```
T sqrt<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# sqrt<T, N:int>

## Signature 

```
vector<T,N> sqrt<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# sqrt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> sqrt<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# step<T>

## Signature 

```
T step<T>(
    T                    y,
    T                    x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _y_ 
* _x_ 

--------------------------------------------------------------------------------
# step<T, N:int>

## Signature 

```
vector<T,N> step<T, N:int>(
    vector<T,N>          y,
    vector<T,N>          x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _y_ 
* _x_ 

--------------------------------------------------------------------------------
# step<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> step<T, N:int, M:int>(
    matrix<T,N,M>        y,
    matrix<T,N,M>        x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _y_ 
* _x_ 

--------------------------------------------------------------------------------
# tan<T>

## Signature 

```
T tan<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# tan<T, N:int>

## Signature 

```
vector<T,N> tan<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# tan<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> tan<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# tanh<T>

## Signature 

```
T tanh<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# tanh<T, N:int>

## Signature 

```
vector<T,N> tanh<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# tanh<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> tanh<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# transpose<T, N:int, M:int>

## Signature 

```
matrix<T,M,N> transpose<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# trunc<T>

## Signature 

```
T trunc<T>(T x);
```

## Target Availability

CPP, CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _x_ 

--------------------------------------------------------------------------------
# trunc<T, N:int>

## Signature 

```
vector<T,N> trunc<T, N:int>(vector<T,N> x);
```

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _x_ 

--------------------------------------------------------------------------------
# trunc<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> trunc<T, N:int, M:int>(matrix<T,N,M> x);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _x_ 

--------------------------------------------------------------------------------
# WaveGetConvergedMask

## Signature 

```
uint WaveGetConvergedMask();
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# WaveGetActiveMask

## Signature 

```
uint WaveGetActiveMask();
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# WaveMaskIsFirstLane

## Signature 

```
bool WaveMaskIsFirstLane(uint mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 

--------------------------------------------------------------------------------
# WaveMaskAllTrue

## Signature 

```
bool WaveMaskAllTrue(
    uint                 mask,
    bool                 condition);
```

## Requirements

GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 
* _condition_ 

--------------------------------------------------------------------------------
# WaveMaskAnyTrue

## Signature 

```
bool WaveMaskAnyTrue(
    uint                 mask,
    bool                 condition);
```

## Requirements

GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 
* _condition_ 

--------------------------------------------------------------------------------
# WaveMaskBallot

## Signature 

```
uint WaveMaskBallot(
    uint                 mask,
    bool                 condition);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 
* _condition_ 

--------------------------------------------------------------------------------
# WaveMaskCountBits

## Signature 

```
uint WaveMaskCountBits(
    uint                 mask,
    bool                 value);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot

## Target Availability

CUDA, HLSL

## Parameters

* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# AllMemoryBarrierWithWaveMaskSync

## Signature 

```
void AllMemoryBarrierWithWaveMaskSync(uint mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 

--------------------------------------------------------------------------------
# GroupMemoryBarrierWithWaveMaskSync

## Signature 

```
void GroupMemoryBarrierWithWaveMaskSync(uint mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 

--------------------------------------------------------------------------------
# AllMemoryBarrierWithWaveSync

## Signature 

```
void AllMemoryBarrierWithWaveSync();
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# GroupMemoryBarrierWithWaveSync

## Signature 

```
void GroupMemoryBarrierWithWaveSync();
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# WaveMaskBroadcastLaneAt<T>

## Signature 

```
T WaveMaskBroadcastLaneAt<T>(
    uint                 mask,
    T                    value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskBroadcastLaneAt<T, N:int>

## Signature 

```
vector<T,N> WaveMaskBroadcastLaneAt<T, N:int>(
    uint                 mask,
    vector<T,N>          value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskBroadcastLaneAt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskBroadcastLaneAt<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        value,
    int                  lane);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskReadLaneAt<T>

## Signature 

```
T WaveMaskReadLaneAt<T>(
    uint                 mask,
    T                    value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskReadLaneAt<T, N:int>

## Signature 

```
vector<T,N> WaveMaskReadLaneAt<T, N:int>(
    uint                 mask,
    vector<T,N>          value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskReadLaneAt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskReadLaneAt<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        value,
    int                  lane);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskShuffle<T>

## Signature 

```
T WaveMaskShuffle<T>(
    uint                 mask,
    T                    value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskShuffle<T, N:int>

## Signature 

```
vector<T,N> WaveMaskShuffle<T, N:int>(
    uint                 mask,
    vector<T,N>          value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskShuffle<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskShuffle<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        value,
    int                  lane);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixCountBits

## Signature 

```
uint WaveMaskPrefixCountBits(
    uint                 mask,
    bool                 value);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskBitAnd<T>

## Signature 

```
T WaveMaskBitAnd<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitAnd<T, N:int>

## Signature 

```
vector<T,N> WaveMaskBitAnd<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitAnd<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskBitAnd<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitOr<T>

## Signature 

```
T WaveMaskBitOr<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitOr<T, N:int>

## Signature 

```
vector<T,N> WaveMaskBitOr<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitOr<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskBitOr<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitXor<T>

## Signature 

```
T WaveMaskBitXor<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitXor<T, N:int>

## Signature 

```
vector<T,N> WaveMaskBitXor<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskBitXor<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskBitXor<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMax<T>

## Signature 

```
T WaveMaskMax<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMax<T, N:int>

## Signature 

```
vector<T,N> WaveMaskMax<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMax<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskMax<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMin<T>

## Signature 

```
T WaveMaskMin<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMin<T, N:int>

## Signature 

```
vector<T,N> WaveMaskMin<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMin<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskMin<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskProduct<T>

## Signature 

```
T WaveMaskProduct<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskProduct<T, N:int>

## Signature 

```
vector<T,N> WaveMaskProduct<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskProduct<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskProduct<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskSum<T>

## Signature 

```
T WaveMaskSum<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskSum<T, N:int>

## Signature 

```
vector<T,N> WaveMaskSum<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskSum<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskSum<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskAllEqual<T>

## Signature 

```
bool WaveMaskAllEqual<T>(
    uint                 mask,
    T                    value);
```

## Requirements

CUDA SM 7.0, GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskAllEqual<T, N:int>

## Signature 

```
bool WaveMaskAllEqual<T, N:int>(
    uint                 mask,
    vector<T,N>          value);
```

## Requirements

CUDA SM 7.0, GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskAllEqual<T, N:int, M:int>

## Signature 

```
bool WaveMaskAllEqual<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        value);
```

## Requirements

CUDA SM 7.0

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixProduct<T>

## Signature 

```
T WaveMaskPrefixProduct<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixProduct<T, N:int>

## Signature 

```
vector<T,N> WaveMaskPrefixProduct<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixProduct<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskPrefixProduct<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixSum<T>

## Signature 

```
T WaveMaskPrefixSum<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixSum<T, N:int>

## Signature 

```
vector<T,N> WaveMaskPrefixSum<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixSum<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskPrefixSum<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskReadLaneFirst<T>

## Signature 

```
T WaveMaskReadLaneFirst<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskReadLaneFirst<T, N:int>

## Signature 

```
vector<T,N> WaveMaskReadLaneFirst<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskReadLaneFirst<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskReadLaneFirst<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskMatch<T>

## Signature 

```
uint WaveMaskMatch<T>(
    uint                 mask,
    T                    value);
```

## Requirements

CUDA SM 7.0

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskMatch<T, N:int>

## Signature 

```
uint WaveMaskMatch<T, N:int>(
    uint                 mask,
    vector<T,N>          value);
```

## Requirements

CUDA SM 7.0

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskMatch<T, N:int, M:int>

## Signature 

```
uint WaveMaskMatch<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        value);
```

## Requirements

CUDA SM 7.0

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitAnd<T>

## Signature 

```
T WaveMaskPrefixBitAnd<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitAnd<T, N:int>

## Signature 

```
vector<T,N> WaveMaskPrefixBitAnd<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitAnd<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskPrefixBitAnd<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitOr<T>

## Signature 

```
T WaveMaskPrefixBitOr<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitOr<T, N:int>

## Signature 

```
vector<T,N> WaveMaskPrefixBitOr<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitOr<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskPrefixBitOr<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitXor<T>

## Signature 

```
T WaveMaskPrefixBitXor<T>(
    uint                 mask,
    T                    expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitXor<T, N:int>

## Signature 

```
vector<T,N> WaveMaskPrefixBitXor<T, N:int>(
    uint                 mask,
    vector<T,N>          expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveMaskPrefixBitXor<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMaskPrefixBitXor<T, N:int, M:int>(
    uint                 mask,
    matrix<T,N,M>        expr);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _mask_ 
* _expr_ 

--------------------------------------------------------------------------------
# QuadReadLaneAt<T>

## Signature 

```
T QuadReadLaneAt<T>(
    T                    sourceValue,
    uint                 quadLaneID);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _sourceValue_ 
* _quadLaneID_ 

--------------------------------------------------------------------------------
# QuadReadLaneAt<T, N:int>

## Signature 

```
vector<T,N> QuadReadLaneAt<T, N:int>(
    vector<T,N>          sourceValue,
    uint                 quadLaneID);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _sourceValue_ 
* _quadLaneID_ 

--------------------------------------------------------------------------------
# QuadReadLaneAt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> QuadReadLaneAt<T, N:int, M:int>(
    matrix<T,N,M>        sourceValue,
    uint                 quadLaneID);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _sourceValue_ 
* _quadLaneID_ 

--------------------------------------------------------------------------------
# QuadReadAcrossX<T>

## Signature 

```
T QuadReadAcrossX<T>(T localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossX<T, N:int>

## Signature 

```
vector<T,N> QuadReadAcrossX<T, N:int>(vector<T,N> localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossX<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> QuadReadAcrossX<T, N:int, M:int>(matrix<T,N,M> localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossY<T>

## Signature 

```
T QuadReadAcrossY<T>(T localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossY<T, N:int>

## Signature 

```
vector<T,N> QuadReadAcrossY<T, N:int>(vector<T,N> localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossY<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> QuadReadAcrossY<T, N:int, M:int>(matrix<T,N,M> localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossDiagonal<T>

## Signature 

```
T QuadReadAcrossDiagonal<T>(T localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossDiagonal<T, N:int>

## Signature 

```
vector<T,N> QuadReadAcrossDiagonal<T, N:int>(vector<T,N> localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _localValue_ 

--------------------------------------------------------------------------------
# QuadReadAcrossDiagonal<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> QuadReadAcrossDiagonal<T, N:int, M:int>(matrix<T,N,M> localValue);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _localValue_ 

--------------------------------------------------------------------------------
# WaveActiveBitAnd<T>

## Signature 

```
T WaveActiveBitAnd<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitAnd<T, N:int>

## Signature 

```
vector<T,N> WaveActiveBitAnd<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitAnd<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveBitAnd<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitOr<T>

## Signature 

```
T WaveActiveBitOr<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitOr<T, N:int>

## Signature 

```
vector<T,N> WaveActiveBitOr<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitOr<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveBitOr<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitXor<T>

## Signature 

```
T WaveActiveBitXor<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitXor<T, N:int>

## Signature 

```
vector<T,N> WaveActiveBitXor<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveBitXor<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveBitXor<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveMax<T>

## Signature 

```
T WaveActiveMax<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveMax<T, N:int>

## Signature 

```
vector<T,N> WaveActiveMax<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveMax<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveMax<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveMin<T>

## Signature 

```
T WaveActiveMin<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveMin<T, N:int>

## Signature 

```
vector<T,N> WaveActiveMin<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveMin<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveMin<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveProduct<T>

## Signature 

```
T WaveActiveProduct<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveProduct<T, N:int>

## Signature 

```
vector<T,N> WaveActiveProduct<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveProduct<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveProduct<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveSum<T>

## Signature 

```
T WaveActiveSum<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveSum<T, N:int>

## Signature 

```
vector<T,N> WaveActiveSum<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveSum<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveActiveSum<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveActiveAllEqual<T>

## Signature 

```
bool WaveActiveAllEqual<T>(T value);
```

## Requirements

GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveActiveAllEqual<T, N:int>

## Signature 

```
bool WaveActiveAllEqual<T, N:int>(vector<T,N> value);
```

## Requirements

GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveActiveAllEqual<T, N:int, M:int>

## Signature 

```
bool WaveActiveAllEqual<T, N:int, M:int>(matrix<T,N,M> value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveActiveAllTrue

## Signature 

```
bool WaveActiveAllTrue(bool condition);
```

## Requirements

GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _condition_ 

--------------------------------------------------------------------------------
# WaveActiveAnyTrue

## Signature 

```
bool WaveActiveAnyTrue(bool condition);
```

## Requirements

GLSL GL_KHR_shader_subgroup_vote, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _condition_ 

--------------------------------------------------------------------------------
# WaveActiveBallot

## Signature 

```
vector<uint,4> WaveActiveBallot(bool condition);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _condition_ 

--------------------------------------------------------------------------------
# WaveActiveCountBits

## Signature 

```
uint WaveActiveCountBits(bool value);
```

## Target Availability

HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# WaveGetLaneCount

## Signature 

```
uint WaveGetLaneCount();
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# WaveGetLaneIndex

## Signature 

```
uint WaveGetLaneIndex();
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# WaveIsFirstLane

## Signature 

```
bool WaveIsFirstLane();
```

## Requirements

GLSL GL_KHR_shader_subgroup_basic, SPIR-V 1.3

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# _WaveCountBits

## Signature 

```
uint _WaveCountBits(vector<uint,4> value);
```

## Target Availability

HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# WavePrefixProduct<T>

## Signature 

```
T WavePrefixProduct<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WavePrefixProduct<T, N:int>

## Signature 

```
vector<T,N> WavePrefixProduct<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WavePrefixProduct<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WavePrefixProduct<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WavePrefixSum<T>

## Signature 

```
T WavePrefixSum<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WavePrefixSum<T, N:int>

## Signature 

```
vector<T,N> WavePrefixSum<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WavePrefixSum<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WavePrefixSum<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveReadLaneFirst<T>

## Signature 

```
T WaveReadLaneFirst<T>(T expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveReadLaneFirst<T, N:int>

## Signature 

```
vector<T,N> WaveReadLaneFirst<T, N:int>(vector<T,N> expr);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveReadLaneFirst<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveReadLaneFirst<T, N:int, M:int>(matrix<T,N,M> expr);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 

--------------------------------------------------------------------------------
# WaveBroadcastLaneAt<T>

## Signature 

```
T WaveBroadcastLaneAt<T>(
    T                    value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveBroadcastLaneAt<T, N:int>

## Signature 

```
vector<T,N> WaveBroadcastLaneAt<T, N:int>(
    vector<T,N>          value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveBroadcastLaneAt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveBroadcastLaneAt<T, N:int, M:int>(
    matrix<T,N,M>        value,
    int                  lane);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveReadLaneAt<T>

## Signature 

```
T WaveReadLaneAt<T>(
    T                    value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveReadLaneAt<T, N:int>

## Signature 

```
vector<T,N> WaveReadLaneAt<T, N:int>(
    vector<T,N>          value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveReadLaneAt<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveReadLaneAt<T, N:int, M:int>(
    matrix<T,N,M>        value,
    int                  lane);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveShuffle<T>

## Signature 

```
T WaveShuffle<T>(
    T                    value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveShuffle<T, N:int>

## Signature 

```
vector<T,N> WaveShuffle<T, N:int>(
    vector<T,N>          value,
    int                  lane);
```

## Requirements

GLSL GL_KHR_shader_subgroup_shuffle, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WaveShuffle<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveShuffle<T, N:int, M:int>(
    matrix<T,N,M>        value,
    int                  lane);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 
* _lane_ 

--------------------------------------------------------------------------------
# WavePrefixCountBits

## Signature 

```
uint WavePrefixCountBits(bool value);
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL

## Parameters

* _value_ 

--------------------------------------------------------------------------------
# WaveGetConvergedMulti

## Signature 

```
vector<uint,4> WaveGetConvergedMulti();
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL


--------------------------------------------------------------------------------
# WaveGetActiveMulti

## Signature 

```
vector<uint,4> WaveGetActiveMulti();
```

## Requirements

GLSL GL_KHR_shader_subgroup_ballot, SPIR-V 1.3

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# WaveMatch<T>

## Signature 

```
vector<uint,4> WaveMatch<T>(T value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMatch<T, N:int>

## Signature 

```
vector<uint,4> WaveMatch<T, N:int>(vector<T,N> value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMatch<T, N:int, M:int>

## Signature 

```
vector<uint,4> WaveMatch<T, N:int, M:int>(matrix<T,N,M> value);
```

## Target Availability

HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixCountBits

## Signature 

```
uint WaveMultiPrefixCountBits(
    bool                 value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitAnd<T>

## Signature 

```
T WaveMultiPrefixBitAnd<T>(
    T                    expr,
    vector<uint,4>       mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitAnd<T, N:int>

## Signature 

```
vector<T,N> WaveMultiPrefixBitAnd<T, N:int>(
    vector<T,N>          expr,
    vector<uint,4>       mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitAnd<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMultiPrefixBitAnd<T, N:int, M:int>(
    matrix<T,N,M>        expr,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitOr<T>

## Signature 

```
T WaveMultiPrefixBitOr<T>(
    T                    expr,
    vector<uint,4>       mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitOr<T, N:int>

## Signature 

```
vector<T,N> WaveMultiPrefixBitOr<T, N:int>(
    vector<T,N>          expr,
    vector<uint,4>       mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitOr<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMultiPrefixBitOr<T, N:int, M:int>(
    matrix<T,N,M>        expr,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitXor<T>

## Signature 

```
T WaveMultiPrefixBitXor<T>(
    T                    expr,
    vector<uint,4>       mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitXor<T, N:int>

## Signature 

```
vector<T,N> WaveMultiPrefixBitXor<T, N:int>(
    vector<T,N>          expr,
    vector<uint,4>       mask);
```

## Requirements

GLSL GL_KHR_shader_subgroup_arithmetic, SPIR-V 1.3

## Target Availability

CUDA, GLSL, HLSL

## Parameters

* _T_ 
* _N_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixBitXor<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMultiPrefixBitXor<T, N:int, M:int>(
    matrix<T,N,M>        expr,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _expr_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixProduct<T>

## Signature 

```
T WaveMultiPrefixProduct<T>(
    T                    value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixProduct<T, N:int>

## Signature 

```
vector<T,N> WaveMultiPrefixProduct<T, N:int>(
    vector<T,N>          value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixProduct<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMultiPrefixProduct<T, N:int, M:int>(
    matrix<T,N,M>        value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixSum<T>

## Signature 

```
T WaveMultiPrefixSum<T>(
    T                    value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixSum<T, N:int>

## Signature 

```
vector<T,N> WaveMultiPrefixSum<T, N:int>(
    vector<T,N>          value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# WaveMultiPrefixSum<T, N:int, M:int>

## Signature 

```
matrix<T,N,M> WaveMultiPrefixSum<T, N:int, M:int>(
    matrix<T,N,M>        value,
    vector<uint,4>       mask);
```

## Target Availability

CUDA, HLSL

## Parameters

* _T_ 
* _N_ 
* _M_ 
* _value_ 
* _mask_ 

--------------------------------------------------------------------------------
# struct Buffer<T>

## Generic Parameters

* _T_ 

## Methods

* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# Buffer<T>.GetDimensions

## Signature 

```
void Buffer<T>.GetDimensions(out uint dim);
```

## Target Availability

HLSL

## Parameters

* _dim_ 

--------------------------------------------------------------------------------
# Buffer<T>.Load

## Signature 

```
/// See Target Availability 1
T Buffer<T>.Load(int location);
/// See Target Availability 2
T Buffer<T>.Load(
    int                  location,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# Buffer<T>.subscript

## Signature 

```
T Buffer<T>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct RWBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RWBuffer<T>.GetDimensions

## Signature 

```
void RWBuffer<T>.GetDimensions(out uint dim);
```

## Target Availability

HLSL

## Parameters

* _dim_ 

--------------------------------------------------------------------------------
# RWBuffer<T>.Load

## Signature 

```
/// See Target Availability 1
T RWBuffer<T>.Load(int location);
/// See Target Availability 2
T RWBuffer<T>.Load(
    int                  location,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RWBuffer<T>.subscript

## Signature 

```
T RWBuffer<T>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# struct RasterizerOrderedBuffer<T>

## Generic Parameters

* _T_ 

## Methods

* _subscript_ 
* _Load_ 
* _GetDimensions_ 


--------------------------------------------------------------------------------
# RasterizerOrderedBuffer<T>.GetDimensions

## Signature 

```
void RasterizerOrderedBuffer<T>.GetDimensions(out uint dim);
```

## Target Availability

HLSL

## Parameters

* _dim_ 

--------------------------------------------------------------------------------
# RasterizerOrderedBuffer<T>.Load

## Signature 

```
/// See Target Availability 1
T RasterizerOrderedBuffer<T>.Load(int location);
/// See Target Availability 2
T RasterizerOrderedBuffer<T>.Load(
    int                  location,
    out uint             status);
```

## Requirements

GLSL GL_EXT_samplerless_texture_functions

## Target Availability

* _1_ GLSL, HLSL
* _2_ HLSL

## Parameters

* _location_ 
* _status_ 

--------------------------------------------------------------------------------
# RasterizerOrderedBuffer<T>.subscript

## Signature 

```
T RasterizerOrderedBuffer<T>.subscript(uint index);
```

## Target Availability

HLSL

## Parameters

* _index_ 

--------------------------------------------------------------------------------
# RAY_FLAG_NONE

```
uint RAY_FLAG_NONE
```


--------------------------------------------------------------------------------
# RAY_FLAG_FORCE_OPAQUE

```
uint RAY_FLAG_FORCE_OPAQUE
```


--------------------------------------------------------------------------------
# RAY_FLAG_FORCE_NON_OPAQUE

```
uint RAY_FLAG_FORCE_NON_OPAQUE
```


--------------------------------------------------------------------------------
# RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH

```
uint RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
```


--------------------------------------------------------------------------------
# RAY_FLAG_SKIP_CLOSEST_HIT_SHADER

```
uint RAY_FLAG_SKIP_CLOSEST_HIT_SHADER
```


--------------------------------------------------------------------------------
# RAY_FLAG_CULL_BACK_FACING_TRIANGLES

```
uint RAY_FLAG_CULL_BACK_FACING_TRIANGLES
```


--------------------------------------------------------------------------------
# RAY_FLAG_CULL_FRONT_FACING_TRIANGLES

```
uint RAY_FLAG_CULL_FRONT_FACING_TRIANGLES
```


--------------------------------------------------------------------------------
# RAY_FLAG_CULL_OPAQUE

```
uint RAY_FLAG_CULL_OPAQUE
```


--------------------------------------------------------------------------------
# RAY_FLAG_CULL_NON_OPAQUE

```
uint RAY_FLAG_CULL_NON_OPAQUE
```


--------------------------------------------------------------------------------
# RAY_FLAG_SKIP_TRIANGLES

```
uint RAY_FLAG_SKIP_TRIANGLES
```


--------------------------------------------------------------------------------
# RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES

```
uint RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES
```


--------------------------------------------------------------------------------
# struct RayDesc

## Fields

* _Origin_ 
* _TMin_ 
* _Direction_ 
* _TMax_ 


--------------------------------------------------------------------------------
# struct RaytracingAccelerationStructure


--------------------------------------------------------------------------------
# struct BuiltInTriangleIntersectionAttributes

## Fields

* _barycentrics_ 


--------------------------------------------------------------------------------
# CallShader<Payload>

## Signature 

```
void CallShader<Payload>(
    uint                 shaderIndex,
    inout Payload        payload);
```

## Target Availability

HLSL

## Parameters

* _Payload_ 
* _shaderIndex_ 
* _payload_ 

--------------------------------------------------------------------------------
# CallShader<Payload>

## Signature 

```
```


--------------------------------------------------------------------------------
# TraceRay<payload_t>

## Signature 

```
void TraceRay<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 RayFlags,
    uint                 InstanceInclusionMask,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    uint                 MissShaderIndex,
    RayDesc              Ray,
    inout payload_t      Payload);
```

## Target Availability

HLSL

## Parameters

* _payload_t_ 
* _AccelerationStructure_ 
* _RayFlags_ 
* _InstanceInclusionMask_ 
* _RayContributionToHitGroupIndex_ 
* _MultiplierForGeometryContributionToHitGroupIndex_ 
* _MissShaderIndex_ 
* _Ray_ 
* _Payload_ 

--------------------------------------------------------------------------------
# TraceRay<payload_t>

## Signature 

```
```


--------------------------------------------------------------------------------
# ReportHit<A>

## Signature 

```
bool ReportHit<A>(
    float                tHit,
    uint                 hitKind,
    A                    attributes);
```

## Target Availability

HLSL

## Parameters

* _A_ 
* _tHit_ 
* _hitKind_ 
* _attributes_ 

--------------------------------------------------------------------------------
# ReportHit<A>

## Signature 

```
```


--------------------------------------------------------------------------------
# IgnoreHit

## Signature 

```
void IgnoreHit();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# AcceptHitAndEndSearch

## Signature 

```
void AcceptHitAndEndSearch();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# DispatchRaysIndex

## Signature 

```
vector<uint,3> DispatchRaysIndex();
```

## Target Availability

CUDA, GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# DispatchRaysDimensions

## Signature 

```
vector<uint,3> DispatchRaysDimensions();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# WorldRayOrigin

## Signature 

```
vector<float,3> WorldRayOrigin();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# WorldRayDirection

## Signature 

```
vector<float,3> WorldRayDirection();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# RayTMin

## Signature 

```
float RayTMin();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# RayTCurrent

## Signature 

```
float RayTCurrent();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# RayFlags

## Signature 

```
uint RayFlags();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# InstanceIndex

## Signature 

```
uint InstanceIndex();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# InstanceID

## Signature 

```
uint InstanceID();
```

## Target Availability

HLSL, __GLSLRAYTRACING


--------------------------------------------------------------------------------
# PrimitiveIndex

## Signature 

```
uint PrimitiveIndex();
```

## Target Availability

HLSL, __GLSLRAYTRACING


--------------------------------------------------------------------------------
# ObjectRayOrigin

## Signature 

```
vector<float,3> ObjectRayOrigin();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# ObjectRayDirection

## Signature 

```
vector<float,3> ObjectRayDirection();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# ObjectToWorld3x4

## Signature 

```
matrix<float,3,4> ObjectToWorld3x4();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# WorldToObject3x4

## Signature 

```
matrix<float,3,4> WorldToObject3x4();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# ObjectToWorld4x3

## Signature 

```
matrix<float,4,3> ObjectToWorld4x3();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# WorldToObject4x3

## Signature 

```
matrix<float,4,3> WorldToObject4x3();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# ObjectToWorld

## Signature 

```
matrix<float,3,4> ObjectToWorld();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# WorldToObject

## Signature 

```
matrix<float,3,4> WorldToObject();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# HitKind

## Signature 

```
uint HitKind();
```

## Target Availability

GL_EXT_RAY_TRACING, GL_NV_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# HIT_KIND_TRIANGLE_FRONT_FACE

```
uint HIT_KIND_TRIANGLE_FRONT_FACE
```


--------------------------------------------------------------------------------
# HIT_KIND_TRIANGLE_BACK_FACE

```
uint HIT_KIND_TRIANGLE_BACK_FACE
```


--------------------------------------------------------------------------------
# dot4add_u8packed

## Signature 

```
uint dot4add_u8packed(
    uint                 left,
    uint                 right,
    uint                 acc);
```

## Target Availability

HLSL

## Parameters

* _left_ 
* _right_ 
* _acc_ 

--------------------------------------------------------------------------------
# dot4add_i8packed

## Signature 

```
int dot4add_i8packed(
    uint                 left,
    uint                 right,
    int                  acc);
```

## Target Availability

HLSL

## Parameters

* _left_ 
* _right_ 
* _acc_ 

--------------------------------------------------------------------------------
# dot2add

## Signature 

```
float dot2add(
    vector<float,2>      left,
    vector<float,2>      right,
    float                acc);
```

## Target Availability

HLSL

## Parameters

* _left_ 
* _right_ 
* _acc_ 

--------------------------------------------------------------------------------
# SetMeshOutputCounts

## Signature 

```
void SetMeshOutputCounts(
    uint                 vertexCount,
    uint                 primitiveCount);
```

## Target Availability

HLSL

## Parameters

* _vertexCount_ 
* _primitiveCount_ 

--------------------------------------------------------------------------------
# DispatchMesh<P>

## Signature 

```
void DispatchMesh<P>(
    uint                 threadGroupCountX,
    uint                 threadGroupCountY,
    uint                 threadGroupCountZ,
    P                    meshPayload);
```

## Target Availability

HLSL

## Parameters

* _P_ 
* _threadGroupCountX_ 
* _threadGroupCountY_ 
* _threadGroupCountZ_ 
* _meshPayload_ 

--------------------------------------------------------------------------------
# struct SAMPLER_FEEDBACK_MIN_MIP

*Implements:* __BuiltinSamplerFeedbackType


--------------------------------------------------------------------------------
# struct SAMPLER_FEEDBACK_MIP_REGION_USED

*Implements:* __BuiltinSamplerFeedbackType


--------------------------------------------------------------------------------
# struct FeedbackTexture2D<T>

## Generic Parameters

* _T_ 

## Methods

* _GetDimensions_ 


--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.GetDimensions

## Signature 

```
void FeedbackTexture2D<T>.GetDimensions(
    out uint             width,
    out uint             height);
void FeedbackTexture2D<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             numberOfLevels);
void FeedbackTexture2D<T>.GetDimensions(
    out float            width,
    out float            height);
void FeedbackTexture2D<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            numberOfLevels);
```

## Target Availability

, HLSL

## Parameters

* _width_ 
* _height_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedback<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedback<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location,
    float                clamp);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 
* _clamp_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedbackBias<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedbackBias<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location,
    float                bias,
    float                clamp);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 
* _bias_ 
* _clamp_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedbackGrad<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedbackGrad<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location,
    vector<float,2>      ddx,
    vector<float,2>      ddy,
    float                clamp);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 
* _ddx_ 
* _ddy_ 
* _clamp_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedbackLevel<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedbackLevel<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location,
    float                lod);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 
* _lod_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedback<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedback<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedbackBias<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedbackBias<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location,
    float                bias);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 
* _bias_ 

--------------------------------------------------------------------------------
# FeedbackTexture2D<T>.WriteSamplerFeedbackGrad<S>

## Signature 

```
void FeedbackTexture2D<T>.WriteSamplerFeedbackGrad<S>(
    Texture2D            tex,
    SamplerState         samp,
    vector<float,2>      location,
    vector<float,2>      ddx,
    vector<float,2>      ddy);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _tex_ 
* _samp_ 
* _location_ 
* _ddx_ 
* _ddy_ 

--------------------------------------------------------------------------------
# struct FeedbackTexture2DArray<T>

## Generic Parameters

* _T_ 

## Methods

* _GetDimensions_ 


--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.GetDimensions

## Signature 

```
void FeedbackTexture2DArray<T>.GetDimensions(
    out uint             width,
    out uint             height,
    out uint             elements);
void FeedbackTexture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out uint             width,
    out uint             height,
    out uint             elements,
    out uint             numberOfLevels);
void FeedbackTexture2DArray<T>.GetDimensions(
    out float            width,
    out float            height,
    out float            elements);
void FeedbackTexture2DArray<T>.GetDimensions(
    uint                 mipLevel,
    out float            width,
    out float            height,
    out float            elements,
    out float            numberOfLevels);
```

## Target Availability

, HLSL

## Parameters

* _width_ 
* _height_ 
* _elements_ 
* _mipLevel_ 
* _numberOfLevels_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedback<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedback<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location,
    float                clamp);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 
* _clamp_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedbackBias<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedbackBias<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location,
    float                bias,
    float                clamp);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 
* _bias_ 
* _clamp_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedbackGrad<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedbackGrad<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location,
    vector<float,3>      ddx,
    vector<float,3>      ddy,
    float                clamp);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 
* _ddx_ 
* _ddy_ 
* _clamp_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedbackLevel<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedbackLevel<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location,
    float                lod);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 
* _lod_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedback<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedback<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedbackBias<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedbackBias<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location,
    float                bias);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 
* _bias_ 

--------------------------------------------------------------------------------
# FeedbackTexture2DArray<T>.WriteSamplerFeedbackGrad<S>

## Signature 

```
void FeedbackTexture2DArray<T>.WriteSamplerFeedbackGrad<S>(
    Texture2DArray       texArray,
    SamplerState         samp,
    vector<float,3>      location,
    vector<float,3>      ddx,
    vector<float,3>      ddy);
```

## Target Availability

CPP, HLSL

## Parameters

* _S_ 
* _texArray_ 
* _samp_ 
* _location_ 
* _ddx_ 
* _ddy_ 

--------------------------------------------------------------------------------
# GeometryIndex

## Signature 

```
uint GeometryIndex();
```

## Target Availability

GL_EXT_RAY_TRACING, HLSL


--------------------------------------------------------------------------------
# COMMITTED_NOTHING

```
uint COMMITTED_NOTHING
```


--------------------------------------------------------------------------------
# COMMITTED_TRIANGLE_HIT

```
uint COMMITTED_TRIANGLE_HIT
```


--------------------------------------------------------------------------------
# COMMITTED_PROCEDURAL_PRIMITIVE_HIT

```
uint COMMITTED_PROCEDURAL_PRIMITIVE_HIT
```


--------------------------------------------------------------------------------
# CANDIDATE_NON_OPAQUE_TRIANGLE

```
uint CANDIDATE_NON_OPAQUE_TRIANGLE
```


--------------------------------------------------------------------------------
# CANDIDATE_PROCEDURAL_PRIMITIVE

```
uint CANDIDATE_PROCEDURAL_PRIMITIVE
```


--------------------------------------------------------------------------------
# struct RayQuery<rayFlags:uint>

## Generic Parameters

* _rayFlags_ 

## Methods

* _CandidateObjectToWorld3x4_ 
* _CommittedObjectRayDirection_ 
* _CommittedInstanceContributionToHitGroupIndex_ 
* _CommittedTriangleFrontFace_ 
* _CandidateType_ 
* _CommittedWorldToObject3x4_ 
* _WorldRayDirection_ 
* _CandidateGeometryIndex_ 
* _CandidateInstanceID_ 
* _CandidateProceduralPrimitiveNonOpaque_ 
* _CandidateWorldToObject3x4_ 
* _init_ 
* _TraceRayInline_ 
* _CommittedGeometryIndex_ 
* _CommittedStatus_ 
* _CommittedRayT_ 
* _CommittedObjectRayOrigin_ 
* _CandidateObjectRayDirection_ 
* _CommittedTriangleBarycentrics_ 
* _CandidateTriangleRayT_ 
* _CommittedObjectToWorld4x3_ 
* _CommittedPrimitiveIndex_ 
* _CandidatePrimitiveIndex_ 
* _CandidateWorldToObject4x3_ 
* _CandidateTriangleFrontFace_ 
* _CommittedInstanceID_ 
* _CommitNonOpaqueTriangleHit_ 
* _RayFlags_ 
* _CandidateObjectToWorld4x3_ 
* _RayTMin_ 
* _CandidateInstanceContributionToHitGroupIndex_ 
* _Proceed_ 
* _CandidateInstanceIndex_ 
* _CandidateTriangleBarycentrics_ 
* _CommittedObjectToWorld3x4_ 
* _CommitProceduralPrimitiveHit_ 
* _Abort_ 
* _CommittedInstanceIndex_ 
* _CommittedWorldToObject4x3_ 
* _CandidateObjectRayOrigin_ 
* _WorldRayOrigin_ 


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.init

## Signature 

```
RayQuery<rayFlags:uint>.init();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.TraceRayInline

## Signature 

```
void RayQuery<rayFlags:uint>.TraceRayInline(
    RaytracingAccelerationStructure accelerationStructure,
    uint                 rayFlags,
    uint                 instanceInclusionMask,
    RayDesc              ray);
```

## Target Availability

HLSL

## Parameters

* _accelerationStructure_ 
* _rayFlags_ 
* _instanceInclusionMask_ 
* _ray_ 

--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.Proceed

## Signature 

```
bool RayQuery<rayFlags:uint>.Proceed();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.Abort

## Signature 

```
void RayQuery<rayFlags:uint>.Abort();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateType

## Signature 

```
uint RayQuery<rayFlags:uint>.CandidateType();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateObjectToWorld3x4

## Signature 

```
matrix<float,3,4> RayQuery<rayFlags:uint>.CandidateObjectToWorld3x4();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateObjectToWorld4x3

## Signature 

```
matrix<float,4,3> RayQuery<rayFlags:uint>.CandidateObjectToWorld4x3();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateWorldToObject3x4

## Signature 

```
matrix<float,3,4> RayQuery<rayFlags:uint>.CandidateWorldToObject3x4();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateWorldToObject4x3

## Signature 

```
matrix<float,4,3> RayQuery<rayFlags:uint>.CandidateWorldToObject4x3();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateInstanceIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CandidateInstanceIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateInstanceID

## Signature 

```
uint RayQuery<rayFlags:uint>.CandidateInstanceID();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateGeometryIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CandidateGeometryIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidatePrimitiveIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CandidatePrimitiveIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateInstanceContributionToHitGroupIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CandidateInstanceContributionToHitGroupIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateObjectRayOrigin

## Signature 

```
vector<float,3> RayQuery<rayFlags:uint>.CandidateObjectRayOrigin();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateObjectRayDirection

## Signature 

```
vector<float,3> RayQuery<rayFlags:uint>.CandidateObjectRayDirection();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateProceduralPrimitiveNonOpaque

## Signature 

```
bool RayQuery<rayFlags:uint>.CandidateProceduralPrimitiveNonOpaque();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateTriangleFrontFace

## Signature 

```
bool RayQuery<rayFlags:uint>.CandidateTriangleFrontFace();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateTriangleBarycentrics

## Signature 

```
vector<float,2> RayQuery<rayFlags:uint>.CandidateTriangleBarycentrics();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CandidateTriangleRayT

## Signature 

```
float RayQuery<rayFlags:uint>.CandidateTriangleRayT();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommitNonOpaqueTriangleHit

## Signature 

```
void RayQuery<rayFlags:uint>.CommitNonOpaqueTriangleHit();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommitProceduralPrimitiveHit

## Signature 

```
void RayQuery<rayFlags:uint>.CommitProceduralPrimitiveHit(float t);
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL

## Parameters

* _t_ 

--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedStatus

## Signature 

```
uint RayQuery<rayFlags:uint>.CommittedStatus();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedObjectToWorld3x4

## Signature 

```
matrix<float,3,4> RayQuery<rayFlags:uint>.CommittedObjectToWorld3x4();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedObjectToWorld4x3

## Signature 

```
matrix<float,4,3> RayQuery<rayFlags:uint>.CommittedObjectToWorld4x3();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedWorldToObject3x4

## Signature 

```
matrix<float,3,4> RayQuery<rayFlags:uint>.CommittedWorldToObject3x4();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedWorldToObject4x3

## Signature 

```
matrix<float,4,3> RayQuery<rayFlags:uint>.CommittedWorldToObject4x3();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedRayT

## Signature 

```
float RayQuery<rayFlags:uint>.CommittedRayT();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedInstanceIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CommittedInstanceIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedInstanceID

## Signature 

```
uint RayQuery<rayFlags:uint>.CommittedInstanceID();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedGeometryIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CommittedGeometryIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedPrimitiveIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CommittedPrimitiveIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedInstanceContributionToHitGroupIndex

## Signature 

```
uint RayQuery<rayFlags:uint>.CommittedInstanceContributionToHitGroupIndex();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedObjectRayOrigin

## Signature 

```
vector<float,3> RayQuery<rayFlags:uint>.CommittedObjectRayOrigin();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedObjectRayDirection

## Signature 

```
vector<float,3> RayQuery<rayFlags:uint>.CommittedObjectRayDirection();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedTriangleFrontFace

## Signature 

```
bool RayQuery<rayFlags:uint>.CommittedTriangleFrontFace();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.CommittedTriangleBarycentrics

## Signature 

```
vector<float,2> RayQuery<rayFlags:uint>.CommittedTriangleBarycentrics();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.RayFlags

## Signature 

```
uint RayQuery<rayFlags:uint>.RayFlags();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.WorldRayOrigin

## Signature 

```
vector<float,3> RayQuery<rayFlags:uint>.WorldRayOrigin();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.WorldRayDirection

## Signature 

```
vector<float,3> RayQuery<rayFlags:uint>.WorldRayDirection();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# RayQuery<rayFlags:uint>.RayTMin

## Signature 

```
float RayQuery<rayFlags:uint>.RayTMin();
```

## Requirements

GLSL GL_EXT_ray_query, GLSL460

## Target Availability

GLSL, HLSL


--------------------------------------------------------------------------------
# struct VkSubpassInput<T>

## Generic Parameters

* _T_ 

## Methods

* _SubpassLoad_ 


--------------------------------------------------------------------------------
# VkSubpassInput<T>.SubpassLoad

## Signature 

```
T VkSubpassInput<T>.SubpassLoad();
```

## Target Availability

HLSL


--------------------------------------------------------------------------------
# struct VkSubpassInputMS<T>

## Generic Parameters

* _T_ 

## Methods

* _SubpassLoad_ 


--------------------------------------------------------------------------------
# VkSubpassInputMS<T>.SubpassLoad

## Signature 

```
T VkSubpassInputMS<T>.SubpassLoad(int sampleIndex);
```

## Target Availability

HLSL

## Parameters

* _sampleIndex_ 
