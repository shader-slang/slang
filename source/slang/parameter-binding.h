#ifndef SLANG_PARAMETER_BINDING_H
#define SLANG_PARAMETER_BINDING_H

#include "../core/basic.h"
#include "syntax.h"

#include "../../Slang.h"

namespace Slang {
namespace Compiler {

class CollectionOfTranslationUnits;

// The parameter-binding interface is responsible for assigning
// binding locations/registers to every parameter of a shader
// program. This can include both parameters declared on a
// particular entry point, as well as parameters declared at
// global scope.
//


// Generate binding information for the given program,
// represented as a collection of different translation units,
// and attach that information to the syntax nodes
// of the program.

void GenerateParameterBindings(
    CollectionOfTranslationUnits*   program);

}}

#endif // SLANG_REFLECTION_H
