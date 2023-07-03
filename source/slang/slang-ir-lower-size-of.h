#ifndef SLANG_IR_LOWER_SIZE_OF_H
#define SLANG_IR_LOWER_SIZE_OF_H

// This defines an IR pass that lowers sizeof/alignof. 

namespace Slang
{

struct IRModule;
class TargetRequest;
class DiagnosticSink;

void lowerSizeOfLike(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

} // namespace Slang

#endif
