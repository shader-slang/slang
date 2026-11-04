//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std legacy -experimental-feature -ignore-capabilities -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2025 -experimental-feature -ignore-capabilities -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -experimental-feature -ignore-capabilities -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 202c -experimental-feature -ignore-capabilities -no-codegen

import slang.raytracing.portable;

typealias StageBox = dyn rt::IMissShader;

StageBox invalidGlobal;
/*CHECK:
         ^^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
         ^^^^^^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader' has no runtime representation
*/

void invalidParameter(StageBox invalidStage);
/*CHECK:
                               ^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
                               ^^^^^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader' has no runtime representation
*/

StageBox invalidResult();
/*CHECK:
^^^^^^^^ runtime use of a structural ray-tracing stage type
^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader' has no runtime representation
*/

// The synthesized memberwise constructor also has a parameter of the invalid field type.
struct InvalidFields
/*CHECK:
       ^^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
       ^^^^^^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader' has no runtime representation
*/
{
    StageBox invalidField;
/*CHECK:
             ^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
             ^^^^^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader' has no runtime representation
*/
    property StageBox invalidProperty { get; }
/*CHECK:
             ^^^^^^^^ runtime use of a structural ray-tracing stage type
             ^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader' has no runtime representation
*/
}

Optional<StageBox> invalidOptional;
/*CHECK:
                   ^^^^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
                   ^^^^^^^^^^^^^^^ structural ray-tracing stage type 'Optional<dyn rt.IMissShader>' has no runtime representation
*/

StageBox invalidArray[1];
/*CHECK:
         ^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
         ^^^^^^^^^^^^ structural ray-tracing stage type 'dyn rt.IMissShader[1]' has no runtime representation
*/

Ptr<StageBox> invalidPointer;
/*CHECK:
              ^^^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
              ^^^^^^^^^^^^^^ structural ray-tracing stage type 'Ptr<dyn rt.IMissShader, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>' has no runtime representation
*/
