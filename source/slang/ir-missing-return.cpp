// ir-missing-return.cpp
#include "ir-missing-return.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang {

class DiagnosticSink;
struct IRModule;

void checkForMissingReturnsRec(
    IRInst*         inst,
    DiagnosticSink* sink)
{
    if( auto code = as<IRGlobalValueWithCode>(inst) )
    {
        for( auto block : code->getBlocks() )
        {
            auto terminator = block->getTerminator();

            if( auto missingReturn = as<IRMissingReturn>(terminator) )
            {
                sink->diagnose(missingReturn, Diagnostics::missingReturn);
            }
        }
    }

    for( auto childInst : inst->getDecorationsAndChildren() )
    {
        checkForMissingReturnsRec(childInst, sink);
    }
}

void checkForMissingReturns(
    IRModule*       module,
    DiagnosticSink* sink)
{
    // Look for any `missingReturn` instructions
    checkForMissingReturnsRec(module->getModuleInst(), sink);
}

}
