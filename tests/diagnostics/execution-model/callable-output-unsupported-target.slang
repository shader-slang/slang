// A target may intentionally provide no type-layout rules for a ray-tracing
// entry-point parameter: CUDA has no callable-payload rules, and Metal has no
// ray-payload, callable-payload, or hit-attribute rules at all. Such a
// parameter must produce a diagnostic (E39032), never a null-rules dereference
// during type layout.
//
// CUDA exercises the callable-payload branch (inout/out payload, non-void
// return). Metal additionally exercises the ray-payload and hit-attribute
// branches via a closesthit entry point. `non-exhaustive` because each target
// emits the same diagnostic for several parameters.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-METAL, non-exhaustive): -target metal

//CHECK: E39032
//CHECK: the current compilation target does not support ray tracing entry point parameters for the 'callable' stage
//CHECK-METAL: E39032
//CHECK-METAL: the current compilation target does not support ray tracing entry point parameters for the 'callable' stage
//CHECK-METAL: the current compilation target does not support ray tracing entry point parameters for the 'closesthit' stage

[shader("callable")]
void callableInOut(inout float p)
{
    p = 1.0;
}

[shader("callable")]
void callableOut(out float p)
{
    p = 1.0;
}

[shader("callable")]
float callableReturn()
{
    return 1.0;
}

struct Payload { float3 color; };
struct Attributes { float2 bary; };

// Metal has no ray-payload or hit-attribute rules either, so the payload and
// attribute parameters of a closesthit entry point must diagnose rather than
// crash. CUDA supports both, so this entry point only diagnoses for Metal.
[shader("closesthit")]
void closestHit(inout Payload p, in Attributes a)
{
    p.color = float3(a.bary, 0.0);
}
