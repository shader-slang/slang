//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
void main()
{
    int arr[4] = {10, 20, 30, 40};
    int x = arr[100];
//CHECK: error
    printf("%d\n", x);
}
