// slang-ir-extern-cpp-struct-fields.h
#pragma once

namespace Slang
{
struct IRModule;

/// Give each field of an `__extern_cpp` struct the field's source name as its C++ name.
///
/// Host C++ code that includes the generated header accesses the fields of an `__extern_cpp`
/// struct by their source names, so the C++ and CUDA source targets run this pass. Other targets
/// keep the generated, uniqued field names.
void applyExternCppStructFieldNames(IRModule* module);

} // namespace Slang
