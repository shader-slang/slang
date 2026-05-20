//TEST:INTERPRET(filecheck=CHECK):
bool bump(inout int c) { c++; return true; }
void main()
{
    int c = 0;
    bool _ = (false) && bump(c) && bump(c) && bump(c);
    // CHECK: c=0
    printf("c=%d\n", c);
}
