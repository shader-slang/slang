#ifndef SLANG_PROFILE_H_INCLUDED
#define SLANG_PROFILE_H_INCLUDED

#include "../core/slang-basic.h"
#include "../../slang.h"

namespace Slang
{
    // Flavors of translation unit
    enum class SourceLanguage : SlangSourceLanguageIntegral
    {
        Unknown = SLANG_SOURCE_LANGUAGE_UNKNOWN, // should not occur
        Slang = SLANG_SOURCE_LANGUAGE_SLANG,
        HLSL = SLANG_SOURCE_LANGUAGE_HLSL,
        GLSL = SLANG_SOURCE_LANGUAGE_GLSL,
        C = SLANG_SOURCE_LANGUAGE_C,
        CPP = SLANG_SOURCE_LANGUAGE_CPP,
        CUDA = SLANG_SOURCE_LANGUAGE_CUDA,
        CountOf = SLANG_SOURCE_LANGUAGE_COUNT_OF,
    };

    // TODO(tfoley): This should merge with the above...
    enum class Language
    {
        Unknown,
#define LANGUAGE(TAG, NAME) TAG,
#include "slang-profile-defs.h"
    };

    enum class ProfileFamily
    {
        Unknown,
#define PROFILE_FAMILY(TAG) TAG,
#include "slang-profile-defs.h"
    };

    enum class ProfileVersion
    {
        Unknown,
#define PROFILE_VERSION(TAG, FAMILY) TAG,
#include "slang-profile-defs.h"
    };

    enum class Stage : SlangStage
    {
        Unknown = SLANG_STAGE_NONE,
#define PROFILE_STAGE(TAG, NAME, VAL) TAG = VAL,
#define PROFILE_STAGE_ALIAS(TAG, NAME, VAL) TAG = VAL,
#include "slang-profile-defs.h"
    };

    const char* getStageName(Stage stage);

    ProfileFamily getProfileFamily(ProfileVersion version);

    struct Profile
    {
        typedef uint32_t RawVal;
        enum RawEnum : RawVal
        {
        Unknown,

#define PROFILE(TAG, NAME, STAGE, VERSION) TAG = (uint32_t(ProfileVersion::VERSION) << 16) | uint32_t(Stage::STAGE),
#define PROFILE_ALIAS(TAG, DEF, NAME) TAG = DEF,
#include "slang-profile-defs.h"
        };

        Profile() {}
        Profile(RawEnum raw)
            : raw(raw)
        {}
        explicit Profile(RawVal raw)
            : raw(raw)
        {}
        explicit Profile(Stage stage)
        {
            setStage(stage);
        }
        explicit Profile(ProfileVersion version)
        {
            setVersion(version);
        }

        bool operator==(Profile const& other) const { return raw == other.raw; }
        bool operator!=(Profile const& other) const { return raw != other.raw; }

        Stage GetStage() const { return Stage(uint32_t(raw) & 0xFFFF); }
        void setStage(Stage stage)
        {
            raw = (raw & ~0xFFFF) | uint32_t(stage);
        }

        ProfileVersion GetVersion() const  { return ProfileVersion((uint32_t(raw) >> 16) & 0xFFFF); }
        void setVersion(ProfileVersion version)
        {
            raw = (raw & 0x0000FFFF) | (uint32_t(version) << 16);
        }

        ProfileFamily getFamily() const { return getProfileFamily(GetVersion()); }

        static Profile LookUp(char const* name);
        char const* getName();

        RawVal raw = Unknown;
    };



    Stage findStageByName(String const& name);

    UnownedStringSlice getStageText(Stage stage);


    // An enum to specify SPIR-V versions.
    // For the moment they are only differentiated by version number, but it may be necessary to
    // differentiate specific versions in other ways (such as the VK1.1 / SPIR-V combination in
    // glslang). The enum is used to encode the representation, and vary as is needed in
    // an implementation. 
    enum class SPIRVVersion : uint32_t;

    SLANG_INLINE SPIRVVersion makeSPIRVVersion(int major, int minor) { return SPIRVVersion((uint32_t(major) << 8) | uint32_t(minor)); }
    SLANG_FORCE_INLINE Index getMajorVersion(SPIRVVersion version) { return Index((uint32_t(version) >> 8) & 0xff); }
    SLANG_FORCE_INLINE Index getMinorVersion(SPIRVVersion version) { return Index(uint32_t(version) & 0xff); }
    SLANG_FORCE_INLINE int32_t asInteger(SPIRVVersion version) { return int32_t(version); }
}

#endif
