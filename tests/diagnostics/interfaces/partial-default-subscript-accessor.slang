//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IFoo<TValue, int Offset>
{
    TValue getVal(int index);

    __subscript<TIndex>(TIndex index) -> TValue
        where TIndex : IInteger
    {
        get { return getVal(index.toInt() + Offset); }
//CHECK:^^^ partial interface accessor default implementation
//CHECK:^^^ interface accessor default implementation is incomplete; define all accessors or none.
        set;
    }
}
