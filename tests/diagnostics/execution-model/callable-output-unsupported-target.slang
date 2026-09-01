// A target may intentionally provide no type-layout rules for a ray-tracing
// entry-point parameter. Metal has no ray-payload, callable-payload, or
// hit-attribute rules, so these parameters must produce a diagnostic (E39032),
// never a null-rules dereference during type layout.
//
// The callable entry points exercise the callable-payload branch (inout/out
// payload and non-void return). The closesthit entry point additionally
// exercises the ray-payload and hit-attribute branches. `non-exhaustive`
// because Metal emits the same diagnostic for several parameters.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target metal

//CHECK: E39032
//CHECK: the current compilation target does not support ray tracing entry point parameters for the 'callable' stage
//CHECK: the current compilation target does not support ray tracing entry point parameters for the 'closesthit' stage

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

// Metal has no ray-payload or hit-attribute rules, so the payload and attribute
// parameters of a closesthit entry point must diagnose rather than crash.
[shader("closesthit")]
void closestHit(inout Payload p, in Attributes a)
{
    p.color = float3(a.bary, 0.0);
}
