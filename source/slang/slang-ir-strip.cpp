// slang-ir-strip.cpp
#include "slang-ir-strip.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang {

    /// Should `inst` be stripped, given the current `options`?
static bool _shouldStripInst(
    IRInst*                 inst,
    IRStripOptions const&   options)
{
    switch( inst->getOp() )
    {
    default:
        return false;

    case kIROp_HighLevelDeclDecoration:
        return true;

    case kIROp_NameHintDecoration:
        return options.shouldStripNameHints;
    }
}

    /// Recursively strip `inst` and its children according to `options`.
static void _stripFrontEndOnlyInstructionsRec(
    IRInst*                 inst,
    IRStripOptions const&   options)
{
    if( _shouldStripInst(inst, options) )
    {
        inst->removeAndDeallocate();
        return;
    }

    if (options.stripSourceLocs)
    {
        inst->sourceLoc = SourceLoc();
    }

    IRInst* nextChild = nullptr;
    for( IRInst* child = inst->getFirstDecorationOrChild(); child; child = nextChild )
    {
        nextChild = child->getNextInst();

        _stripFrontEndOnlyInstructionsRec(child, options);
    }
}

void stripFrontEndOnlyInstructions(
    IRModule*               module,
    IRStripOptions const&   options)
{
    _stripFrontEndOnlyInstructionsRec(module->getModuleInst(), options);
}

}
