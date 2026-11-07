//TEST_IGNORE_FILE:

// Helper for global-varying-include.slang. A module-scope varying in an `__include`d file is at the
// top level of its module.
implementing "global-varying-include";

in float4 gIncluded;
