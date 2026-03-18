//TEST:INTERPRET(filecheck=CHECK):

// Verify the core type-level PackBranch behavior, including recursive helper
// expansion and symbolic branch selection over packs.

interface IStaticInt
{
    static int get();
}

struct IntConst<let N : int> : IStaticInt
{
    static int get() { return N; }
}

int chooseType<let each S : int>()
{
    return __packBranch(S, IntConst<1>, IntConst<2>)::get();
}

int chooseTypePack<each T>()
{
    return __packBranch(T, IntConst<1>, IntConst<2>)::get();
}

interface IEvalNode
{
    associatedtype Result : IStaticInt;
    static int eval();
}

struct EvalLeaf<let Head : int> : IEvalNode
{
    typealias Result = IntConst<Head>;

    static int eval()
    {
        return Head;
    }
}

struct EvalNode<let Head : int, let each Tail : int> : IEvalNode where nonempty(Tail)
{
    typealias Next = EvalHelper<__first(Tail), __trimHead(Tail)>;
    typealias Result = Next::Result;

    static int eval()
    {
        return Next::eval();
    }
}

typealias EvalHelper<let Head : int, let each Tail : int> =
    __packBranch(Tail, EvalLeaf<Head>, EvalNode<Head, Tail>);

int helperResult<let Head : int, let each Tail : int>()
{
    return EvalHelper<Head, Tail>::Result::get();
}

int helperEval<let Head : int, let each Tail : int>()
{
    return EvalHelper<Head, Tail>::eval();
}

void main()
{
    printf("%d\n", chooseType<>());
    // CHECK: 1

    printf("%d\n", chooseType<4>());
    // CHECK: 2

    printf("%d\n", chooseTypePack<>());
    // CHECK: 1

    printf("%d\n", chooseTypePack<int>());
    // CHECK: 2

    printf("%d\n", helperResult<4>());
    // CHECK: 4

    printf("%d\n", helperResult<4, 8, 16>());
    // CHECK: 16

    printf("%d\n", helperEval<4, 8, 16>());
    // CHECK: 16
}
