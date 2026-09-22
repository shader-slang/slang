// Codegen test for shader-slang/slang#13220: interface dispatch on a C-family target.
//
// Lowering an interface (existential) value builds a witness-tag dispatch `switch` with one `case`
// per concrete conformer plus a `default:` arm for tags outside that closed, statically-enumerable
// conformer set. The switch selector and the case tags are synthesized from the same set, so no
// valid tag selects the default arm. On targets that can express an unreachable terminator the dead
// default arm is that marker (letting the downstream compiler drop the range check); targets that
// cannot keep a defined default. This checks emitted code only, so it needs no GPU.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -line-directive-mode none

interface IHit
{
    uint getValue();
}

struct HitA : IHit
{
    uint v;
    uint getValue() { return v; }
}

struct HitB : IHit
{
    uint v;
    uint getValue() { return v * 2u + 1u; }
}

// The set of concrete types this returns is closed and statically known (HitA, HitB), so the
// dispatcher built for the `hit.getValue()` call below enumerates exactly those two tags.
IHit makeHit(uint type, uint v)
{
    if (type == 0u)
        return HitA(v);
    return HitB(v);
}

RWStructuredBuffer<uint> outputs;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    IHit hit = makeHit(tid.x, tid.y);
    outputs[tid.x] = hit.getValue();
}

// CUDA and C++ can express an unreachable terminator, so the dispatcher's dead default arm is the
// marker (the prelude spells it `__builtin_unreachable()` on CUDA/clang/gcc, `__assume(0)` on MSVC).
//CUDA: default:
//CUDA: SLANG_PRELUDE_UNREACHABLE();
//CPP: default:
//CPP: SLANG_PRELUDE_UNREACHABLE();

// HLSL has no unreachable spelling, so the dispatcher keeps its defined default (a zero-valued
// assignment) and never emits the marker. (`numthreads` anchors the check to the compiled entry.)
//HLSL: numthreads
//HLSL: default:
//HLSL: 0U
//HLSL-NOT: SLANG_PRELUDE_UNREACHABLE
