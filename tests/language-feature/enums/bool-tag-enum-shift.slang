//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// A `bool`-tagged enum conforms to `ILogical`, so it can be shifted. The shift round-trips through
// `int`, as a `bool` shift does: `On << 1` is `On` and `On >> 1` is `Off`. Each form below reaches
// a different producer of the shift: the `__EnumType` operator, the synthesized `shl`/`shr`
// witness called directly, and the same witness called through a generic `T : ILogical`.

enum Mode : bool
{
    Off = false,
    On = true,
}

T shlLogical<T : ILogical>(T value, uint amount)
{
    return value << amount;
}
T shrLogical<T : ILogical>(T value, uint amount)
{
    return value >> amount;
}
T shrMethod<T : ILogical>(T value, int amount)
{
    return value.shr(amount);
}

int toInt(Mode m)
{
    return m == Mode.On ? 1 : 0;
}

//TEST_INPUT:ubuffer(data=[1 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Mode on = outputBuffer[0] != 0 ? Mode.On : Mode.Off;
    int i = 0;
    outputBuffer[i++] = toInt(on << 1u);              // CHECK: {{^}}1{{$}}
    outputBuffer[i++] = toInt(on >> 1);               // CHECK-NEXT: {{^}}0{{$}}
    outputBuffer[i++] = toInt(on.shl(2));             // CHECK-NEXT: {{^}}1{{$}}
    outputBuffer[i++] = toInt(on.shr(1));             // CHECK-NEXT: {{^}}0{{$}}
    outputBuffer[i++] = toInt(shlLogical(on, 3u));    // CHECK-NEXT: {{^}}1{{$}}
    outputBuffer[i++] = toInt(shrLogical(on, 1u));    // CHECK-NEXT: {{^}}0{{$}}
    outputBuffer[i++] = toInt(shrMethod(on, 0));      // CHECK-NEXT: {{^}}1{{$}}
    Mode m = on;
    m >>= 1;
    outputBuffer[i++] = toInt(m);                     // CHECK-NEXT: {{^}}0{{$}}
    outputBuffer[i++] = toInt(Mode.Off << 1);         // CHECK-NEXT: {{^}}0{{$}}
}
