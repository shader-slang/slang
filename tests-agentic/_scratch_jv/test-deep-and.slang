//TEST:INTERPRET(filecheck=CHECK):
bool bump(inout int c) { c++; return true; }
void main()
{
    int c = 0;
    // First false short-circuits the rest.
    bool r = (false) && bump(c) && bump(c) && bump(c);
    // CHECK: c=0
    printf("c=%d\n", c);
    int c2 = 0;
    bool r2 = (true) && bump(c2) && bump(c2) && bump(c2);
    // CHECK: c2=3
    printf("c2=%d\n", c2);
}
