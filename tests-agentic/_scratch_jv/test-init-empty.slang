//TEST:INTERPRET(filecheck=CHECK):
struct R { int a; int b; int c; }
void main()
{
    R r = {};
    // CHECK: a=0 b=0 c=0
    printf("a=%d b=%d c=%d\n", r.a, r.b, r.c);
}
