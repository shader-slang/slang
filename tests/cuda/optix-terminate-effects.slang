// Preserve outgoing payload writes across shader exits before varying legalization.
// A nullary conditional callee can observe the payload without receiving its address.
// Existing optix-terminate-nested and optix-terminate-nested-recursion tests cover
// pointer-mutating helpers, two-level writeback and the recursive E55214 boundary.

//TEST:SIMPLE(filecheck=ACCEPT): -target cuda -entry directAccept
//TEST:SIMPLE(filecheck=IGNORE): -target cuda -entry directIgnore
//TEST:SIMPLE(filecheck=CONDITIONAL_ACCEPT): -target cuda -entry conditionalAccept
//TEST:SIMPLE(filecheck=CONDITIONAL_IGNORE): -target cuda -entry conditionalIgnore
//TEST:SIMPLE(filecheck=ORDINARY): -target cuda -entry ordinaryNames

[shader("anyhit")]
void directAccept(inout uint payload)
{
    payload = 11;
    AcceptHitAndEndSearch();
    payload = 91;
}
// ACCEPT: __anyhit__directAccept
// ACCEPT: 11U
// ACCEPT: optixTerminateRay();
// ACCEPT: 91U

[shader("anyhit")]
void directIgnore(inout uint payload)
{
    payload = 12;
    IgnoreHit();
    payload = 92;
}
// IGNORE: __anyhit__directIgnore
// IGNORE: 12U
// IGNORE: optixIgnoreIntersection();
// IGNORE: 92U

[noinline]
void conditionalExit<let accept : bool>()
{
    if (RayTCurrent() > 0.5)
    {
        if (accept)
            AcceptHitAndEndSearch();
        else
            IgnoreHit();
    }
}

[noinline]
void outerExit<let accept : bool>()
{
    conditionalExit<accept>();
}

[shader("anyhit")]
void conditionalAccept(inout uint payload)
{
    payload = 31;
    outerExit<true>();
    // The helper's normal-return branch must still reach this store.
    payload = 93;
}
// CONDITIONAL_ACCEPT: __anyhit__conditionalAccept
// CONDITIONAL_ACCEPT: if(
// CONDITIONAL_ACCEPT: optixSetPayload_0(31U);
// CONDITIONAL_ACCEPT: optixTerminateRay();
// CONDITIONAL_ACCEPT: }
// CONDITIONAL_ACCEPT: optixSetPayload_0(93U);

[shader("anyhit")]
void conditionalIgnore(inout uint payload)
{
    payload = 32;
    outerExit<false>();
    payload = 94;
}
// CONDITIONAL_IGNORE: __anyhit__conditionalIgnore
// CONDITIONAL_IGNORE: if(
// CONDITIONAL_IGNORE: optixSetPayload_0(32U);
// CONDITIONAL_IGNORE: optixIgnoreIntersection();
// CONDITIONAL_IGNORE: }
// CONDITIONAL_IGNORE: optixSetPayload_0(94U);

namespace Ordinary
{
    [noinline]
    void IgnoreHit(inout uint value)
    {
        value += 7;
    }

    [noinline]
    void AcceptHitAndEndSearch(inout uint value)
    {
        value += 9;
    }
}

[shader("anyhit")]
void ordinaryNames(inout uint payload)
{
    Ordinary::IgnoreHit(payload);
    Ordinary::AcceptHitAndEndSearch(payload);
}
// The same spelling does not make an ordinary function a shader exit. In particular,
// no payload writeback is inserted before either call, and no forced hoisting occurs.
// ORDINARY: __anyhit__ordinaryNames
// ORDINARY-NOT: optixSetPayload_
// ORDINARY: IgnoreHit{{[^;]*}};
// ORDINARY-NOT: optixSetPayload_
// ORDINARY: AcceptHitAndEndSearch{{[^;]*}};
// ORDINARY: optixSetPayload_0
