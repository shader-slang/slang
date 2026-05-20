//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int a[2][3];
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 3; ++c)
            a[r][c] = r * 10 + c;
    // CHECK: top=2
    printf("top=%d\n", a[0][2]);
    // CHECK: bot=12
    printf("bot=%d\n", a[1][2]);
}
