// optix-terminate-nested.slang
//
// Regression test for shader-slang/slang#11658.
//
// When a shader-terminating ray intrinsic (AcceptHitAndEndSearch -> optixTerminateRay,
// IgnoreHit -> optixIgnoreIntersection) is called from a *nested* user function (not
// [ForceInline]) of an `anyhit` entry point, the generated OptiX code must still write the
// payload back BEFORE the ray terminates. Before the fix the write-back was stranded at the
// (dead) entry-point epilogue, so the terminating call ran first and the payload mutation was
// silently lost.
//
// FileCheck asserts the ordering: optixSetPayload_N appears before the terminating intrinsic in
// the emitted entry-point kernel. (CUDA-source emit only; no GPU needed.)

//TEST:SIMPLE(filecheck=ACCEPT): -target cuda -entry anyHitAccept
//TEST:SIMPLE(filecheck=IGNORE): -target cuda -entry anyHitIgnore
//TEST:SIMPLE(filecheck=TWOLEVEL): -target cuda -entry anyHitTwoLevel
//TEST:SIMPLE(filecheck=COND): -target cuda -entry anyHitConditional

struct Payload
{
    uint hit;
}

// Plain (non-[ForceInline]) helpers that mutate the payload then terminate the ray.
void nestedAccept(inout Payload p)
{
    p.hit = 1;
    AcceptHitAndEndSearch();
}

void nestedIgnore(inout Payload p)
{
    p.hit = 2;
    IgnoreHit();
}

// Two-level nesting (entry -> outer -> inner -> terminate) exercises the transitive inlining of
// the fix: the terminating call is two calls deep from the entry point.
void innerTerminate(inout Payload p)
{
    p.hit = 3;
    AcceptHitAndEndSearch();
}

void outerHelper(inout Payload p)
{
    p.hit = 4;
    innerTerminate(p);
}

// Conditional terminate inside a nested callee: the write-back must be placed before the
// terminating call within the branch, not only at the entry-point epilogue.
void conditionalTerminate(inout Payload p, float t)
{
    p.hit = 5;
    if (t < 0.5)
        AcceptHitAndEndSearch();
}

[shader("anyhit")]
void anyHitAccept(inout Payload payload)
{
    nestedAccept(payload);
}
// ACCEPT: optixSetPayload_0
// ACCEPT: optixTerminateRay

[shader("anyhit")]
void anyHitIgnore(inout Payload payload)
{
    nestedIgnore(payload);
}
// IGNORE: optixSetPayload_0
// IGNORE: optixIgnoreIntersection

[shader("anyhit")]
void anyHitTwoLevel(inout Payload payload)
{
    outerHelper(payload);
}
// TWOLEVEL: optixSetPayload_0
// TWOLEVEL: optixTerminateRay

[shader("anyhit")]
void anyHitConditional(inout Payload payload)
{
    conditionalTerminate(payload, RayTCurrent());
}
// COND: optixSetPayload_0
// COND: optixTerminateRay
