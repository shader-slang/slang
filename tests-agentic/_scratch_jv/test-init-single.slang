//TEST:INTERPRET(filecheck=CHECK):
struct One { int v; }
void main()
{
    One o = {99};
    // CHECK: v=99
    printf("v=%d\n", o.v);
}
