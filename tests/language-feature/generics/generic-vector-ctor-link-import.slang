// Imported helper module for generic-vector-ctor-link.slang.

[Differentiable]
vector<T, N> importedVectorCtor<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(0.0f);
}

vector<T, N> importedSiblingCtor<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(1.0f);
}
