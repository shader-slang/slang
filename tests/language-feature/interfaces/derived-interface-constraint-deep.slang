//TEST:INTERPRET(filecheck=CHECK):

// Interface-level `__constraint` must remain visible when inherited through a
// chain of derived interfaces.

interface IBase
{
    associatedtype DataType;
    DataType getData();
}

interface IMid : IBase
{
    __constraint DataType == This;
    int mid();
}

interface ILeaf : IMid
{
    int leaf();
}

int useEquality<T : ILeaf>(T x)
{
    // `DataType == This` is declared on IMid and reaches `T` through ILeaf, so
    // `x.getData()` (type `T.DataType == T`) provides both `mid()` and `leaf()`.
    return x.getData().mid() + x.getData().leaf();
}

struct Node : ILeaf
{
    typealias DataType = Node;
    Node getData() { return this; }
    int mid() { return 10; }
    int leaf() { return 5; }
}

void main()
{
    Node n;
    // CHECK: 15
    printf("%d\n", useEquality(n));
}
