// slang-ir-address-analysis.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-dominators.h"

namespace Slang
{
    struct AddressInfo : public RefObject
    {
        IRInst* addrInst = nullptr;
        AddressInfo* parentAddress = nullptr;
        bool isConstant = false;
        List<AddressInfo*> children;
    };

    struct AddressAccessInfo
    {
        OrderedDictionary<IRInst*, RefPtr<AddressInfo>> addressInfos;
    };

    // Gather info on all addresses used by `func`.
    AddressAccessInfo analyzeAddressUse(IRDominatorTree* domTree, IRGlobalValueWithCode* func);
}
