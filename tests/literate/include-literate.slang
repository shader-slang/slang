//TEST:INTERPRET(filecheck=CHECK):

module include_literate;

__include "include-literate-impl";

int main()
{
    // CHECK: 142
    printf("%d\n", literateIncludedFunc(42));
    return 0;
}
