//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang legacy -output-using-type

//TEST_INPUT:ubuffer(count=7, stride=4):out
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
    // In legacy mode, an explicit generic argument can be a boxed type. In `castBox`, the
    // unresolved target is compared with the whole operand's type, so the cast to `Box`
    // succeeds. We preserve the three getter calls without any setter call.
    outputBuffer[0] = 0;
    outputBuffer[1] = 0;
    Storage storage = Storage(0);
    let result = castBox<Box>(storage.item);
    outputBuffer[2] = result.hasValue ? 1 : 0;
    outputBuffer[3] = result.hasValue ? result.value.get() : -1;

    // Legacy mode also permits these generic calls with an opened runtime payload type.
    // The input keeps both payload types possible, so `Source` remains unknown and the
    // ordinary comparisons with `Foo` fail even when the selected payload is a `Foo`.
    int kind = outputBuffer[6];
    dyn IFoo value = makeRuntimeBox(kind, 37);
    outputBuffer[4] = ordinaryIs(value) ? 1 : 0;
    outputBuffer[5] = ordinaryAs(value).hasValue ? 1 : 0;
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^3$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^37$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
