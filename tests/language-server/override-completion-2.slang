//TEST:LANG_SERVER(filecheck=CHECK):
interface IBar{ associatedtype AT; }
interface IFoo
{
    associatedtype B : IBar;
//HOVER:7,12
    static B.AT execute(int x);
}

struct Impl : IFoo
{
//COMPLETE:13,14
    override 
}

//CHECK: associatedtype This.B
//CHECK: static B.AT execute(int x)