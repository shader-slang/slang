//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float4x4 m;
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            m[r][c] = float(r*4 + c);
    // CHECK: ok=15
    printf("ok=%d\n", int(m[3][3]));
}
