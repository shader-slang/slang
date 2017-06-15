#ifndef SLANG_PROFILE_H_INCLUDED
#define SLANG_PROFILE_H_INCLUDED

#include "../core/basic.h"
#include "../../slang.h"

namespace Slang
{
    // Flavors of translation unit
    enum class SourceLanguage : SlangSourceLanguage
    {
        Unknown = SLANG_SOURCE_LANGUAGE_UNKNOWN, // should not occur
        Slang = SLANG_SOURCE_LANGUAGE_SLANG,
        HLSL = SLANG_SOURCE_LANGUAGE_HLSL,
        GLSL = SLANG_SOURCE_LANGUAGE_GLSL,

        // A separate PACKAGE of Slang code that has been imported
        ImportedSlangCode,
    };

    // TODO(tfoley): This should merge with the above...
    enum class Language
    {
        Unknown,
#define LANGUAGE(TAG, NAME) TAG,
#include "profile-defs.h"
    };

    enum class ProfileFamily
    {
        Unknown,
#define PROFILE_FAMILY(TAG) TAG,
#include "profile-defs.h"
    };

    enum class ProfileVersion
    {
        Unknown,
#define PROFILE_VERSION(TAG, FAMILY) TAG,
#include "profile-defs.h"
    };

    enum class Stage : SlangStage
    {
        Unknown = SLANG_STAGE_NONE,
#define PROFILE_STAGE(TAG, NAME, VAL) TAG = VAL,
#include "profile-defs.h"
    };

    ProfileFamily getProfileFamily(ProfileVersion version);

    struct Profile
    {
        typedef uint32_t RawVal;
        enum : RawVal
        {
        Unknown,

#define PROFILE(TAG, NAME, STAGE, VERSION) TAG = (uint32_t(Stage::STAGE) << 16) | uint32_t(ProfileVersion::VERSION),
#include "profile-defs.h"
        };

        Profile() {}
        Profile(RawVal raw)
            : raw(raw)
        {}

        bool operator==(Profile const& other) const { return raw == other.raw; }
        bool operator!=(Profile const& other) const { return raw != other.raw; }

        Stage GetStage() const { return Stage((uint32_t(raw) >> 16) & 0xFFFF); }
        ProfileVersion GetVersion() const { return ProfileVersion(uint32_t(raw) & 0xFFFF); }
        ProfileFamily getFamily() const { return getProfileFamily(GetVersion()); }

        static Profile LookUp(char const* name);

        RawVal raw = Unknown;
    };
}

#endif
