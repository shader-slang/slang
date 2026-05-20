//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float3 a = float3(1.0, 2.0, 3.0);
    float3 b = float3(int3(10, 20, 30));
    float3 c = a + b;
    // CHECK: c=11,22,33
    printf("c=%d,%d,%d\n", int(c.x), int(c.y), int(c.z));
}
