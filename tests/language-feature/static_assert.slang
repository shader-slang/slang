//TEST:SIMPLE(filecheck=CHK):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHK_NEG):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV):-target spirv -stage compute -entry computeMain

static_assert(true, "PASS_global");
static_assert(1 + 1 == 2);

__generic<T:IArithmetic>
void TEST_specialize()
{
    static_assert(true,   "TEST_specialize true");
    static_assert(T is float, "TEST_specialize T_is_float");
    static_assert(T is int,   "TEST_specialize T_is_int");
}

void TEST_target_switch()
{
    static_assert(false, "TEST_target_switch all");
    __target_switch
    {
    case hlsl:
        static_assert(false, "TEST_target_switch hlsl");
        return;
    case glsl:
        static_assert(false, "TEST_target_switch glsl");
        return;
    default:
        static_assert(false, "TEST_target_switch default");
        return;
    }
}

__generic<T:IArithmetic>
struct MyType
{
    __init()
    {
        static_assert(true, "MyType init true");
        static_assert(false, "MyType init false");
    }
};

__generic<T:IFloat>
extension MyType<T>
{
};

[numthreads(1,1,1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    // CHK-DAG: TEST_specialize T_is_int
    // CHK-DAG: TEST_specialize<float>
    TEST_specialize<float>();

    // CHK-DAG: TEST_specialize T_is_float
    // CHK-DAG: TEST_specialize<int>
    TEST_specialize<int>();

    //HLSL: TEST_target_switch all
    //HLSL: TEST_target_switch hlsl
    //HLSL-NOT: TEST_target_switch glsl
    //HLSL-NOT: TEST_target_switch default
    //
    //GLSL: TEST_target_switch all
    //GLSL-NOT: TEST_target_switch hlsl
    //GLSL: TEST_target_switch glsl
    //GLSL-NOT: TEST_target_switch default
    //
    //SPV: TEST_target_switch all
    //SPV-NOT: TEST_target_switch hlsl
    //SPV-NOT: TEST_target_switch glsl
    //SPV: TEST_target_switch default
    //
    TEST_target_switch();

    //CHK_NEG-NOT: MyType init true
    //CHK-DAG: MyType init false
    MyType<float> obj;

    // This file's failing assertions abort compilation, so these buffers hold the diagnostic
    // output. The passing global-scope assertion must produce no diagnostic on any target — its
    // message must not appear. (Carrier removal from *emitted* code is covered by
    // static-assert-carrier-no-leak.slang, which compiles successfully.)
    //CHK_NEG-NOT: PASS_global
    //GLSL-NOT: PASS_global
    //SPV-NOT: PASS_global

    //CHK-DAG: error[E41402]
    //CHK-DAG: static_assert.slang:[[#@LINE+1]]
    static_assert(dispatchThreadID.x == 0, "not constant");
}

