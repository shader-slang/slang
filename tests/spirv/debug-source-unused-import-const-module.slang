// Non-test helper module for debug-source-unused-import.slang (no //TEST: directive).
// The consuming entry point reads only this constant, whose value is folded into the use site, so
// the linked code reaches none of this module's debug records, and the linker leaves its file list
// out of the output.
module "debug-source-unused-import-const-module";

public static const int constModuleMarker13982 = 7;
