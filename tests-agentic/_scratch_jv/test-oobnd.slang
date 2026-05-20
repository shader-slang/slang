//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int arr[4] = {10, 20, 30, 40};
    // CHECK: x={{.*}}
    printf("x=%d\n", arr[100]);
}
