//TEST:LANG_SERVER(filecheck=CHECK):
struct Parent<V>
{
    static void test<T, int size>(vector<T, size> v, int m)
    {}
}

void use()
{
//SIGNATURE:11,22
    Parent<int>.test<float>
}

//CHECK:static func Parent<int>.test<T, int size> -> void