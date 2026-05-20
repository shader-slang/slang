//TEST:INTERPRET(filecheck=CHECK):
interface IA { int a(); }
interface IB { int b(); }
interface IC { int c(); }
struct Triple : IA, IB, IC
{
    int a() { return 100; }
    int b() { return 20; }
    int c() { return 3; }
}
int combine<T : IA & IB & IC>(T v) { return v.a() + v.b() + v.c(); }
void main()
{
    Triple t;
    // CHECK: r=123
    printf("r=%d\n", combine(t));
}
