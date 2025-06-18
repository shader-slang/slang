//TEST:LANG_SERVER(filecheck=CHECK):
interface IBar{}
interface IFoo
{
    int eval();
    void execute<int z,T:IBar>(int x);
}

struct Impl : IFoo
{
//COMPLETE:12,14
    override 
}

//CHECK-DAG: int eval
//CHECK-DAG: void execute<int z, T>