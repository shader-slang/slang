// Appending a struct whose matrix is stored differently from its logical form must pack the value
// into the element type of the `AppendStructuredBuffer` (shader-slang/slang#13385).
//
// HLSL is the target that keeps `Append` through buffer element lowering, so the struct reaches
// the `StructuredBufferAppend` instruction still in its logical form.

//TEST:SIMPLE(filecheck=LOCAL): -target hlsl -entry main -stage compute -DFROM_LOCAL
//TEST:SIMPLE(filecheck=LOCAL): -target hlsl -entry main -stage compute -DFROM_LOCAL -matrix-layout-column-major
//TEST:SIMPLE(filecheck=LOCAL): -target hlsl -entry main -stage compute -DFROM_LOCAL -DCOLUMN_MAJOR -matrix-layout-row-major
//TEST:SIMPLE(filecheck=CBUFFER): -target hlsl -entry main -stage compute -DFROM_CBUFFER
//TEST:SIMPLE(filecheck=BUFFER): -target hlsl -entry main -stage compute -DFROM_STRUCTURED_BUFFER
//TEST:SIMPLE(filecheck=BUFFER): -target hlsl -entry main -stage compute -DFROM_CONSUME
//TEST:SIMPLE(filecheck=NESTED): -target hlsl -entry main -stage compute -DNESTED

//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -stage compute -profile cs_6_0 -DFROM_LOCAL
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -stage compute -profile cs_6_0 -DFROM_CBUFFER
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -stage compute -profile cs_6_0 -DFROM_STRUCTURED_BUFFER
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -stage compute -profile cs_6_0 -DFROM_CONSUME
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -stage compute -profile cs_6_0 -DNESTED

#if defined(NESTED)
struct I { row_major float3x2 m; float x; };
struct S { I i[2]; int k; };
#elif defined(COLUMN_MAJOR)
struct S { column_major float2x2 m; };
#else
struct S { row_major float2x2 m; };
#endif

#if defined(FROM_CBUFFER)
cbuffer C { S s; }
#elif defined(FROM_STRUCTURED_BUFFER)
StructuredBuffer<S> src;
#elif defined(FROM_CONSUME)
ConsumeStructuredBuffer<S> src;
#endif

AppendStructuredBuffer<S> a;

// The appended value is a temporary of the append buffer's element type with the matrix packed
// into it, except when the source already has that type.
// LOCAL: _S{{[0-9]+}}.m_0.data_{{[0-9]+}} =
// LOCAL: a_0.Append(_S{{[0-9]+}});

// A `cbuffer` element is lowered to a different storage type than the append buffer element, so
// the matrix is unpacked from the former and packed into a temporary of the latter.
// CBUFFER: AppendStructuredBuffer<[[T:[A-Za-z_0-9]+]] > a_0
// CBUFFER: = float2x2(C_0.s_0.m_0.data_{{[0-9]+}}
// CBUFFER: [[T]] _S{{[0-9]+}};
// CBUFFER: _S{{[0-9]+}}.m_0.data_{{[0-9]+}} =
// CBUFFER: a_0.Append(_S{{[0-9]+}});

// A structured or consume buffer element has the same storage type, so the value is copied with no
// unpacking or packing.
// BUFFER: void main()
// BUFFER-NOT: float2x2(
// BUFFER: a_0.Append(

// NESTED: _S{{[0-9]+}}.i_0.data_{{[0-9]+}}[int(0)].m_0.data_{{[0-9]+}} =
// NESTED: _S{{[0-9]+}}.i_0.data_{{[0-9]+}}[int(1)].m_0.data_{{[0-9]+}} =
// NESTED: a_0.Append(_S{{[0-9]+}});

// DXIL: define void @main()
// DXIL: @dx.op.bufferUpdateCounter
[numthreads(1, 1, 1)]
void main()
{
#if defined(FROM_CBUFFER)
    a.Append(s);
#elif defined(FROM_STRUCTURED_BUFFER)
    a.Append(src[0]);
#elif defined(FROM_CONSUME)
    a.Append(src.Consume());
#elif defined(NESTED)
    S t = (S)0;
    t.i[1].m = float3x2(1, 2, 3, 4, 5, 6);
    a.Append(t);
#else
    S t;
    t.m = float2x2(1, 2, 3, 4);
    a.Append(t);
#endif
}
