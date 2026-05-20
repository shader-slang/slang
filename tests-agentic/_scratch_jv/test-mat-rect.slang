//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float3x2 m = float3x2(1, 2, 3, 4, 5, 6);
    // CHECK: m=1,2|3,4|5,6
    printf("m=%d,%d|%d,%d|%d,%d\n",
        int(m[0][0]), int(m[0][1]),
        int(m[1][0]), int(m[1][1]),
        int(m[2][0]), int(m[2][1]));
}
