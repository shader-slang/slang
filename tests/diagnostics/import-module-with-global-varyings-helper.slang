//TEST_IGNORE_FILE:

// Helper module for import-module-with-global-varyings.slang. It declares a module-scope varying.
// It stays legacy Slang, with no `module` declaration and no `public`, because either would make it
// Slang 2025 and reject the varying in the helper itself.

in float4 gHelperColor;
