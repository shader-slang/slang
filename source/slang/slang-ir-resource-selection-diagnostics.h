#pragma once

namespace Slang
{
struct CodeGenContext;
struct IRInst;

/// Warn if branching around `originalConsumer` may change its behavior under conditional execution.
///
/// `specializeResourceSelections` calls this once before copying an operation into branches for
/// conditionally chosen resources. `originalConsumer` must use resources without computing another
/// resource value; for example, a sample consumes a texture and sampler and returns a color.
/// Resource-producing operations, such as indexing an array of textures, must first be followed to
/// the operations that use their results. We emit no warning for operations known to be independent
/// of invocation participation. This procedure does not mutate the IR or reject the transformation.
void diagnoseResourceSelectionDispatch(CodeGenContext* codeGenContext, IRInst* originalConsumer);

} // namespace Slang
