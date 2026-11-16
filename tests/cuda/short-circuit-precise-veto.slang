// A `[precise]`-qualified instruction in the second operand's chain keeps
// its diamond: hoisting would move a value the user asked to keep precise,
// so the pass declines. The precise qualifier sits on the multiply inside
// the inlined helper (SSA construction clones the decoration onto the stored
// value, and inlining lands it in the value arm), which the hoist gate
// rejects. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[ForceInline]
bool loadAndCheck(uint i)
{
    precise float w = itemMin[i].x * 2.0;
    return w > 0.5;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 2 &&
        loadAndCheck(tid.x))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: &&
}
