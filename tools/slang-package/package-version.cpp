// package-version.cpp

#include "package-version.h"

#include "core/slang-string-util.h"

namespace Slang
{
namespace PackageTool
{

static SlangResult _parsePackageVersionComponent(const UnownedStringSlice& text, uint32_t& outValue)
{
    if (text.getLength() == 0)
        return SLANG_FAIL;
    uint64_t value = 0;
    for (auto character : text)
    {
        if (character < '0' || character > '9')
            return SLANG_FAIL;
        value = value * 10u + uint64_t(character - '0');
        if (value > 0xffffffffu)
            return SLANG_FAIL;
    }
    outValue = uint32_t(value);
    return SLANG_OK;
}

/// Drop trailing zero components, leaving at least one. `1.2.0.0` becomes `1.2`, and `0.0.0`
/// becomes `0`. A zero before a non-zero component stays, so `1.0.1` is unchanged.
static void _removeTrailingZeros(List<uint32_t>& components)
{
    while (components.getCount() > 1 && components.getLast() == 0)
        components.removeLast();
}

SlangResult PackageVersion::parsePreservingTrailingZeros(
    const UnownedStringSlice& text,
    PackageVersion& outVersion)
{
    List<UnownedStringSlice> parts;
    StringUtil::split(text, '.', parts);
    if (parts.getCount() < 1 || parts.getCount() > kMaxComponentCount)
        return SLANG_FAIL;

    PackageVersion parsed;
    for (auto part : parts)
    {
        uint32_t component = 0;
        if (SLANG_FAILED(_parsePackageVersionComponent(part, component)))
            return SLANG_FAIL;
        parsed.components.add(component);
    }
    outVersion = parsed;
    return SLANG_OK;
}

PackageVersion::PackageVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    components.add(major);
    components.add(minor);
    components.add(patch);
    _removeTrailingZeros(components);
}

SlangResult PackageVersion::parse(const UnownedStringSlice& text, PackageVersion& outVersion)
{
    SLANG_RETURN_ON_FAIL(parsePreservingTrailingZeros(text, outVersion));
    _removeTrailingZeros(outVersion.components);
    return SLANG_OK;
}

String PackageVersion::format() const
{
    StringBuilder builder;
    for (Index i = 0; i < components.getCount(); ++i)
    {
        if (i != 0)
            builder << ".";
        builder << components[i];
    }
    return builder.produceString();
}

int PackageVersion::compare(const PackageVersion& other) const
{
    Index shared = components.getCount();
    if (other.components.getCount() < shared)
        shared = other.components.getCount();
    for (Index i = 0; i < shared; ++i)
    {
        if (components[i] < other.components[i])
            return -1;
        if (components[i] > other.components[i])
            return 1;
    }
    if (components.getCount() < other.components.getCount())
        return -1;
    if (components.getCount() > other.components.getCount())
        return 1;
    return 0;
}

} // namespace PackageTool
} // namespace Slang
