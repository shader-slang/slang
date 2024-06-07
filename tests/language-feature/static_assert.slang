//TEST:SIMPLE(filecheck=CHK):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHK):-target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHK):-target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV):-target spirv -stage compute -entry computeMain

// TODO: requires changes in parser
//static_assert(true, "PASS_global");
//static_assert(false, "FAIL_global");

__generic<T:IArithmetic>
void TEST_specialize_float()
{
    //CHK-NOT:PASS_specialize_float
    static_assert(T is float, "PASS_specialize_float");

    //CHK: ([[#@LINE+1]]): error 41400:
    static_assert(T is int, "FAIL_specialize_float");
}

__generic<T:IArithmetic>
void TEST_specialize_int()
{
    //CHK: ([[#@LINE+1]]): error 41400:
    static_assert(T is float, "FAIL_specialize_int");

    //CHK-NOT:PASS_specialize_int
    static_assert(T is int, "PASS_specialize_int");
}

void TEST_target_switch()
{
    // We should hit only one for the given target
    __target_switch
    {
    case hlsl:
        //HLSL: error {{.*}} FAIL_HLSL
        //HLSL-NOT: error {{.*}} FAIL_GLSL
        //HLSL-NOT: error {{.*}} FAIL_SPV
        static_assert(false, "FAIL_HLSL");
        return;
    case glsl:
        //GLSL: error {{.*}} FAIL_GLSL
        //GLSL-NOT: error {{.*}} FAIL_HLSL
        //GLSL-NOT: error {{.*}} FAIL_SPV
        static_assert(false, "FAIL_GLSL");
        return;
    default:
        //SPV: error {{.*}} FAIL_SPV
        //SPV-NOT: error {{.*}} FAIL_HLSL
        //SPV-NOT: error {{.*}} FAIL_GLSL
        static_assert(false, "FAIL_SPV");
        return;
    }
}

__generic<T:IArithmetic>
struct MyType
{
    // TODO: requires changes in parser
    //static_assert(true, "PASS_struct");
    //static_assert(false, "FAIL_struct");

    __init()
    {
        //CHK-NOT:PASS_init
        static_assert(true, "PASS_init");

        //CHK: ([[#@LINE+1]]): error 41400:
        static_assert(false, "FAIL_init");
    }
};

__generic<T:IFloat>
extension MyType<T>
{
};

[numthreads(1,1,1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    TEST_specialize_float<float>();
    TEST_specialize_int<int>();
    TEST_target_switch();
    MyType<float> obj;

    //CHK: ([[#@LINE+1]]): error 41402:
    static_assert(dispatchThreadID.x == 0, "FAIL_not_constant");
}

