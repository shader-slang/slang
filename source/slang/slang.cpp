#include "slang.h"

#include "../core/slang-archive-file-system.h"
#include "../core/slang-castable.h"
#include "../core/slang-io.h"
#include "../core/slang-performance-profiler.h"
#include "../core/slang-platform.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-string-util.h"
#include "../core/slang-string.h"
#include "../core/slang-type-convert-util.h"
#include "../core/slang-type-text-util.h"
// Artifact
#include "../compiler-core/slang-artifact-associated-impl.h"
#include "../compiler-core/slang-artifact-container-util.h"
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-artifact-impl.h"
#include "../compiler-core/slang-artifact-util.h"
#include "../compiler-core/slang-source-loc.h"
#include "../core/slang-file-system.h"
#include "../core/slang-memory-file-system.h"
#include "../core/slang-writer.h"
#include "core/slang-shared-library.h"
#include "slang-ast-dump.h"
#include "slang-check-impl.h"
#include "slang-check.h"
#include "slang-doc-ast.h"
#include "slang-doc-markdown-writer.h"
#include "slang-ir.h"
#include "slang-lookup.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-module-library.h"
#include "slang-options.h"
#include "slang-parameter-binding.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-reflection-json.h"
#include "slang-repro.h"
#include "slang-serialize-ast.h"
#include "slang-serialize-container.h"
#include "slang-serialize-ir.h"
#include "slang-tag-version.h"
#include "slang-type-layout.h"

#include <sys/stat.h>

// Used to print exception type names in internal-compiler-error messages
#include <typeinfo>

namespace Slang
{

const char* getBuildTagString()
{
    if (UnownedStringSlice(SLANG_TAG_VERSION) == "0.0.0-unknown")
    {
        // If the tag is unknown, then we will try to get the timestamp of the shared library
        // and use that as the version string, so that we can at least return something
        // that uniquely identifies the build.
        static String timeStampString =
            String(SharedLibraryUtils::getSharedLibraryTimestamp((void*)spCreateSession));
        return timeStampString.getBuffer();
    }
    return SLANG_TAG_VERSION;
}

Profile getEffectiveProfile(EntryPoint* entryPoint, TargetRequest* target)
{
    auto entryPointProfile = entryPoint->getProfile();
    auto targetProfile = target->getOptionSet().getProfile();

    // Depending on the target *format* we might have to restrict the
    // profile family to one that makes sense.
    //
    // TODO: Some of this should really be handled as validation at
    // the front-end. People shouldn't be allowed to ask for SPIR-V
    // output with Shader Model 5.0...
    switch (target->getTarget())
    {
    default:
        break;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        if (targetProfile.getFamily() != ProfileFamily::GLSL)
        {
            targetProfile.setVersion(ProfileVersion::GLSL_150);
        }
        break;

    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
        if (targetProfile.getFamily() != ProfileFamily::DX)
        {
            targetProfile.setVersion(ProfileVersion::DX_5_1);
        }
        break;
    case CodeGenTarget::Metal:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::MetalLibAssembly:
        if (targetProfile.getFamily() != ProfileFamily::METAL)
        {
            targetProfile.setVersion(ProfileVersion::METAL_2_3);
        }
        break;
    }

    auto entryPointProfileVersion = entryPointProfile.getVersion();
    auto targetProfileVersion = targetProfile.getVersion();

    // Default to the entry point profile, since we know that has the right stage.
    Profile effectiveProfile = entryPointProfile;

    // Ignore the input from the target profile if it is missing.
    if (targetProfile.getFamily() != ProfileFamily::Unknown)
    {
        // If the target comes from a different profile family, *or* it is from
        // the same family but has a greater version number, then use the target's version.
        if (targetProfile.getFamily() != entryPointProfile.getFamily() ||
            (targetProfileVersion > entryPointProfileVersion))
        {
            effectiveProfile.setVersion(targetProfileVersion);
        }
    }

    // Now consider the possibility that the chosen stage might force an "upgrade"
    // to the profile level.
    ProfileVersion stageMinVersion = ProfileVersion::Unknown;
    switch (effectiveProfile.getFamily())
    {
    case ProfileFamily::DX:
        switch (effectiveProfile.getStage())
        {
        default:
            break;

        case Stage::RayGeneration:
        case Stage::Intersection:
        case Stage::ClosestHit:
        case Stage::AnyHit:
        case Stage::Miss:
        case Stage::Callable:
            // The DirectX ray tracing stages implicitly
            // require Shader Model 6.3 or later.
            //
            stageMinVersion = ProfileVersion::DX_6_3;
            break;

            //  TODO: Add equivalent logic for geometry, tessellation, and compute stages.
        }
        break;

    case ProfileFamily::GLSL:
        switch (effectiveProfile.getStage())
        {
        default:
            break;

        case Stage::RayGeneration:
        case Stage::Intersection:
        case Stage::ClosestHit:
        case Stage::AnyHit:
        case Stage::Miss:
        case Stage::Callable:
            stageMinVersion = ProfileVersion::GLSL_460;
            break;

            //  TODO: Add equivalent logic for geometry, tessellation, and compute stages.
        }
        break;

    default:
        break;
    }

    if (stageMinVersion > effectiveProfile.getVersion())
    {
        effectiveProfile.setVersion(stageMinVersion);
    }

    return effectiveProfile;
}

} // namespace Slang
