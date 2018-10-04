// profile.cpp
#include "profile.h"

namespace Slang {

ProfileFamily getProfileFamily(ProfileVersion version)
{
    switch( version )
    {
    default: return ProfileFamily::Unknown;

#define PROFILE_VERSION(TAG, FAMILY) case ProfileVersion::TAG: return ProfileFamily::FAMILY;
#include "profile-defs.h"
    }
}

const char* getStageName(Stage stage)
{
    switch(stage)
    {
#define PROFILE_STAGE(ID, NAME, ENUM) \
    case Stage::ID: return #NAME;

#include "profile-defs.h"

    default:
        return nullptr;
    }

}



}
