//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int arr[4] = {10, 20, 30, 40};
    int x = arr[100];
    printf("x=%d\n", x);
    //CHECK: anything
}
