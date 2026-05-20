//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int r = ((((1 + 2) * 3) - 4) / 5);
    // (((3)*3)-4)/5 = (9-4)/5 = 1
    // CHECK: r=1
    printf("r=%d\n", r);
}
