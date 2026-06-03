//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that a thread-launch node shader compiles to dxil.

[shader("node")]
[NodeLaunch("thread")]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK_DXIL: define void @main()
