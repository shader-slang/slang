//TEST:INTERPRET(filecheck=CHECK): -warnings-as-errors all

// A retained explicit identity cast must preserve the checked operand's behavior.
static const int count = int(float(2.0f));

int readCount<let N : int>()
{
    return N;
}

int choose(uint8_t value)
{
    return 1;
}

int choose(float value)
{
    return 2;
}

int receiveDouble(double value)
{
    return value == 1.0 ? 1 : 0;
}

int recordValue(inout int calls, int value)
{
    calls++;
    return value;
}

bool recordCondition(inout int calls)
{
    calls++;
    return true;
}

void run(bool flag)
{
    int calls = 0;
    int selected = int(flag ? recordValue(calls, 10) : recordValue(calls, 20));
    printf("select %d %d\n", selected, calls);

    calls = 0;
    bool conjunction = bool(flag && recordCondition(calls));
    printf("and %d %d\n", int(conjunction), calls);

    calls = 0;
    bool disjunction = bool(flag || recordCondition(calls));
    printf("or %d %d\n", int(disjunction), calls);
}

void main()
{
    run(true);
    // CHECK: select 10 1
    // CHECK-NEXT: and 1 1
    // CHECK-NEXT: or 1 0

    run(false);
    // CHECK-NEXT: select 20 1
    // CHECK-NEXT: and 0 0
    // CHECK-NEXT: or 1 1

    // Literal folding must still work where a compile-time constant is required.
    int values[count] = { 10, 20 };
    printf("constant %d %d\n", readCount<count>(), values[1]);
    // CHECK-NEXT: constant 2 20

    printf("bits %d %d\n",
        __floatAsInt(float(float(1.0f))), __floatAsInt(float(int(1))));
    // CHECK-NEXT: bits 1065353216 1065353216

    // Literal-specific overload ranking and warning exemptions survive the cast.
    printf("literal %d %d\n", choose(int(1)), receiveDouble(float(1.0f)));
    // CHECK-NEXT: literal 1 1

    // Taking the address through an identity cast must still reach the original lvalue.
    int value = 7;
    int* pointer = __getAddress(int(value));
    *pointer = 9;
    printf("address %d\n", value);
    // CHECK-NEXT: address 9

    NativeString text = NativeString("");
    printf("string [%s]\n", text);
    // CHECK-NEXT: string []
}
