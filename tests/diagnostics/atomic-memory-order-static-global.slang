// An order held in a mutable `static` global that is only initialized folds to its constant by
// the time memory orders are validated, so this compiles on targets that encode the order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;

static MemoryOrder globalOrder = MemoryOrder.Acquire;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = flag[0].load(globalOrder);
}

// CHECK: result code = 0
