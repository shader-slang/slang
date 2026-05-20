//TEST:INTERPRET(filecheck=CHECK):
int five() { return 5; }
void main()
{
    // CHECK: r=5
    printf("r=%d\n", five());
}
