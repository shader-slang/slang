//TEST:SIMPLE(filecheck=FUNC1): -target spirv -O0 -entry func1
//TEST:SIMPLE(filecheck=FUNC2): -target spirv -O0 -entry func2
//TEST:SIMPLE(filecheck=FUNC3): -target spirv -O0 -entry func3
//TEST:SIMPLE(filecheck=FUNC4): -target spirv -O0 -entry func4

// FUNC1: %func1 = OpFunction
// FUNC1: OpLoad %int %a_value_value
// FUNC1: OpLoad %int %a_value_hasValue
[shader("vertex")]
void func1(Optional<Optional<int>> a)
{
    Optional<int> b = a.value;
    unused(b);
}

// FUNC2: OpConstantComposite %_slang_Optional__slang_Optional_int %{{.+}} %false
// FUNC2: %func2 = OpFunction
// FUNC2: OpCompositeExtract %_slang_Optional_int %{{.+}} 0
[shader("vertex")]
void func2()
{
    Optional<Optional<int>> a = Optional<Optional<int>>();
    unused(a);
    Optional<int> b = a.value;
    unused(b);
}

// FUNC3: %func3 = OpFunction
// FUNC3: OpStore %a %{{.+}}
[shader("vertex")]
void func3()
{
    int a = Optional<Optional<int>>().value.value;
    unused(a);
}

// FUNC4: %func4 = OpFunction
// FUNC4: OpStore %c %{{.+}}
[shader("vertex")]
void func4()
{
    Optional<Optional<int>> a = Optional<Optional<int>>();
    Optional<int> b = a.value;
    int c = b.value;
    unused(c);
}
