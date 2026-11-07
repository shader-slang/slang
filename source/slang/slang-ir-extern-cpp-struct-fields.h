// slang-ir-extern-cpp-struct-fields.h
#pragma once

namespace Slang
{
struct IRModule;

/// Give the fields of each `__extern_cpp` struct their source names as their C++ names.
///
/// Host C++ code that includes the generated header accesses the fields of an `__extern_cpp`
/// struct by their source names. Only the C++ and CUDA source targets run this pass: GPU backends
/// reserve different keywords, and Metal and WGSL flatten stage inputs into one struct with fresh
/// keys (Metal copies the name hint and debug decorations, WGSL moves every key decoration), so
/// those targets keep generated, uniqued field names.
///
/// The pass names every field of such a struct that has a name hint, so those names are distinct
/// from each other. A field keeps its source name unless the checker synthesized it (such as
/// bitfield backing storage) or another field shares the name (the field that holds an inherited
/// struct is named `base`); those fields get the name plus the smallest unused number, such as
/// `base_0`. Under `-obfuscate`, which strips name hints, fields keep their hashed names.
void applyExternCppStructFieldNames(IRModule* module);

} // namespace Slang
