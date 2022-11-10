#include "slang-ir-legalize-mesh-outputs.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"

namespace Slang
{

void legalizeMeshOutputTypes(IRModule* module)
{
    SharedIRBuilder builderStorage;
    builderStorage.init(module);
    IRBuilder builder(&builderStorage);

    for (auto inst : module->getGlobalInsts())
    {
        if (auto meshOutput = as<IRMeshOutputType>(inst))
        {
            auto elemType = meshOutput->getElementType();
            auto maxCount = meshOutput->getMaxElementCount();
            auto arrayType = builder.getArrayType(elemType, maxCount);
            meshOutput->replaceUsesWith(arrayType);
        }
    }
}

}
