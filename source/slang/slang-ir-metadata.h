// slang-ir-metadata.h
#pragma once

namespace Slang
{

class ArtifactPostEmitMetadata;
class DiagnosticSink;
struct IRModule;

// Walks `irModule` and records metadata for the target-visible cooperative
// matrix/vector types and combinations. For each cooperative operation, the
// collector validates operands inline and emits a diagnostic on `sink` for
// malformed input; offending instructions are skipped rather than recorded.
void collectCooperativeMetadata(
    const IRModule* irModule,
    DiagnosticSink* sink,
    ArtifactPostEmitMetadata& outMetadata);

void collectMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata);

} // namespace Slang
