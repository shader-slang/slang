//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main

class Counter
{
    uint value;
    __init() {}
}

// Success and error carry the same nominal type. Their case identity must survive lowering,
// and either selected payload must outlive the function that creates it.
[noinline]
Counter makeResult(uint selector) throws Counter
{
    Counter result = new Counter();
    result.value = 41 + selector;
    if (selector != 0)
        throw result;
    return result;
}

[noinline]
void makeVoidResult(uint selector) throws Counter
{
    if (selector != 0)
    {
        Counter error = new Counter();
        error.value = 50 + selector;
        throw error;
    }
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    uint one = hgbc_asm { result : $$uint = OpIntLit 1; };
    do
    {
        Counter result = try makeResult(zero);
        print(result.value);
    }
    catch (error: Counter)
    {
        print(error.value + 1000);
    }
    do
    {
        Counter result = try makeResult(one);
        print(result.value + 1000);
    }
    catch (error: Counter)
    {
        print(error.value);
    }
    do
    {
        try makeVoidResult(zero);
        print(7u);
        try makeVoidResult(one);
        print(1000u);
    }
    catch (error: Counter)
    {
        print(error.value);
    }
}

// CHECK: 41
// CHECK-NEXT: 42
// CHECK-NEXT: 7
// CHECK-NEXT: 51

// BYTECODE-NOT: AnyValue
// BYTECODE-DAG: OpBoxType
// BYTECODE-DAG: OpTaggedUnionType
// BYTECODE-DAG: OpBox
// BYTECODE-DAG: OpExtractTaggedUnion
// BYTECODE-NOT: AnyValue
