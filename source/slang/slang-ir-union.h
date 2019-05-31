// slang-ir-union.h
#pragma once

namespace Slang {

struct IRModule;

    /// Desugar any unions types, and code using them, in `module`
    ///
    /// Union types will be replaced with ordinary `struct` types that store
    /// the data of the underlying type as a "bag of bits" and references
    /// to cases of the union will be replaced with logic to extract the
    /// relevant bits.
    ///
void desugarUnionTypes(
    IRModule*       module);

} // namespace Slang
