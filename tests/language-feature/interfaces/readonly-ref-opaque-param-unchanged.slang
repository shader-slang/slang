// `readonly` on a `__ref` parameter whose type is an image or buffer stays the GLSL memory
// qualifier in either keyword order, without changing the reference passing mode.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -dump-ir -o -

// CHECK-LABEL: func %readFirst
// CHECK: [MemoryQualifierSetDecoration(2 : Int)]
// CHECK-NEXT: param %{{[^ ]*}}{{.*}}: RefParam(TextureType({{.*}}), 0 : UInt64
[noinline]
float readFirst(__ref readonly RWTexture2D<float> t)
{
    return t[uint2(0, 0)];
}

// CHECK-LABEL: func %readSecond
// CHECK: [MemoryQualifierSetDecoration(2 : Int)]
// CHECK-NEXT: param %{{[^ ]*}}{{.*}}: RefParam(TextureType({{.*}}), 0 : UInt64
[noinline]
float readSecond(readonly __ref RWTexture2D<float> t)
{
    return t[uint2(1, 0)];
}

// A repeated `readonly` is still one memory qualifier, and the reference stays read-write.
// CHECK-LABEL: func %readTwice
// CHECK: [MemoryQualifierSetDecoration(2 : Int)]
// CHECK-NEXT: param %{{[^ ]*}}{{.*}}: RefParam(TextureType({{.*}}), 0 : UInt64
[noinline]
float readTwice(readonly readonly __ref RWTexture2D<float> t)
{
    return t[uint2(0, 1)];
}

RWTexture2D<float> gTex;
RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main()
{
    RWTexture2D<float> t = gTex;
    gOut[0] = readFirst(t) + readSecond(t) + readTwice(t);
}
