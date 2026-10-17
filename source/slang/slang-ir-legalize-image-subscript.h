#pragma once

#include "slang-compiler.h"
#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

/// Rewrite every `store` or `swizzledStore` whose address is rooted at an `imageSubscript`, as in
/// `tex[i] = v`, `tex[i].w = v` or `tex[i].xy = v`, into a `kIROp_ImageStore` of the whole texel.
/// A store that writes only some components first reads the texel with a `kIROp_ImageLoad`; a
/// dynamic component index, as in `tex[i][k] = v`, becomes a select per component.
///
/// The texel type of these image ops is a 4-component vector on Metal, GLSL and SPIR-V, whose image
/// APIs always use four components, and the texture's own element type on CUDA, where
/// `surf*read<T>`/`surf*write<T>` move exactly `sizeof(T)` bytes. On CUDA we also report the
/// accesses the CUDA prelude cannot express, and warn about each read-modify-write.
///
/// Reads through an `imageSubscript`, as in `tex[i].w += v` or an `inout` argument, are left in
/// place (shader-slang/slang#13362).
void legalizeImageSubscript(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
