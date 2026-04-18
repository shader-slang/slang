// Regression test for https://github.com/shader-slang/slang/issues/8135
// Using an associatedtype declared in a parent interface inside a child
// interface's method signatures previously crashed with:
//   assert failure: parentNonBlock  (slang-ir.cpp)

//TEST:INTERPRET(filecheck=CHECK):

public interface IStorage<T>
{
    public associatedtype Address;
}

public interface IParameterStorage<T> : IStorage<T>
{
    [mutating] public void add(Address address, T value);
    public Address getAddress(T value);
}

public struct MyStorage : IParameterStorage<int>
{
    public typedef uint Address;

    private uint _addr;
    private int  _val;

    [mutating] public void add(uint address, int value)
    {
        _addr = address;
        _val  = value;
    }

    public uint getAddress(int value) { return _addr; }
}

void main()
{
    MyStorage s;
    s.add(42u, 10);
    let addr = s.getAddress(10);
    // CHECK: 42
    printf("%u\n", addr);
}
