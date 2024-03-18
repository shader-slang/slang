#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir.h"
#include "slang-ir-util.h"

namespace Slang
{
    void checkUnsupportedInst(IRModule* module, DiagnosticSink* sink)
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            switch (globalInst->getOp())
            {
            case kIROp_VectorType:
            case kIROp_MatrixType:
                {
                    if (!as<IRBasicType>(globalInst->getOperand(0)))
                    {
                        sink->diagnose(findFirstUseLoc(globalInst), Diagnostics::unsupportedBuiltinType, globalInst);
                    }
                    break;
                }
            default:
                break;
            }
        }
    }

}
