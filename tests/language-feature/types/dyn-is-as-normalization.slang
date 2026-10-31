//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang legacy -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2025 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2026 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 202c -output-using-type

//TEST_INPUT:ubuffer(count=28, stride=4):out
RWStructuredBuffer<int> outputBuffer;

[anyValueSize(16)]
interface IFoo
{
    int get();
}

struct Foo : IFoo
{
    int payload;

    int get()
    {
        return payload;
    }
}

struct Bar : IFoo
{
    int payload;

    int get()
    {
        return payload;
    }
}

dyn IFoo makeRuntimeBox(int kind, int payload)
{
    if ((kind & 1) == 0)
        return Foo(payload);
    return Bar(payload);
}

struct Storage
{
    int unused;

    property dyn IFoo item
    {
        get
        {
            outputBuffer[0] += 1;
            return Foo(37);
        }
        set(dyn IFoo value)
        {
            outputBuffer[1] += 1;
        }
    }
}

dyn IFoo makeEffectfulBox()
{
    outputBuffer[19] += 1;
    return Foo(37);
}

typealias Box = dyn IFoo;

Optional<T> castBox<T>(Box value)
{
    return value as T;
}

bool ordinaryIs<Source : IFoo>(Source value)
{
    return value is Foo;
}

Optional<Foo> ordinaryAs<Source : IFoo>(Source value)
{
    return value as Foo;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // The input keeps both payload types possible at runtime. Direct tests and casts must
    // distinguish the conformance witnesses and preserve the successful payload values.
    int kind = outputBuffer[27];
    dyn IFoo first = makeRuntimeBox(kind, 37);
    dyn IFoo second = makeRuntimeBox(kind + 1, 53);
    outputBuffer[2] = first is Foo ? 1 : 0;
    outputBuffer[3] = first is Bar ? 1 : 0;
    outputBuffer[4] = second is Foo ? 1 : 0;
    outputBuffer[5] = second is Bar ? 1 : 0;
    let firstFoo = first as Foo;
    outputBuffer[6] = firstFoo.hasValue ? 1 : 0;
    outputBuffer[7] = firstFoo.hasValue ? firstFoo.value.payload : -1;
    outputBuffer[8] = (first as Bar).hasValue ? 1 : 0;
    outputBuffer[9] = (second as Foo).hasValue ? 1 : 0;
    let secondBar = second as Bar;
    outputBuffer[10] = secondBar.hasValue ? 1 : 0;
    outputBuffer[11] = secondBar.hasValue ? secondBar.value.payload : -1;

    // Opening a property with both accessors currently reads its getter twice: once for the
    // extracted type and once for the payload and witness. These counts preserve accepted
    // evaluation behavior while the checked AST makes that opening explicit.
    Storage storage = Storage(0);
    outputBuffer[0] = 0;
    outputBuffer[1] = 0;
    outputBuffer[14] = storage.item is Foo ? 1 : 0;
    outputBuffer[12] = outputBuffer[0];
    outputBuffer[13] = outputBuffer[1];

    outputBuffer[0] = 0;
    outputBuffer[1] = 0;
    let propertyFoo = storage.item as Foo;
    outputBuffer[15] = outputBuffer[0];
    outputBuffer[16] = outputBuffer[1];
    outputBuffer[17] = propertyFoo.hasValue ? 1 : 0;
    outputBuffer[18] = propertyFoo.hasValue ? propertyFoo.value.payload : -1;

    // A proven interface test does not evaluate its operand, including an effectful call.
    outputBuffer[19] = 0;
    outputBuffer[20] = makeEffectfulBox() is IFoo ? 1 : 0;

    // In `castBox`, the unconstrained target is compared with the whole box type. Casting to
    // `Foo` fails and reads the argument's getter three times without calling the setter.
    outputBuffer[0] = 0;
    outputBuffer[1] = 0;
    let genericFoo = castBox<Foo>(storage.item);
    outputBuffer[21] = outputBuffer[0];
    outputBuffer[22] = outputBuffer[1];
    outputBuffer[23] = genericFoo.hasValue ? 1 : 0;
    outputBuffer[24] = genericFoo.hasValue ? genericFoo.value.payload : -1;

    // These generic calls specialize `Source` to the statically known type `Foo`, so their
    // ordinary type comparisons succeed in every tested language mode.
    outputBuffer[25] = ordinaryIs(Foo(37)) ? 1 : 0;
    outputBuffer[26] = ordinaryAs(Foo(37)).value.payload;
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^3$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^37$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^53$}}
// CHECK-NEXT: {{^2$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^2$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^37$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^3$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^-1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^37$}}
// CHECK-NEXT: {{^0$}}
