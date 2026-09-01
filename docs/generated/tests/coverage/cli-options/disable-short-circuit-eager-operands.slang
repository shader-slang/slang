//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -disable-short-circuit makes && evaluate both operands up front and emit a single `if(a && b)`, where the default lowers the same expression into a branch that guards the right-hand call.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::DisableShortCircuit` arm of
// `OptionsParser::_parse` in slang-options.cpp, which adds
// `OptionKind::DisableShortCircuit` to the linkage option set.
//
// Consider the shader below. `sideEffect` writes to a buffer, so whether the
// right-hand operand of `&&` runs is directly visible in the generated HLSL. By
// default Slang lowers `sideEffect(0) && sideEffect(1)` into a temporary plus an
// `if(_S1) { ... } else { _S2 = false; }` region, so the second call only happens
// when the first returned true. With `-disable-short-circuit` both calls are
// emitted unconditionally as `bool _S1 = ...; bool _S2 = ...;` and the test
// becomes a plain `if(_S1 && _S2)`.
//
// DEFAULT pins the guarded shape (the `else` arm that assigns `false`), EAGER
// pins the fused `if(_S1 && _S2)`; each CHECK token was copied verbatim from a
// real slangc run. Generated names are wildcarded because they are incidental.

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=EAGER):-target hlsl -entry main -stage compute -disable-short-circuit

RWStructuredBuffer<int> b;

bool sideEffect(int i)
{
    b[i] = 1;
    return true;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    if (sideEffect(0) && sideEffect(1))
        b[2] = 3;
}

//DEFAULT: bool {{[A-Za-z_0-9]+}} = sideEffect_0(int(0));
//DEFAULT: if({{[A-Za-z_0-9]+}})
//DEFAULT: sideEffect_0(int(1));
//DEFAULT: else
//DEFAULT: = false;

//EAGER: bool {{[A-Za-z_0-9]+}} = sideEffect_0(int(0));
//EAGER: bool {{[A-Za-z_0-9]+}} = sideEffect_0(int(1));
//EAGER: if({{[A-Za-z_0-9]+}} && {{[A-Za-z_0-9]+}})
