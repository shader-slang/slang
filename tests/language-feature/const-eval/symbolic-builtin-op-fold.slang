// Regression test: shift / mod / bitwise on a symbolic (generic value parameter) integer fold
// through the decl-free `BuiltinOperationIntVal` (the `default:` arm of
// `tryConstantFoldBuiltinOperatorExpr`), which must substitute/resolve correctly when the
// generic is specialized. (`+`/`-`/`*` go through `PolynomialIntVal` instead and are covered
// elsewhere.)
//TEST:INTERPRET(filecheck=CHECK):

int sizes<int N>()
{
    int a[N << 1];        // Lsh   (symbolic BuiltinOperationIntVal)
    int b[N % 7];         // Mod
    int c[(N & 6) + 1];   // BitAnd (+1 keeps the array size positive)
    int d[N >> 1];        // Rsh
    return a.getCount() * 1000 + b.getCount() * 100 + c.getCount() * 10 + d.getCount();
}

void main()
{
    // sizes<5>: a=5<<1=10; b=5%7=5; c=(5&6)+1=4+1=5; d=5>>1=2  => 10*1000+5*100+5*10+2 = 10552
    printf("%d\n", sizes<5>());
    //CHECK: 10552
}
