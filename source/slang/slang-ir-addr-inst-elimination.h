// slang-ir-addr-inst-elimination.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct SharedIRBuilder;
class DiagnosticSink;

struct AddressConversionPolicy
{
    virtual bool shouldConvertAddrInst(IRInst* addrInst) = 0;
};
SlangResult eliminateAddressInsts(
    SharedIRBuilder* sharedBuilder,
    AddressConversionPolicy* policy,
    IRFunc* func,
    DiagnosticSink* sink);

}
