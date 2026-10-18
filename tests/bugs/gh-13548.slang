// Regression test for #13548: a value read before a store must not be re-read after it when
// it is folded into `NonUniformResourceIndex` in textual targets.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST:SIMPLE(filecheck=SRC): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target glsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[1 0 0], stride=4):out,name=buf
RWStructuredBuffer<uint> buf;

// `buf[0]` is the loop count and `buf[1]` selects the branch, so that neither the loop nor the
// branch is folded away.
void swap(out uint ra, out uint rb)
{
    uint a = 0, b = 1;
    for (uint k = 0; k < buf[0]; ++k)
    {
        uint oldA = a;
        a = b;
        b = oldA;
        if (buf[1] == 0)
        {
            // SRC: [[B:_S[0-9]+]] = b_0;
            // SRC: [[A:_S[0-9]+]] = a_0;
            // SRC: a_0 = {{NonUniformResourceIndex|nonuniformEXT}}([[B]]);
            // SRC: b_0 = {{NonUniformResourceIndex|nonuniformEXT}}([[A]]);
            a = NonUniformResourceIndex(a);
            b = NonUniformResourceIndex(b);
            break;
        }
    }
    ra = a;
    rb = b;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint a, b;
    swap(a, b);
    buf[1] = a;
    buf[2] = b;
}

// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
