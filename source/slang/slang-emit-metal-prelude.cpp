// slang-emit-metal-prelude.cpp
#include "slang-emit-metal.h"

namespace Slang
{

const char* MetalSourceEmitter::kMetalBuiltinPreludeMatrixCompMult = R"(
template<typename T, int A, int B>
matrix<T,A,B> _slang_matrixCompMult(matrix<T,A,B> m1, matrix<T,A,B> m2)
{
    matrix<T,A,B> result;
    for (int i = 0; i < A; i++)
        result[i] = m1[i] * m2[i];
    return result;
}
)";

const char* MetalSourceEmitter::kMetalBuiltinPreludeMatrixReshape = R"(
template<int A, int B, typename T, int N, int M>
matrix<T,A,B> _slang_matrixReshape(matrix<T,N,M> m)
{
    matrix<T,A,B> result = T(0);
    for (int i = 0; i < min(A,N); i++)
        for (int j = 0; j < min(B,M); j++)
            result[i] = m[i][j];
    return result;
}
)";

const char* MetalSourceEmitter::kMetalBuiltinPreludeVectorReshape = R"(
template<int A, typename T, int N>
vec<T,A> _slang_vectorReshape(vec<T,N> v)
{
    vec<T,A> result = T(0);
    for (int i = 0; i < min(A,N); i++)
        result[i] = v[i];
    return result;
}
)";

const char* MetalSourceEmitter::kMetalBuiltinPreludeMatrixFmod = R"(
template<typename T, int A, int B>
matrix<T,A,B> _slang_matrixFmod(matrix<T,A,B> m1, matrix<T,A,B> m2)
{
    matrix<T,A,B> result;
    for (int i = 0; i < A; i++)
        result[i] = fmod(m1[i], m2[i]);
    return result;
}
)";

const char* MetalSourceEmitter::kMetalBuiltinPreludeSimdgroupMatrixOps = R"(
#include <metal_simdgroup_matrix>
template<typename T, int Cols, int Rows, typename V>
void _slang_simdgroup_fill(thread simdgroup_matrix<T, Cols, Rows>* dest, V val) {
    *dest = make_filled_simdgroup_matrix<T, Cols, Rows>(T(val));
}
template<typename Matrix, typename T>
Matrix _slang_simdgroup_load(const device T* src, ulong elements_per_row) {
    Matrix result;
    simdgroup_load(result, src, elements_per_row);
    return result;
}
template<typename Matrix, typename T>
Matrix _slang_simdgroup_load_transpose(const device T* src, ulong elements_per_row) {
    Matrix result;
    simdgroup_load(result, src, elements_per_row, ulong2(0), true);
    return result;
}
template<typename Matrix, typename T>
Matrix _slang_simdgroup_load(const threadgroup T* src, ulong elements_per_row) {
    Matrix result;
    simdgroup_load(result, src, elements_per_row);
    return result;
}
template<typename Matrix, typename T>
Matrix _slang_simdgroup_load_transpose(const threadgroup T* src, ulong elements_per_row) {
    Matrix result;
    simdgroup_load(result, src, elements_per_row, ulong2(0), true);
    return result;
}
)";

} // namespace Slang
