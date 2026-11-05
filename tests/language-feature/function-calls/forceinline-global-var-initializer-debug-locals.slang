// A force-inlined helper called from a `static` global variable initializer keeps the debug
// information of its locals. The initializer block is moved into the entry point that reads the
// variable, so the inlined locals belong to a function there, unlike a `static const` initializer
// that is inlined at module scope.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute -g3
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute -g2

uniform int divisor;
RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int doubledPlusOne(int a)
{
    int doubledValue = a * 2;
    return doubledValue + 1;
}

static int gGlobal = doubledPlusOne(divisor);

struct Holder
{
    static int gMember = doubledPlusOne(divisor + 1);
}

// SPV: [[NAME:%[0-9]+]] = OpString "doubledValue"
// SPV: DebugLocalVariable [[NAME]]

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = gGlobal + Holder.gMember;
}
