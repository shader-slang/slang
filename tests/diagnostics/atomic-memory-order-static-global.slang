// An order held in a mutable `static` global that is never assigned after its initializer folds
// to its constant by the time memory orders are validated, at the default optimization and
// debug-info levels, so this compiles on targets that encode the order. The fold is an optimizer
// outcome rather than a language guarantee: with -g2 the order stays a load of the global, and
// SPIR-V and Metal report E41407, as they do for a global that is assigned elsewhere. A
// `static const` global is the guaranteed form (atomic-memory-order-accepted-shapes.slang).

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEBUG):-target spirv -g2
//DIAGNOSTIC_TEST:SIMPLE(diag=DEBUG):-target metal -g2

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;

static MemoryOrder globalOrder = MemoryOrder.Acquire;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = flag[0].load(globalOrder);
//DEBUG:                  ^ E41407
//DEBUG:                  ^ the memory order of an atomic operation must be a compile-time constant when targeting
}

// CHECK: result code = 0
