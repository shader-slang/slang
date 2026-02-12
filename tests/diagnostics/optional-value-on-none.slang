// Test that accessing .value on both Optional and Conditional that are always 'none' produces errors
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -DTEST_OPT
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_COND): -target spirv -DTEST_COND
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_CONDTEX): -target spirv -DTEST_CONDTEX
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_IFLET): -target spirv -DTEST_IFLET

#if defined(TEST_CONDTEX) || defined(TEST_IFLET)
uniform Conditional<Texture2D, false> Tex;
#endif

[shader("fragment")]
float4 main() : SV_Target
{
#ifdef TEST_OPT
    Optional<float4> opt = none;
    // CHECK_OPT: ([[# @LINE+1]]): error 41027
    return opt.value;
#endif

#ifdef TEST_COND
    Conditional<float4, false> cond;
    // CHECK_COND: ([[# @LINE+1]]): error 41027
    return cond.get().value;
#endif

#ifdef TEST_CONDTEX
    // CHECK_CONDTEX: ([[# @LINE+1]]): error 41027
    return Tex.get().value.Load(int3(0));
#endif

#ifdef TEST_IFLET
    // CHECK_IFLET-NOT: error 41027
    if (let t = Tex.get())
    {
        return t.Load(int3(0));
    }
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
#endif
}
