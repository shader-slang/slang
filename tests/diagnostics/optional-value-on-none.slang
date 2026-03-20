// Test that accessing .value on both Optional and Conditional that are always 'none' produces errors
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -DTEST_OPT
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_COND): -target spirv -DTEST_COND
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_CONDTEX): -target spirv -DTEST_CONDTEX
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_IFLET): -target spirv -DTEST_IFLET
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_VALUE): -target spirv -DTEST_VALUE
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NESTED): -target spirv -DTEST_NESTED
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_INLINE): -target spirv -DTEST_INLINE

#if defined(TEST_CONDTEX) || defined(TEST_IFLET)
uniform Conditional<Texture2D, false> Tex;
#endif

#ifdef TEST_INLINE
float4 alwaysNone()
{
    Optional<float4> o = none;
    return o.value;
}
#endif

[shader("fragment")]
float4 main() : SV_Target
{
#ifdef TEST_OPT
    Optional<float4> opt = none;
    // CHECK_OPT: error[E41027]
    // CHECK_OPT-NEXT: --> {{.*}}:[[# @LINE+1]]:
    return opt.value;
#endif

#ifdef TEST_COND
    Conditional<float4, false> cond;
    // CHECK_COND: error[E41027]
    // CHECK_COND-NEXT: --> {{.*}}:[[# @LINE+1]]:
    return cond.get().value;
#endif

#ifdef TEST_CONDTEX
    // CHECK_CONDTEX: error[E41027]
    // CHECK_CONDTEX-NEXT: --> {{.*}}:[[# @LINE+1]]:
    return Tex.get().value.Load(int3(0));
#endif

#ifdef TEST_IFLET
    // CHECK_IFLET-NOT: error[E41027]
    if (let t = Tex.get())
    {
        return t.Load(int3(0));
    }
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
#endif

#ifdef TEST_VALUE
    Optional<float4> opt = float4(1.0);
    // CHECK_VALUE-NOT: error[E41027]
    return opt.value;
#endif

#ifdef TEST_NESTED
    Optional<Optional<int>> opt = none;
    // CHECK_NESTED: error[E41027]
    // CHECK_NESTED-NEXT: --> {{.*}}:[[# @LINE+1]]:
    return opt.value.value;
#endif

#ifdef TEST_INLINE
    // CHECK_INLINE: error[E41027]
    return alwaysNone();
#endif
}
