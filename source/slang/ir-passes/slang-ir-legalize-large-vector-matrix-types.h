#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// The largest vector element count, and matrix row or column count, that the compiler accepts.
/// Sizes up to `kMaxNativeVectorElementCount` map to native target types; larger sizes up to this
/// bound are lowered to arrays by `legalizeLargeVectorMatrixTypes`.
const int kMaxVectorElementCount = 16;

/// The largest vector element count (and matrix row/column count) that target languages have a
/// native type for.
const int kMaxNativeVectorElementCount = 4;

/// Lower vectors with more than 4 elements and matrices with more than 4 rows or columns
/// (up to `kMaxVectorElementCount`) to arrays, on every target:
///
/// - `vector<T,N>` becomes `T[N]`.
/// - `matrix<T,R,C>` becomes an array of `R` rows, each row being `vector<T,C>` when `C <= 4`,
///   or `T[C]` otherwise. The rows are laid out one after another (row-major) whatever the matrix
///   layout mode.
///
/// Operations that are defined element-wise on vectors and matrices (arithmetic, comparisons,
/// casts, `select`, construction, swizzles and reshapes) are rewritten into operations on the
/// elements. Calls to target intrinsics whose signature still uses such a type, and uses as
/// shader stage inputs or outputs, are diagnosed.
void legalizeLargeVectorMatrixTypes(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
