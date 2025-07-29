// slang-profile.cpp
#include "slang-profile.h"

namespace Slang
{

Profile Profile::lookUp(UnownedStringSlice const& name)
{
#define PROFILE(TAG, NAME, STAGE, VERSION)           \
    if (name == UnownedTerminatedStringSlice(#NAME)) \
        return Profile::TAG;
#define PROFILE_ALIAS(TAG, DEF, NAME)                \
    if (name == UnownedTerminatedStringSlice(#NAME)) \
        return Profile::TAG;
#include "slang-profile-defs.h"

    return Profile::Unknown;
}

Profile Profile::lookUp(char const* name)
{
    return lookUp(UnownedStringSlice(name));
}

CapabilitySet Profile::getCapabilityName()
{
    List<CapabilityName> result;
    switch (getVersion())
    {
#define PROFILE_VERSION(TAG, NAME)       \
    case ProfileVersion::TAG:            \
        result.add(CapabilityName::TAG); \
        break;
#include "slang-profile-defs.h"
    default:
        break;
    }
    switch (getStage())
    {
#define PROFILE_STAGE(TAG, NAME, VAL)     \
    case Stage::TAG:                      \
        result.add(CapabilityName::NAME); \
        break;
#include "slang-profile-defs.h"
    default:
        break;
    }

    CapabilitySet resultSet = CapabilitySet(result);
    for (auto i : this->additionalCapabilities)
        resultSet.join(i);
    return resultSet;
}

char const* Profile::getName()
{
    switch (raw)
    {
    default:
        return "unknown";

#define PROFILE(TAG, NAME, STAGE, VERSION) \
    case Profile::TAG:                     \
        return #NAME;
#define PROFILE_ALIAS(TAG, DEF, NAME) /* empty */
#include "slang-profile-defs.h"
    }
}

static const StageInfo kStages[] = {
#define PROFILE_STAGE(ID, NAME, ENUM) {#NAME, Stage::ID},

#define PROFILE_STAGE_ALIAS(ID, NAME, VAL) {#NAME, Stage::ID},

#include "slang-profile-defs.h"
};

ConstArrayView<StageInfo> getStageInfos()
{
    return makeConstArrayView(kStages);
}

Stage findStageByName(String const& name)
{
    for (auto entry : kStages)
    {
        if (name == entry.name)
        {
            return entry.stage;
        }
    }

    return Stage::Unknown;
}

UnownedStringSlice getStageText(Stage stage)
{
    for (auto entry : kStages)
    {
        if (stage == entry.stage)
        {
            return UnownedStringSlice(entry.name);
        }
    }
    return UnownedStringSlice();
}

Stage getStageFromAtom(CapabilityAtom atom)
{
    switch (atom)
    {
    case CapabilityAtom::vertex:
        return Stage::Vertex;
    case CapabilityAtom::hull:
        return Stage::Hull;
    case CapabilityAtom::domain:
        return Stage::Domain;
    case CapabilityAtom::geometry:
        return Stage::Geometry;
    case CapabilityAtom::fragment:
        return Stage::Fragment;
    case CapabilityAtom::compute:
        return Stage::Compute;
    case CapabilityAtom::_mesh:
        return Stage::Mesh;
    case CapabilityAtom::_amplification:
        return Stage::Amplification;
    case CapabilityAtom::_anyhit:
        return Stage::AnyHit;
    case CapabilityAtom::_closesthit:
        return Stage::ClosestHit;
    case CapabilityAtom::_intersection:
        return Stage::Intersection;
    case CapabilityAtom::_raygen:
        return Stage::RayGeneration;
    case CapabilityAtom::_miss:
        return Stage::Miss;
    case CapabilityAtom::_callable:
        return Stage::Callable;
    case CapabilityAtom::dispatch:
        return Stage::Dispatch;
    default:
        SLANG_UNEXPECTED("unknown stage atom");
        UNREACHABLE_RETURN(Stage::Unknown);
    }
}

CapabilityAtom getAtomFromStage(Stage stage)
{
    // Convert Slang::Stage to CapabilityAtom.
    // Note that capabilities do not share the same values as Slang::Stage
    // and must be explicitly converted.
    switch (stage)
    {
    case Stage::Compute:
        return CapabilityAtom::compute;
    case Stage::Vertex:
        return CapabilityAtom::vertex;
    case Stage::Fragment:
        return CapabilityAtom::fragment;
    case Stage::Geometry:
        return CapabilityAtom::geometry;
    case Stage::Hull:
        return CapabilityAtom::hull;
    case Stage::Domain:
        return CapabilityAtom::domain;
    case Stage::Mesh:
        return CapabilityAtom::_mesh;
    case Stage::Amplification:
        return CapabilityAtom::_amplification;
    case Stage::RayGeneration:
        return CapabilityAtom::_raygen;
    case Stage::AnyHit:
        return CapabilityAtom::_anyhit;
    case Stage::ClosestHit:
        return CapabilityAtom::_closesthit;
    case Stage::Miss:
        return CapabilityAtom::_miss;
    case Stage::Intersection:
        return CapabilityAtom::_intersection;
    case Stage::Callable:
        return CapabilityAtom::_callable;
    case Stage::Dispatch:
        return CapabilityAtom::dispatch;
    default:
        SLANG_UNEXPECTED("unknown stage");
        UNREACHABLE_RETURN(CapabilityAtom::Invalid);
    }
}

ProfileFamily getProfileFamily(ProfileVersion version)
{
    switch (version)
    {
    default:
        return ProfileFamily::Unknown;

#define PROFILE_VERSION(TAG, FAMILY) \
    case ProfileVersion::TAG:        \
        return ProfileFamily::FAMILY;
#include "slang-profile-defs.h"
    }
}

bool isRaytracingStage(Stage inStage)
{
    switch (inStage)
    {
    case Stage::RayGeneration:
    case Stage::Miss:
    case Stage::Intersection:
    case Stage::ClosestHit:
    case Stage::Callable:
    case Stage::AnyHit:
        return true;
    default:
        return false;
    }
}

const char* getStageName(Stage stage)
{
    switch (stage)
    {
#define PROFILE_STAGE(ID, NAME, ENUM) \
    case Stage::ID:                   \
        return #NAME;

#include "slang-profile-defs.h"

    default:
        return nullptr;
    }
}

void printDiagnosticArg(StringBuilder& sb, Stage val)
{
    sb << getStageName(val);
}

void printDiagnosticArg(StringBuilder& sb, ProfileVersion val)
{
    sb << Profile(val).getName();
}

String getHLSLProfileName(Profile profile)
{
    switch (profile.getFamily())
    {
    case ProfileFamily::DX:
        // Profile version is a DX one, so stick with it.
        break;

    default:
        // Profile is a non-DX profile family, so we need to try
        // to clobber it with something to get a default.
        //
        // TODO: This is a huge hack...
        profile.setVersion(ProfileVersion::DX_5_1);
        break;
    }

    char const* stagePrefix = nullptr;
    switch (profile.getStage())
    {
        // Note: All of the raytracing-related stages require
        // compiling for a `lib_*` profile, even when only a
        // single entry point is present.
        //
        // We also go ahead and use this target in any case
        // where we don't know the actual stage to compiel for,
        // as a fallback option.
        //
        // TODO: We also want to use this option when compiling
        // multiple entry points to a DXIL library.
        //
    default:
        stagePrefix = "lib";
        break;

        // The traditional rasterization pipeline and compute
        // shaders all have custom profile names that identify
        // both the stage and shader model, which need to be
        // used when compiling a single entry point.
        //
#define CASE(NAME, PREFIX)     \
    case Stage::NAME:          \
        stagePrefix = #PREFIX; \
        break
        CASE(Vertex, vs);
        CASE(Hull, hs);
        CASE(Domain, ds);
        CASE(Geometry, gs);
        CASE(Fragment, ps);
        CASE(Compute, cs);
        CASE(Amplification, as);
        CASE(Mesh, ms);
#undef CASE
    }

    char const* versionSuffix = nullptr;
    switch (profile.getVersion())
    {
#define CASE(TAG, SUFFIX)        \
    case ProfileVersion::TAG:    \
        versionSuffix = #SUFFIX; \
        break
        CASE(DX_4_0, _4_0);
        CASE(DX_4_1, _4_1);
        CASE(DX_5_0, _5_0);
        CASE(DX_5_1, _5_1);
        CASE(DX_6_0, _6_0);
        CASE(DX_6_1, _6_1);
        CASE(DX_6_2, _6_2);
        CASE(DX_6_3, _6_3);
        CASE(DX_6_4, _6_4);
        CASE(DX_6_5, _6_5);
        CASE(DX_6_6, _6_6);
        CASE(DX_6_7, _6_7);
        CASE(DX_6_8, _6_8);
        CASE(DX_6_9, _6_9);
#undef CASE

    default:
        return "unknown";
    }

    String result;
    result.append(stagePrefix);
    result.append(versionSuffix);
    return result;
}


} // namespace Slang
