// Codegen + execution test for shader-slang/slang#13220: interface dispatch on a C-family target.
//
// Lowering an interface (existential) value builds a witness-tag dispatch `switch` with one `case`
// per concrete conformer plus a `default:` arm for tags outside that closed, statically-enumerable
// conformer set. The switch selector and the case tags are synthesized from the same set, so no
// valid tag selects the default arm. On targets that can express an unreachable terminator the dead
// default arm is that marker (letting the downstream compiler drop the range check); targets that
// cannot keep a defined default.
//
// The SIMPLE variants check the emitted code (GPU-free). The COMPARE_COMPUTE variant additionally
// compiles the emitted C++ *through the host compiler* (exercising the new SLANG_PRELUDE_UNREACHABLE
// macro) and executes closed-set dispatch on the CPU, confirming the never-taken default arm leaves
// runtime behavior unchanged; it also drives a CUDA (nvrtc) syntax compile of the marker. The
// dispatched type is read from a buffer so the dispatcher is not resolved away at compile time.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=EXEC): -shaderobj -output-using-type -cpu

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

// The dispatch type is read from a buffer at runtime, so the existential is not resolved at compile
// time and the dispatcher (with its default arm) is actually built.
//TEST_INPUT: set types = ubuffer(data=[0 1 0 1], stride=4)
StructuredBuffer<uint> types;
//TEST_INPUT: set outputs = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputs;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0u; i < 4u; i++)
        outputs[i] = makeHit(types[i], i + 5u).getValue();

    // types = [0, 1, 0, 1]; v = i + 5 = 5, 6, 7, 8
    // i=0: HitA(5)          -> 5
    // i=1: HitB(6) = 6*2+1  -> 13
    // i=2: HitA(7)          -> 7
    // i=3: HitB(8) = 8*2+1  -> 17
    //EXEC: 5
    //EXEC: 13
    //EXEC: 7
    //EXEC: 17
}

// CUDA and C++ can express an unreachable terminator, so the dispatcher's dead default arm is the
// marker (the prelude spells it `__builtin_unreachable()` on CUDA/clang/gcc, `__assume(0)` on MSVC,
// and `abort()` on any other host compiler).
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
