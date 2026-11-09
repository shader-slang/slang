//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct Resources
{
    RWStructuredBuffer<uint> values;
}

[noinline]
Optional<RWStructuredBuffer<uint>> getNoBuffer()
{
    return none;
}

[noinline]
Optional<Resources> getNoResources()
{
    return none;
}

[host]
void main()
{
    print("buffer has value:");
    print(getNoBuffer().hasValue);
    print("resources have value:");
    print(getNoResources().hasValue);
}

// CHECK: buffer has value:
// CHECK-NEXT: false
// CHECK-NEXT: resources have value:
// CHECK-NEXT: false
