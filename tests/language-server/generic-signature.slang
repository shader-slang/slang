//TEST:LANG_SERVER(filecheck=CHECK):
struct Parent<V>
{
    static void test<T, int size>(vector<T, size> v, int m)
    {}
    static void test(){}
}

void use()
{
//SIGNATURE:12,14
    Parent<int>.test(1, 2);
}

//CHECK: struct Parent<V>