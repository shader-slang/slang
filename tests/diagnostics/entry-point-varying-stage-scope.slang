// Positive test: verify the stage-scoped varying-type rules do NOT fire on
// stages where the reporter (issues #9446, #9448, #9449, #9450, #9452) saw
// no failure, namely compute/ray-tracing-payload-style stages and
// amplification (task) shaders, whose outputs use `TaskPayloadWorkgroupEXT`
// rather than an interface block.
//
// Rejecting any of these would be over-rejection: the user instruction is
// "never reject a potentially valid shader". If the downstream backend has
// its own issue with one of these shapes, that is its responsibility to
// diagnose.

//TEST:SIMPLE(filecheck=COMPUTE): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MISS): -target spirv -entry missMain -stage miss
//TEST:SIMPLE(filecheck=TASK): -target spirv -entry taskMain -stage amplification

import linalg;

// --- Compute: any banned varying shape is fine as an internal variable,
// and an `Atomic<int>` parameter is auto-classified uniform (no error).
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(Atomic<int> counter)
{
    // Internal declarations that would be rejected at a varying boundary.
    matrix<float, 5, 5> big;
    vector<bool, 4> bvec;
    CoopVec<float, 4> cvec;
}

// COMPUTE-NOT: E38050
// COMPUTE-NOT: E38051
// COMPUTE-NOT: not a valid varying type
// COMPUTE-NOT: not a valid SPIR-V varying type
// COMPUTE-NOT: matrix row and column counts

// --- Miss: payload carries vector<bool>, which issue #9452 notes is
// accepted by the miss stage despite failing on interface-block stages.
struct Payload
{
    float x;
    vector<bool, 4> flags;
}

[shader("miss")]
void missMain(inout Payload p)
{
}

// MISS-NOT: E38050
// MISS-NOT: E38051
// MISS-NOT: not a valid varying type
// MISS-NOT: not a valid SPIR-V varying type

// --- Amplification: task shaders ship their payload via
// `TaskPayloadWorkgroupEXT`, not an interface block, so the interface-block
// rules must not fire here either.
struct TaskPayload
{
    float x;
    vector<bool, 4> flags;
    matrix<float, 5, 5> oversized;
}

[shader("amplification")]
[numthreads(1, 1, 1)]
void taskMain()
{
    TaskPayload p;
    p.x = 0;
    DispatchMesh(1, 1, 1, p);
}

// TASK-NOT: E38050
// TASK-NOT: E38051
// TASK-NOT: not a valid varying type
// TASK-NOT: not a valid SPIR-V varying type
// TASK-NOT: matrix row and column counts
