//TEST:LANG_SERVER(filecheck=CHECK):

// Verify hover reports the folded value for a static const initialized
// through recursive PackBranch-based value aggregation.

interface IEval
{
    static const int value;
}

struct Empty : IEval
{
    static const int value = 0;
}

struct SumNode<let Head : int, let each Tail : int> : IEval
{
    typealias Next = __packBranch(
        __trimHead(Tail),
        Empty,
        SumNode<__first(Tail), __trimHead(Tail)>);
    static const int value = Head + Next.value;
}

typealias Sum<let each vals : int> = __packBranch(vals, Empty, SumNode<__first(vals), vals>);

//HOVER:29,19
// CHECK: static const int sum = 10
static const int sum = Sum<1, 2, 3, 4>.value;
