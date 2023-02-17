// slang-ir-addr-inst-elimination.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

struct AddressConversionPolicy
{
    virtual bool shouldConvertAddrInst(IRInst* addrInst) = 0;
};
SlangResult eliminateAddressInsts(
    AddressConversionPolicy* policy,
    IRFunc* func,
    DiagnosticSink* sink);

}
