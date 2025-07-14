//TEST_IGNORE_FILE:
// struct-inheritance-imported.slang
#pragma warning(disable:30816)
public struct Base
{
    public int a;
}

public struct Derived : Base
{}

public int getA(Derived d) { return d.a; }
