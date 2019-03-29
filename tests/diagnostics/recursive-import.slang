//DIAGNOSTIC_TEST:SIMPLE:

// A file that recursively imports itself
// (including transitive cases) should be diagnosed.

import recursive_import_extra;
