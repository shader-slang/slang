//TEST:LANG_SERVER(filecheck=CHECK):
interface IFoo
{
    int getSum();
}

void test(IFoo f)
{
//HOVER:10,9
    f.getSum();
}

//CHECK: IFoo.getSum
