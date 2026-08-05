//TEST:INTERPRET(filecheck=CHECK):

struct VectorizeTo<T, let N : int>
{
}

extension<let N : int, T : __BuiltinIntegerType> VectorizeTo<vector<T, N>, N>
{
    typealias Result = vector<T, N>;
}

extension<T : __BuiltinIntegerType> VectorizeTo<T, 1>
{
    typealias Result = T;
}

void main()
{
    VectorizeTo<uint, 1>.Result scalarValue = 7u;
    VectorizeTo<vector<uint, 4>, 4>.Result vectorValue = uint4(1, 2, 3, 4);

    // CHECK: 7
    printf("%u\n", scalarValue);

    // CHECK: 10
    printf("%u\n", vectorValue.x + vectorValue.y + vectorValue.z + vectorValue.w);
}
