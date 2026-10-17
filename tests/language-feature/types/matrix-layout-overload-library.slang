// Library half of matrix-layout-overload-module.slang. It is precompiled to a .slang-module by that
// test's COMPILE step, so its two `pick` overloads, which differ only in matrix layout, are found
// by mangled name when the consumer links against the serialized module.
module matrix_layout_overload_library;

public func pick(m : row_major float2x3[2]) -> float { return 1.0; }
public func pick(m : column_major float2x3[2]) -> float { return 2.0; }
