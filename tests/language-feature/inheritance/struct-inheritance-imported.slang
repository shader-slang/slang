//TEST_IGNORE_FILE:
// struct-inheritance-imported.slang

struct Base
{
    int a;
}

struct Derived : Base
{}

int getA(Derived d) { return d.a; }
