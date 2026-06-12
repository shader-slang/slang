//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8

// Test that a thread-launch node shader compiles to dxil without [numthreads].

[shader("node")]
[NodeLaunch("thread")]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK_DXIL: define void @main()
//CHK_HLSL: [NodeLaunch("thread")]
//CHK_HLSL-NOT: numthreads
