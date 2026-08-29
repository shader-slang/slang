#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

struct MatrixTypeLoweringOptions
{
    // Lowers every matrix to an array of row vectors, including floating-point matrices that the
    // source-oriented target would otherwise preserve.
    bool lowerAllMatrixTypes = false;
};

// Lowers matrix types to arrays for targets and emission paths that need structural values.
void legalizeMatrixTypes(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink,
    MatrixTypeLoweringOptions options);

} // namespace Slang
