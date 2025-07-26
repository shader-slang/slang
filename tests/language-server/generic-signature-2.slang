//TEST:LANG_SERVER(filecheck=CHECK):
struct Parent<V>
{
    static void test<T, int size>(vector<T, size> v, int m)
    {}
    static void test(){}
}

void use()
{
//SIGNATURE:12,22
    Parent<int>.test<
}

//CHECK:static func Parent<int>.test<T, int size> -> void