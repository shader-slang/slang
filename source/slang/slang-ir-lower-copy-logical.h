#pragma once

namespace Slang
{

struct IRModule;

/// Lower copy-logical operations to simpler IR (element-wise field/element loads and stores).
///
/// When `onlyUntypedPtrOperand` is true, only `copyLogical` insts whose source or destination is an
/// untyped SPIR-V pointer (`IRSPIRVUntypedPtrType`) are lowered; the rest are left intact. This is
/// used on SPIR-V 1.4+, where `OpCopyLogical` is otherwise kept, because a whole-aggregate
/// load/store through an untyped descriptor-heap pointer is miscompiled by spirv-opt (see #13022):
/// lowering it to per-field untyped access chains keeps every access to that buffer element-wise.
/// (Below 1.4 `copyLogical` is already lowered unconditionally, so no separate call is needed.)
void lowerCopyLogical(IRModule* module, bool onlyUntypedPtrOperand = false);

} // namespace Slang
