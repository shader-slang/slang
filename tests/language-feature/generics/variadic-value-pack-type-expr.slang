//TEST:INTERPRET(filecheck=CHECK):

// Test that value pack params are correctly scoped inside
// array type expressions in return type position.
// This exercises the ReplaceScopeVisitor for IndexExpr,
// ExpandExpr, and EachExpr in the parser.

struct Arr<let each N : int>
{
    int getCount() { return countof(N); }
}

// `D` is a value pack referenced inside expand/each in a return type
// nested inside an array type expr.
Arr<expand each D>[countof(D)] makeArrs<let each D : int>()
{
    Arr<expand each D> result[countof(D)];
    return result;
}

void main()
{
    let arrs = makeArrs<3, 5, 7>();
    printf("%d\n", arrs.getCount());
    // CHECK: 3
    printf("%d\n", arrs[0].getCount());
    // CHECK: 3
}
