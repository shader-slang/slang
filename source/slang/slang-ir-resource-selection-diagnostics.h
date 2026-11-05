#pragma once

namespace Slang
{
struct CodeGenContext;
struct IRInst;

/// Warn when conditional execution of a resource operation may change its behavior.
///
/// Requires `originalConsumer` to be an operation that resource-selection specialization will
/// clone into branches, after any resource-producing operations have been followed to their uses.
/// The caller invokes this procedure once per original operation, before cloning it. The procedure
/// emits no warning for operations known to be independent of invocation participation, and does
/// not mutate the IR or reject the planned transformation.
void diagnoseResourceSelectionDispatch(CodeGenContext* codeGenContext, IRInst* originalConsumer);

} // namespace Slang
