//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

String format<each T>(String pattern, expand each T arguments)
{
    return hgbc_asm
    {
        result : $$String = OpFormatString $pattern $expand each arguments;
    };
}

uint getStringByteCount(String value)
{
    return hgbc_asm
    {
        result : $$uint = OpStringGetByteCount $value;
    };
}

uint readStringByte(String value, uint index)
{
    return hgbc_asm
    {
        result : $$uint = OpStringReadByte $value $index;
    };
}

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

struct StringHolder
{
    String value;
}

[noinline]
String echo(String value)
{
    return value;
}

[noinline]
String makeLabel(String prefix, uint index, float weight, bool active)
{
    StringHolder holder = {format("{}:{}:{:.1f}:{}", prefix, index, weight, active)};
    return format("<{}>", holder.value);
}

[noinline]
String readSecond(Array<String, 2> values)
{
    return values[1];
}

[host]
void main()
{
    print(echo("literal through call"));

    String label = makeLabel("item", 7, 2.5, true);
    print(label);
    print(getStringByteCount(label));
    print(readStringByte(label, 0));
    print(readStringByte(label, 16));
    if (getStringByteCount(label) != 17)
        fail(format("unexpected label: {}", label));

    String overwritten = format("old {}", 1);
    overwritten = format("new {}", 2);
    print(overwritten);

    String loopValue = "not formatted yet";
    for (uint i = 0; i < 3; ++i)
        loopValue = format("loop {}", i);
    print(loopValue);

    Array<String, 2> values = {format("left {}", 3), format("right {}", 4)};
    print(readSecond(values));
    print(format("{{{1}}}-{0}", "zero", "one"));
}

// CHECK: literal through call
// CHECK-NEXT: <item:7:2.5:true>
// CHECK-NEXT: 17
// CHECK-NEXT: 60
// CHECK-NEXT: 62
// CHECK-NEXT: new 2
// CHECK-NEXT: loop 2
// CHECK-NEXT: right 4
// CHECK-NEXT: {one}-zero
