//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -specialize <typename> supplies the generic argument for a generic entry point so it compiles to a concrete non-generic entry, where omitting the flag fails with E38014.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::Specialize` arm of `OptionsParser::_parse`
// in slang-options.cpp, which attaches the named type as a specialization
// argument to the most recently declared entry point.
//
// `main` below is declared `void main<T : IFoo>()`, so it cannot be compiled on
// its own. DEFAULT pins that failure — `error[E38014]: generic entry point used
// without specialization arguments`, whose message text itself names
// `-specialize` as the remedy. SPECIALIZED passes `-specialize Impl`, and the
// emitted HLSL then contains `Impl_get_0()` returning `42.0f`, i.e. the
// witness was resolved to `Impl` and inlined into a plain `void main()` with no
// generic machinery left. That call is the tight distinguishing token; the
// generated `_0` suffix is part of it because Slang's name mangling is stable
// for a single non-overloaded method here.

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPECIALIZED):-target hlsl -entry main -stage compute -specialize Impl

interface IFoo
{
    float get();
}

struct Impl : IFoo
{
    float get() { return 42.0; }
}

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main<T : IFoo>()
{
    T t;
    outBuf[0] = t.get();
}

//DEFAULT: error[E38014]: generic entry point used without specialization arguments

//SPECIALIZED: float Impl_get_0()
//SPECIALIZED: return 42.0f;
//SPECIALIZED: void main()
//SPECIALIZED: = Impl_get_0();
