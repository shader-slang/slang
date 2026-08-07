//TEST:SIMPLE(filecheck=FUNC1): -target spirv -entry func1
//TEST:SIMPLE(filecheck=FUNC2): -target spirv -entry func2
//TEST:SIMPLE(filecheck=FUNC3): -target spirv -entry func3
//TEST:SIMPLE(filecheck=FUNC4): -target spirv -entry func4

struct Results
{
    Optional<Optional<int>> a;
    Optional<int> b;
    int c;
};

RWStructuredBuffer<Results> results;

// FUNC1: %func1 = OpFunction
// FUNC1: OpLoad %int %a_value_value
// FUNC1: OpLoad %int %a_value_hasValue
[shader("vertex")]
void func1(Optional<Optional<int>> a)
{
    Optional<int> b = a.value;
    results[0].b = b;
}

// FUNC2: %func2 = OpFunction
// FUNC2: OpAccessChain %_ptr_StorageBuffer__slang_Optional__slang_Optional_int_std430
// FUNC2: OpStore %{{.+}} %int_0
// FUNC2: OpStore %{{.+}} %int_1
// FUNC2: OpStore %{{.+}} %int_1
// FUNC2: OpAccessChain %_ptr_StorageBuffer__slang_Optional_int_std430
// FUNC2: OpStore %{{.+}} %int_0
// FUNC2: OpStore %{{.+}} %int_1
[shader("vertex")]
void func2()
{
    Optional<Optional<int>> a = Optional<Optional<int>>(0);
    results[0].a = a;
    Optional<int> b = a.value;
    results[0].b = b;
}

// FUNC3: %func3 = OpFunction
// FUNC3: [[RESULT:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_int %{{.+}} %int_2
// FUNC3-NEXT: OpStore [[RESULT]] %int_0
[shader("vertex")]
void func3()
{
    int a = Optional<Optional<int>>(0).value.value;
    results[0].c = a;
}

// FUNC4: %func4 = OpFunction
// FUNC4: [[RESULT:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_int %{{.+}} %int_2
// FUNC4-NEXT: OpStore [[RESULT]] %int_0
[shader("vertex")]
void func4()
{
    Optional<Optional<int>> a = Optional<Optional<int>>(0);
    Optional<int> b = a.value;
    int c = b.value;
    results[0].c = c;
}
