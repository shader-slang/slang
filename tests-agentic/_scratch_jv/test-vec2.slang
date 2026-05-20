//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float2 v = float2(7.0, 8.0);
    // CHECK: v=7,8
    printf("v=%d,%d\n", int(v.x), int(v.y));
}
