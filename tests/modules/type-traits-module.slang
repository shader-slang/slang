struct TypeTrait< T : __BuiltinFloatingPointType, int X, int Y, int Z>
{
    static const int M = X;
    static const int N = Y;
    static const int K = Z;
    static const int ElementCountPerVector = sizeof(uint4) / sizeof(T);
    static const int Uint4AlignedK = ((K + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;
}
