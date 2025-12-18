// Check that we can parse and check an anonymous struct declaration as function return type.

//TEST:INTERPRET(filecheck=CHECK):

module testa;

func test(int a) -> struct{ int a,b; } {
    return {a,2};
}

void main()
{
    let m = test(1);
    // CHECK: 3
    printf("%d\n", m.a + m.b);
}
