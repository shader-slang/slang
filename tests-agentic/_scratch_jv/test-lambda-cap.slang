//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int delta = 10;
    var addDelta = (int x) => x + delta;
    // CHECK: r=17
    printf("r=%d\n", addDelta(7));
}
