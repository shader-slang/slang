//TEST:INTERPRET(filecheck=CHECK):
struct E { int v; }
struct D { E e; }
struct C { D d; }
struct B { C c; }
struct A { B b; }
void main()
{
    A a;
    a.b.c.d.e.v = 42;
    // CHECK: v=42
    printf("v=%d\n", a.b.c.d.e.v);
}
