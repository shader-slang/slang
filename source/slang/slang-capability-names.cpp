// slang-capability-names.cpp
//
// Lightweight implementation of getCapabilityNames(), extracted from
// slang-capability.cpp so it can be compiled without heavy compiler
// dependencies (e.g. slang-ast-builder.h).

#include "slang-capability.h"

#include <stdint.h>

namespace Slang
{

namespace
{ // anonymous – avoids ODR conflict with the same types in slang-capability.cpp

enum class CapabilityNameFlavor : int32_t
{
    Concrete,
    Abstract,
    Alias,
};

struct CapabilityAtomInfo
{
    UnownedStringSlice name;
    CapabilityNameFlavor flavor;
    CapabilityName abstractBase;
    uint32_t rank;
    ArrayView<CapabilityAtomSet*> canonicalRepresentation;
};

#include "slang-generated-capability-defs-impl.h"

static CapabilityAtomInfo const& _getInfoByName(CapabilityName atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityName::Count));
    return kCapabilityNameInfos[Int(atom)];
}

} // anonymous namespace

void getCapabilityNames(List<UnownedStringSlice>& ioNames)
{
    ioNames.reserve(Count(CapabilityName::Count));
    for (Index i = 0; i < Count(CapabilityName::Count); ++i)
    {
        if (_getInfoByName(CapabilityName(i)).flavor != CapabilityNameFlavor::Abstract)
        {
            ioNames.add(_getInfoByName(CapabilityName(i)).name);
        }
    }
}

} // namespace Slang
