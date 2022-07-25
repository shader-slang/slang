// slang-ir-strip-witness-tables.cpp
#include "slang-ir-strip-witness-tables.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

void stripWitnessTables(IRModule* module)
{
    // Our goal here is to empty out any witness tables in
    // the IR so that they don't keep other symbols alive
    // further into compilation. Luckily we expect all
    // witness tables to live directly at the global scope
    // (or inside of a generic, which we can ignore for
    // now because the emit logic also ignores generics),
    // and there is a single function we can call to
    // remove all of the content from the witness tables
    // (since the key-value associations are stored as
    // children of each table).

    for( auto inst : module->getGlobalInsts() )
    {
        auto witnessTable = as<IRWitnessTable>(inst);
        if(!witnessTable)
            continue;
        auto conformanceType = witnessTable->getConformanceType();
        if (conformanceType && conformanceType->findDecoration<IRComInterfaceDecoration>())
            continue;

        witnessTable->removeAndDeallocateAllDecorationsAndChildren();
    }
}

}
