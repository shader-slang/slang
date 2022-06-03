// slang-ir-com-interface.cpp
#include "slang-ir-com-interface.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct ComInterfaceLoweringContext
{
    void processModule(IRModule* module, ArtifactStyle artifactStyle, DiagnosticSink* sink)
    {
        m_diagnosticSink = sink;
      
        for (auto child : module->getGlobalInsts())
        {
            switch (child->getOp())
            {
                case kIROp_InterfaceType:
                {
                    _addInterface(static_cast<IRInterfaceType*>(child));
                    break;
                }
                case kIROp_ThisType:
                {
                    // TODO(JS): 
                    // Not clear why this extra path is needed for ThisType. 
                    // Surely it would be found, by just finding interfaces in global scope.
                    // Left in to keep behavior the same as previously
                    auto thisType = static_cast<IRThisType*>(child);
                    _addInterface(as<IRInterfaceType>(thisType->getConstraintType()));
                    break;
                }
                default: break;
            }
        }

        // For all interfaces found replace uses
        {
            SharedIRBuilder sharedBuilder;
            sharedBuilder.init(module);

            IRBuilder builder(sharedBuilder);

            builder.setInsertInto(module->getModuleInst());

            List<IRUse*> uses;

            for (auto comIntf : m_comInterfaces)
            {   
                uses.clear();

                // Find all of the uses *before* doing any replacement
                // Otherwise we end up replacing the replacement type leading 
                // to it pointing to itself.
                for (auto use = comIntf->firstUse; use; use = use->nextUse)
                {
                    uses.add(use);
                }

                // TODO(JS): This is a temporary fix, in that whether kernel or not 
                // shouldn't control the ptr type in general
                // It's necessary here though because Kernel doesn't have ComPtr<>
                // so has to be a naked pointer
                IRType* result = (artifactStyle == ArtifactStyle::Host) ?
                    static_cast<IRType*>(builder.getComPtrType(comIntf)) :
                    static_cast<IRType*>(builder.getPtrType(comIntf));

                // Go through uses for the interface
                for (auto use : uses)
                {
                    // Can we do a replacement in this use scenario?
                    if (_canReplace(use))
                    {
                        // Do the replacement
                        use->set(result);
                    }
                }
            }
        }
    }

protected:
    void _addInterface(IRInterfaceType* intf)
    {
        if (intf && intf->findDecoration<IRComInterfaceDecoration>())
        {
            m_comInterfaces.Add(intf);
        }
    }

    static bool _canReplace(IRUse* use)
    {
        switch (use->getUser()->getOp())
        {
        case kIROp_WitnessTableIDType:
        case kIROp_WitnessTableType:
        case kIROp_RTTIPointerType:
        case kIROp_RTTIHandleType:
        {
            // Don't replace
            return false;
        }
        case kIROp_ThisType:
        case kIROp_ComPtrType:
        case kIROp_PtrType:
        {
            // We can have ** and ComPtr<T>*.
            // If it's a pointer type it could be because it is a global.
            break;
        }
        default:
            break;
        }
        return true;
    }

    DiagnosticSink* m_diagnosticSink;
    HashSet<IRInterfaceType*> m_comInterfaces;          ///< All of the unique found COM interfaces
};

void lowerComInterfaces(IRModule* module, ArtifactStyle artifactStyle, DiagnosticSink* sink)
{
    ComInterfaceLoweringContext context;
    return context.processModule(module, artifactStyle, sink);
}

}
