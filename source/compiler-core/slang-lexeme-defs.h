// This multi-include file exposes the generated lexeme hierarchy to its clients.
// The generated output is checked in because the lexer is also used to build fiddle.
#if 0 // FIDDLE TEMPLATE:
$(require("source/compiler-core/slang-lexeme-defs.lua").generate())
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-lexeme-defs.h.fiddle"
#endif // FIDDLE END
