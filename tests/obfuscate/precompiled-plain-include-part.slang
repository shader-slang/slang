//TEST_IGNORE_FILE:

implementing "precompiled-plain-include";

import "precompiled-plain-include-lib";

internal int addThree(int x)
{
    return addTwoPlainInclude(addOnePlainInclude(x));
}
