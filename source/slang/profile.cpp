// profile.cpp
#include "Profile.h"


namespace Slang {
namespace Compiler {


ProfileFamily getProfileFamily(ProfileVersion version)
{
    switch( version )
    {
    default: return ProfileFamily::Unknown;

#define PROFILE_VERSION(TAG, FAMILY) case ProfileVersion::TAG: return ProfileFamily::FAMILY;
#include "profile-defs.h"
    }
}

}}
