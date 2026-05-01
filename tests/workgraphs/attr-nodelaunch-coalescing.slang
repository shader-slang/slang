//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main

// Test that a coalescing-launch node shader compiles to dxil and spirv.

[shader("node")]
[NodeLaunch("coalescing")]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK_DXIL: define void @main()
//CHK_SPIRV: OpFunction
