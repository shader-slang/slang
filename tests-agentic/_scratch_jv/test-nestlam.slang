//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    var add = (int x) => (int y) => x + y;
    var addToFive = add(5);
    // CHECK: r=12
    printf("r=%d\n", addToFive(7));
}
