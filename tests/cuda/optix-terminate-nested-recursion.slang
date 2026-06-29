// optix-terminate-nested-recursion.slang
//
// Companion diagnostic test for shader-slang/slang#11658.
//
// When a shader-terminating ray intrinsic (AcceptHitAndEndSearch / IgnoreHit) is reachable from an
// `anyhit` entry point only through a *recursive* callee, the CUDA legalizer's pre-pass cannot
// flatten the call by inlining (a recursive chain would not terminate). Rather than silently
// dropping the payload write-back, it emits E55214 with actionable guidance.
//
// Such recursion is normally rejected earlier by E55201 ("recursion not allowed"), so this test
// passes -disable-non-essential-validations to let the recursive chain reach the CUDA legalizer
// and exercise the residual-diagnostic path. (Diagnostic only; no GPU needed.)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry anyHitRecursive -disable-non-essential-validations

struct Payload
{
    uint hit;
}

// Recursive (non-[ForceInline]) callee that terminates the ray. The terminate-reaching subgraph
// has a cycle, so the pre-pass cannot inline it into the entry point.
void recurse(inout Payload p, int depth)
{
    p.hit = 1;
    if (depth > 0)
        recurse(p, depth - 1);
    else
        AcceptHitAndEndSearch();
}

[shader("anyhit")]
void anyHitRecursive(inout Payload payload, float t)
{
    recurse(payload, (int)t);
//CHECK:   ^ shader-terminating intrinsic in non-inlinable callee
//CHECK:   ^ a shader-terminating intrinsic ('IgnoreHit' or 'AcceptHitAndEndSearch') is reachable from this ray entry point only through a call that could not be inlined (for example, recursion); mark the intervening function(s) '[ForceInline]' or call the intrinsic directly in the entry point so the ray payload is written back before the ray terminates.
}
