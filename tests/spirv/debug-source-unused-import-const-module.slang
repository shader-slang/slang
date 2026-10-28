// Non-test helper module for debug-source-unused-import.slang (no //TEST: directive).
// The consuming entry point reads only this constant, whose value is folded into the use site, so
// no linked debug record refers to this file and none of its debug records belong in the output.
module "debug-source-unused-import-const-module";

public static const int constModuleMarker13982 = 7;
