//TEST:DIAGNOSTIC_SIMPLE
interface IFoo
{
    int getSum();
}

struct Pair<T:IFoo, U: IFoo> : IFoo
{
    T first;
    U second;
    int getSum() { return first.getSum() + second.getSum(); }
}

void m()
{
    Pair<IFoo, IFoo> v;

}