// Positive test: verify the stage-scoped varying-type rules do NOT fire on
// stages where the reporter (issues #9446, #9448, #9449, #9450, #9452) saw
// no failure, namely compute/ray-tracing-payload-style stages.
//
// Rejecting any of these would be over-rejection: the user instruction is
// "never reject a potentially valid shader". If the downstream backend has
// its own issue with one of these shapes, that is its responsibility to
// diagnose.

//TEST:SIMPLE(filecheck=COMPUTE): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MISS): -target spirv -entry missMain -stage miss
//TEST:SIMPLE(filecheck=CALLABLE): -target spirv -entry callableMain -stage callable

import linalg;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Valid internal use of matrices with out-of-range dims and exotic element
    // types is left to the downstream backend; the frontend does not reject.
    matrix<float, 5, 5> big;
    matrix<uint64_t, 2, 2> bigElem;
    vector<bool, 4> bvec;
}

// COMPUTE-NOT: E38050
// COMPUTE-NOT: E38051
// COMPUTE-NOT: not a valid varying type
// COMPUTE-NOT: not a valid SPIR-V varying type
// COMPUTE-NOT: matrix row and column counts

// Miss shader: takes a "payload" struct parameter by inout. Issue #9452
// explicitly notes vector<bool> is accepted by the miss stage.
struct Payload
{
    vector<bool, 4> flags;
    matrix<uint64_t, 2, 2> m;
}

[shader("miss")]
void missMain(inout Payload p)
{
}

// MISS-NOT: E38050
// MISS-NOT: E38051
// MISS-NOT: not a valid varying type
// MISS-NOT: not a valid SPIR-V varying type
// MISS-NOT: matrix row and column counts

// Callable shader: also accepted per issue #9452.
struct CallData
{
    CoopVec<float, 4> v;
}

[shader("callable")]
void callableMain(inout CallData d)
{
}

// CALLABLE-NOT: E38050
// CALLABLE-NOT: E38051
// CALLABLE-NOT: not a valid varying type for this stage
