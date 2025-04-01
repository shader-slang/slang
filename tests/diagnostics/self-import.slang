//TEST_IGNORE_FILE:

// A file that imports a module which spans two files, including itself.
// This file is intended to be compiled as part of the self-import-extra
// test and does nothing by itself.

import "self-import-extra";

struct S { }

extension S : I { }

void main()
{
}
