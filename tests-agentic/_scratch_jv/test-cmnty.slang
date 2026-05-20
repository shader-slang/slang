//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    bool c = true;
    float v = c ? 1 : 2.5f;
    // CHECK: v=1
    printf("v=%d\n", int(v));
    bool c2 = false;
    float v2 = c2 ? 1 : 2.5f;
    // CHECK: v2=2.5
    printf("v2=%.1f\n", v2);
}
