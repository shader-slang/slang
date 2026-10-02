#pragma once

namespace Slang
{

struct IRModule;

/// Expand every array-to-array `BuiltinCast` into a conversion of each element.
///
/// The front end produces such a cast when an array of matrices converts to an array whose
/// matrices have another layout. `lowerLValueCast` produces one to copy an `inout`/`out` argument
/// back after the call, and for `inout` a second one to copy it in. No backend can emit the cast
/// directly. We run this pass after
/// `specializeMatrixLayout`, which turns a cast between layouts that resolve to the same layout
/// into an identity that simplification removes, and after the buffer-load specialization passes,
/// which see through the cast to keep reading the original buffer.
///
/// An array whose length is a small literal becomes a `makeArray` of converted elements. A longer
/// array, or one whose length is not a literal (for example a specialization constant), is
/// converted in a loop into a temporary, which keeps the code size independent of the length.
/// A cast directly in module scope has no control flow for a loop, so it is always unrolled.
void lowerArrayBuiltinCasts(IRModule* module);

} // namespace Slang
