// Copying a whole struct out of a buffer must not leave a `CopyLogical` for the emitter when the
// struct is stored differently from its logical form (shader-slang/slang#13379).
//
// The `LOAD_*` cases copy the struct into a local variable. The `STORE_*` cases write a struct read
// from a `cbuffer` or `ParameterBlock` to a structured buffer whose element is lowered under a
// different layout.

//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_CBUFFER
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_CBUFFER -matrix-layout-column-major
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_CBUFFER -matrix-layout-row-major
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_CBUFFER -DPLAIN_MATRIX
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_CONSTANT_BUFFER
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_CBUFFER -DDIRECT_MEMBER_READ
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_STRUCTURED_BUFFER
//TEST:SIMPLE(filecheck=LOAD): -target glsl -entry main -stage compute -DLOAD_STRUCTURED_BUFFER -DCOLUMN_MAJOR_2X3 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage compute -DLOAD_CBUFFER
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage compute -emit-spirv-via-glsl -DLOAD_CBUFFER

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute -DSTORE
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute -DSTORE
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute -DSTORE
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage compute -emit-spirv-via-glsl -DSTORE

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute -DSTORE -DPLAIN_MATRIX
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage compute -DSTORE -DPLAIN_MATRIX
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry main -stage compute -DSTORE -DPLAIN_MATRIX
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry main -stage compute -DSTORE -DPLAIN_MATRIX

//TEST:SIMPLE(filecheck=WGSL_SCALAR): -target wgsl -entry main -stage compute -DSTORE -DSCALAR
//TEST:SIMPLE(filecheck=WGSL_ARRAY): -target wgsl -entry main -stage compute -DSTORE -DVECTOR_AND_ARRAY

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute -DSTORE -DSTORE_DIRECT
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute -DSTORE -DSTORE_DIRECT
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute -DSTORE -DSTORE_DIRECT

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute -DSTORE -DFROM_PARAMETER_BLOCK
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute -DSTORE -DFROM_PARAMETER_BLOCK
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute -DSTORE -DFROM_PARAMETER_BLOCK

// HLSL keeps `Append` through buffer element lowering and is not covered here.
//TEST:SIMPLE(filecheck=APPEND): -target glsl -entry main -stage compute -DSTORE_APPEND
//TEST:SIMPLE(filecheck=APPEND): -target wgsl -entry main -stage compute -DSTORE_APPEND
//TEST:SIMPLE(filecheck=APPEND): -target metal -entry main -stage compute -DSTORE_APPEND -DPLAIN_MATRIX
//TEST:SIMPLE(filecheck=APPEND): -target cuda -entry main -stage compute -DSTORE_APPEND -DPLAIN_MATRIX

#if defined(COLUMN_MAJOR_2X3)
struct S { column_major float2x3 m; };
#elif defined(PLAIN_MATRIX)
struct S { float2x2 m; };
#elif defined(SCALAR)
struct S { float x; };
#elif defined(VECTOR_AND_ARRAY)
struct S { float3 v; float a[2]; };
#else
struct S { row_major float2x2 m; };
#endif

#if defined(LOAD_CBUFFER)
cbuffer C { S s; }
RWStructuredBuffer<float> o;
#elif defined(LOAD_CONSTANT_BUFFER)
ConstantBuffer<S> s;
RWStructuredBuffer<float> o;
#elif defined(LOAD_STRUCTURED_BUFFER)
StructuredBuffer<S> s;
RWStructuredBuffer<float> o;
#elif defined(STORE_APPEND)
cbuffer C { S s; }
AppendStructuredBuffer<S> b;
#elif defined(FROM_PARAMETER_BLOCK)
struct P { S s; };
struct W { S s; float f; };
ParameterBlock<P> p;
RWStructuredBuffer<W> b;
#else
cbuffer C { S s; }
RWStructuredBuffer<S> b;
#endif

// LOAD: o_0._data[uint(0)] = {{.*}}m_0
// SPIRV: OpEntryPoint

// The matrix is unpacked from the source layout and packed into the destination layout.
// GLSL: b_0._data[uint(0)].{{.*}}m_0.data_{{[0-9]+}} =
// HLSL: b_0[int(0)].{{.*}}m_0.data_{{[0-9]+}} =
// WGSL: b_0[i32(0)].{{.*}}m_0.data_{{[0-9]+}} =
// METAL: (b_1+int(0))->m_0)->data_{{[0-9]+}} =
// CUDA: globalParams_0->b_0)[int(0)])->m_0)->data_{{[0-9]+}} =
// CPP: ->b_0)[int(0)])->m_0)->data_{{[0-9]+}} =
// WGSL_SCALAR: b_0[i32(0)].x_0 = C_0.s_0.x_0;
// The array member of this case hits shader-slang/slang#13380 on WGSL and is not checked here.
// WGSL_ARRAY: b_0[i32(0)].v_0 = C_0.s_0.v_0;
// APPEND: AppendStructuredBuffer_Append_0(
[numthreads(1, 1, 1)]
void main()
{
#if defined(DIRECT_MEMBER_READ)
    o[0] = s.m[0][0];
#elif defined(LOAD_STRUCTURED_BUFFER)
    S t = s[0];
    o[0] = t.m[0][0];
#elif defined(LOAD_CBUFFER) || defined(LOAD_CONSTANT_BUFFER)
    S t = s;
    o[0] = t.m[0][0];
#elif defined(STORE_DIRECT)
    b[0] = s;
#elif defined(FROM_PARAMETER_BLOCK)
    S t = p.s;
    b[0].s = t;
#elif defined(STORE_APPEND)
    S t = s;
    b.Append(t);
#else
    S t = s;
    b[0] = t;
#endif
}
