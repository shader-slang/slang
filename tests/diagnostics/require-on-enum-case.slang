// Regression test for https://github.com/shader-slang/slang/issues/14664.
//
// [require(...)] on an individual enum case must parse, and a consumer of
// the case must inherit that case's per-target capability requirements,
// mirroring whole-enum [require] behavior. E36107 is the entry-point error
// for capability requirements that conflict with the compilation target;
// E36100 is the use-site error for two conflicting requirements joined
// inside one consumer.
//
// The first invocation consumes the spirv-gated case for `-target hlsl` and
// must fail with E36107. The second consumes the hlsl-gated case for
// `-target hlsl` and must compile clean (exit-code=0, exhaustive: any
// diagnostic fails the test). The third is the control case: a whole-enum
// [require] consumed in the same position must keep failing with E36107.
// The fourth and fifth mirror the first two on `-target spirv`.
// The sixth and seventh (guarded by -DTEST_MIXED, because the conflicting
// consumer is diagnosed in every invocation that compiles it) consume a case
// that carries its own [require] inside a [require]-gated enum: the
// case-level and enum-level requirements conflict inside the consumer, so
// both targets fail with E36100.
// The eighth consumes a case with two stacked [require]s: their union
// (hlsl or spirv) is satisfiable on `-target hlsl`, so it must compile clean —
// pinning that multiple [require]s on one case union rather than intersect.
// The ninth consumes a case whose tag references a gated case: tag references
// are not capability-checked (mirroring variable initializers), so it must
// compile clean on `-target hlsl`.
// The tenth (guarded by -DTEST_UNSCOPED, because the unqualified name only
// resolves under `-unscoped-enum`) references the spirv-gated case by its
// unqualified name: the enclosing-scope alias the name resolves to must carry
// the case's requirements, so it must fail with E36107.
// The eleventh mirrors the tenth's success side: the unqualified spelling of
// a case the target satisfies must compile clean, pinning that the alias join
// propagates exactly the case's own requirements.
// The twelfth pins the parser design itself: case names that collide with
// registered modifier keywords (`point`, `linear`, `sample`) must keep
// parsing, with or without a leading attribute — a `ParseModifiers`-based
// parse would consume them as modifiers and reject the enum.
//
// The `if (v == ...)` in each entry point exercises the operator-position
// reference form in addition to the initializing assignment.

//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV_CASE_ON_HLSL,non-exhaustive):-target hlsl -entry computeMainSpirv -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL_CASE_ON_HLSL,exit-code=0):-target hlsl -entry computeMainHlsl -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=WHOLE_ENUM_ON_HLSL,non-exhaustive):-target hlsl -entry computeMainWholeEnum -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV_CASE_ON_SPIRV,exit-code=0):-target spirv -entry computeMainSpirv -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL_CASE_ON_SPIRV,non-exhaustive):-target spirv -entry computeMainHlsl -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED_ON_HLSL,non-exhaustive):-target hlsl -entry computeMainMixed -stage compute -DTEST_MIXED
//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED_ON_SPIRV,non-exhaustive):-target spirv -entry computeMainMixed -stage compute -DTEST_MIXED
//DIAGNOSTIC_TEST:SIMPLE(diag=UNION_CASE_ON_HLSL,exit-code=0):-target hlsl -entry computeMainUnion -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TAG_BOUNDARY_ON_HLSL,exit-code=0):-target hlsl -entry computeMainTag -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSCOPED_ON_HLSL,non-exhaustive):-target hlsl -unscoped-enum -entry computeMainUnscoped -stage compute -DTEST_UNSCOPED
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSCOPED_OK,exit-code=0):-target hlsl -unscoped-enum -entry computeMainUnscopedOk -stage compute -DTEST_UNSCOPED_OK
//DIAGNOSTIC_TEST:SIMPLE(diag=KEYWORD_NAMES,exit-code=0):-target hlsl -entry computeMainKeywordNames -stage compute

enum MyEnum
{
    [require(spirv)]
    FirstValue = 0,

    [require(hlsl)]
    SecondValue = 1,
};

[require(spirv)]
enum MyWholeEnum
{
    Value = 0,
};

// Stacked [require]s are alternatives: `EitherValue` is available on hlsl or
// spirv, so a hlsl compile must succeed.
enum UnionEnum
{
    [require(hlsl)]
    [require(spirv)]
    EitherValue = 0,
};

// `B`'s tag references the gated `A`; tag references are not capability-checked,
// so consuming `B` inherits nothing from `A`.
enum TagEnum
{
    [require(spirv)]
    A = 0,

    B = A,
};

// `point`, `linear` and `sample` are registered modifier keywords, so a
// `ParseModifiers`-based parse of enum cases would consume them as modifiers
// and reject this enum; bracket-only attribute parsing keeps them as names.
enum FilterMode
{
    [require(hlsl)]
    point = 0,

    linear = 1,

    sample = 2,
};

#ifdef TEST_MIXED
// Case-level and enum-level requirements combine: a consumer of `MixedValue`
// needs both `hlsl` and `spirv`, which no target satisfies.
[require(spirv)]
enum MixedEnum
{
    [require(hlsl)]
    MixedValue = 0,
};
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainSpirv(uint3 tid : SV_DispatchThreadID)
{
    MyEnum v = MyEnum.FirstValue;
    if (v == MyEnum.FirstValue)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainHlsl(uint3 tid : SV_DispatchThreadID)
{
    MyEnum v = MyEnum.SecondValue;
    switch (v)
    {
    case MyEnum.SecondValue:
        break;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainWholeEnum(uint3 tid : SV_DispatchThreadID)
{
    MyWholeEnum v = MyWholeEnum.Value;
    if (v == MyWholeEnum.Value)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainUnion(uint3 tid : SV_DispatchThreadID)
{
    UnionEnum v = UnionEnum.EitherValue;
    if (v == UnionEnum.EitherValue)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainTag(uint3 tid : SV_DispatchThreadID)
{
    TagEnum v = TagEnum.B;
    if (v == TagEnum.B)
    {
    }
}

#ifdef TEST_MIXED
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainMixed(uint3 tid : SV_DispatchThreadID)
{
    MixedEnum v = MixedEnum.MixedValue;
    if (v == MixedEnum.MixedValue)
    {
    }
}
#endif

#ifdef TEST_UNSCOPED
// Under `-unscoped-enum` the unqualified `FirstValue` resolves to the
// enclosing-scope alias the parser synthesizes for the case.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainUnscoped(uint3 tid : SV_DispatchThreadID)
{
    MyEnum v = FirstValue;
    if (v == FirstValue)
    {
    }
}
#endif

#ifdef TEST_UNSCOPED_OK
// The unqualified spelling of a case the target satisfies must compile clean:
// the alias join propagates exactly the case's own requirements, no more.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainUnscopedOk(uint3 tid : SV_DispatchThreadID)
{
    MyEnum v = SecondValue;
    if (v == SecondValue)
    {
    }
}
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainKeywordNames(uint3 tid : SV_DispatchThreadID)
{
    FilterMode m = FilterMode.point;
    if (m == FilterMode.linear)
    {
    }
    if (m == FilterMode.sample)
    {
    }
}

//SPIRV_CASE_ON_HLSL: E36107
//WHOLE_ENUM_ON_HLSL: E36107
//HLSL_CASE_ON_SPIRV: E36107
//MIXED_ON_HLSL: E36100
//MIXED_ON_SPIRV: E36100
//UNSCOPED_ON_HLSL: E36107
