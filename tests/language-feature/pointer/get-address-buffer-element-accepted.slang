//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Which mutable structured-buffer subscripts `__getAddress` accepts (shader-slang/slang#12581):
// RWStructuredBuffer, RasterizerOrderedStructuredBuffer, a member of such an element, and
// buffers with an explicit layout argument. Every accepted form yields a pointer with the
// default layout, matching `&buf[i]`.

// A front-end rejection aborts before any SPIR-V is emitted, so forbid a diagnostic first,
// then confirm emission reached the entry point. (`error:` — not bare `error` — avoids matching
// the test harness's `standard error = {` wrapper line.)
//CHECK-NOT: error:
//CHECK: OpEntryPoint

struct Elem
{
    int a;
    int b;
}

RWStructuredBuffer<int> rwBuffer;
RasterizerOrderedStructuredBuffer<int> rovBuffer;
RWStructuredBuffer<Elem> rwStructBuffer;
RWStructuredBuffer<int, Std140DataLayout> rwBuffer140;
RWStructuredBuffer<int, Std430DataLayout> rwBuffer430;

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    int* p1 = __getAddress(rwBuffer[id.x]);
    int* p2 = __getAddress(rovBuffer[id.x]);

    // A member of a mutable-buffer element chains through the element.
    int* p3 = __getAddress(rwStructBuffer[id.x].b);

    // Regardless of the buffer's layout argument, the pointer has the default layout,
    // so it assigns into a plain `int*`.
    int* p4 = __getAddress(rwBuffer140[id.x]);
    int* p5 = __getAddress(rwBuffer430[id.x]);

    output[0] = *p1 + *p2 + *p3 + *p4 + *p5;
}
