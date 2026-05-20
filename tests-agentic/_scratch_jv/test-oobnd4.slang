//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main()
{
    int arr[4] = {10, 20, 30, 40};
    int x = arr[100];
    printf("%d\n", x);
}
