// Intentionally placed in a subdirectory and never opened by the language server: it can only be
// found via the workspace-root recursive scan that populates the #include search path (see the
// shader-slang/slang#13179 regression in ../main.slang). No //TEST directive, so slang-test does
// not run this file directly.
static const int helperValue = 42;
