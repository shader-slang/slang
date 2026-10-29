implementing portable;

// Generated from the compiler's KnownBuiltinDeclName values. These annotations identify library
// declarations that the compiler uses when preparing types for backend code generation. For
// example, Context leads to the Payload and Record types needed by a generated stage function.
// Looking up those requirements by name would also work, but would tie the compiler to their
// spelling. Keeping the StageContext annotation when renaming Context to ShaderContext lets the
// compiler keep the same lookup; shader code using the old name would still need updating.
#include "raytracing-builtins.slang"
