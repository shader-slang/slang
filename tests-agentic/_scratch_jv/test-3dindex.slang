//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int arr[2][2][2];
    arr[0][0][0] = 1;
    arr[1][1][1] = 8;
    arr[0][1][0] = 3;
    // CHECK: a=1 b=8 c=3
    printf("a=%d b=%d c=%d\n", arr[0][0][0], arr[1][1][1], arr[0][1][0]);
}
