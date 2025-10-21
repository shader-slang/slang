//TEST:INTERPRET(filecheck=CHECK):

void printNumbers<each T>(expand each T args) where T == int
{
    // An single expression statement whose type will be `(void, void, ...)`.
    // where each `void` is the result of evaluating expression `printf(...)` with
    // each corresponding element in `args` passed as print operand.
    //
    expand printf("%d\n", each args);

    // The above statement is equivalent to:
    // ```
    // (printf("%d\n", args[0]), printf("%d\n", args[1]), ..., printf("%d\n", args[n-1]));
    // ```
}
void compute<each T>(expand each T args) where T == int
{
    // Maps every element in `args` to `elementValue + 1`, and forwards the
    // new values as arguments to `printNumber`.
    printNumbers(expand ((each args) + 1));

    // The above statement is equivalent to:
    // ```
    // printNumber(args[0] + 1, args[1] + 1, ..., args[n-1] + 1);
    // ```
}
void main()
{
    compute(1,2,3);
    // CHECK: 2
    // CHECK: 3
    // CHECK: 4
}