#pragma once

namespace Slang
{
struct IRModule;

/// Replace each load of a `groupshared` variable whose value decides whether, or how many times,
/// a workgroup barrier runs with an `IRWorkgroupUniformLoad`.
///
/// WGSL's uniformity analysis treats every plain read of a `var<workgroup>` as possibly
/// non-uniform, so a barrier controlled by such a read is rejected even when one thread wrote the
/// value before an earlier barrier. Only `workgroupUniformLoad` produces a value that WGSL treats
/// as uniform.
void insertWorkgroupUniformLoadsForWGSL(IRModule* module);

} // namespace Slang
