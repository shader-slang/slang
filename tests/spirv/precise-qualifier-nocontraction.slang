// Regression test for issue #12198: on the direct SPIR-V backend the per-variable
// `precise` qualifier must decorate every floating-point op contributing to a
// precise value with `NoContraction` -- including compiler-generated intermediates
// (a sub-expression temporary, or a value that crosses control flow) that carry no
// `IRPreciseDecoration` of their own -- even in the default floating-point mode, and
// it must NOT decorate ops outside that precise cone (fast math is preserved
// elsewhere). This is distinct from #11933, which covered the global `-fp-mode`.

//TEST:SIMPLE(filecheck=STRAIGHT): -target spirv-asm -entry straightLine -stage compute
//TEST:SIMPLE(filecheck=MIXED): -target spirv-asm -entry mixed -stage compute
//TEST:SIMPLE(filecheck=BRANCH): -target spirv-asm -entry branched -stage compute

StructuredBuffer<float3> inputBuffer;
StructuredBuffer<float> scalarBuffer;
RWStructuredBuffer<float> outputBuffer;

float area(float3 lo, float3 hi)
{
    precise float dx = hi.x - lo.x;
    precise float dy = hi.y - lo.y;
    precise float dz = hi.z - lo.z;
    precise float axy = dx * dy;
    precise float ayz = dy * dz;
    precise float azx = dz * dx;
    // The sum lowers to two adds; only the outer add carries the qualifier, but the
    // inner (`axy + ayz`) temporary must be decorated too or spirv-opt reassociates.
    precise float s = axy + ayz + azx;
    return s;
}

[numthreads(1, 1, 1)]
void straightLine(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = area(inputBuffer[0], inputBuffer[1]);
}

// The precise cone (and its contributing intermediates) must carry NoContraction,
// and source order must be preserved: the three source multiplies (dx*dy, dy*dz,
// dz*dx) all survive. The bug reassociated into `dy*(dx+dz) + dz*dx`, which has only
// two multiplies, so requiring three OpFMul is the crisp discriminator.
//STRAIGHT: OpDecorate %{{.*}} NoContraction
//STRAIGHT: OpFMul %float
//STRAIGHT: OpFMul %float
//STRAIGHT: OpFMul %float
//STRAIGHT: OpFAdd %float
//STRAIGHT: OpFAdd %float

[numthreads(1, 1, 1)]
void mixed(uint3 tid: SV_DispatchThreadID)
{
    float a = scalarBuffer[0];
    float b = scalarBuffer[1];
    float c = scalarBuffer[2];
    float d = scalarBuffer[3];
    // Not precise (uses a, b): must stay contraction-eligible (no NoContraction).
    float fast = a * b + a;
    // Precise (uses c, d -- disjoint from `fast`, so no CSE merges the cones): both its
    // multiply and add must be decorated, the multiply being an undecorated intermediate.
    precise float slow = c * d + c;
    outputBuffer[tid.x] = fast + slow;
}

// Exactly the two ops of the precise cone (the `c*d` multiply and the `+c` add) are
// decorated; the three non-precise ops (`a*b`, `+a`, and the final `fast + slow`) are
// not. Pinning the count to two rejects both over-decoration (fast math lost elsewhere)
// and a decorate-only-the-qualified-inst fix that would miss the multiply intermediate.
//MIXED-COUNT-2: OpDecorate %{{[A-Za-z0-9_]+}} NoContraction
//MIXED-NOT: OpDecorate %{{.*}} NoContraction

[numthreads(1, 1, 1)]
void branched(uint3 tid: SV_DispatchThreadID)
{
    float a = scalarBuffer[0];
    float b = scalarBuffer[1];
    float c = scalarBuffer[2];
    float d = scalarBuffer[3];
    // The precise value flows out of a conditional, so its initializer arithmetic
    // reaches the qualified block parameter through a phi rather than as an operand.
    precise float s = (tid.x > 0) ? (a * b + c * d) : 0.0f;
    outputBuffer[tid.x] = s;
}

// Propagation reaches the arithmetic behind the phi: both source multiplies are
// decorated even though neither carries the qualifier directly.
//BRANCH: OpDecorate %{{.*}} NoContraction
//BRANCH: OpFMul %float
//BRANCH: OpFMul %float
//BRANCH: OpFAdd %float
