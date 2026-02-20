// register-bindings.slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

// Various bad forms for register bindings

// No register index given:
TextureCube b : register(t);
/*CHECK:
                         ^ expected a register index
                         ^ expected a register index after 't'
*/

// Unexpected name in place of `space`:
SamplerState c : register(s0, s1);
/*CHECK:
                              ^^ expected 'space'
                              ^^ expected 'space', got 's'
*/

// No space index given after `space`:
SamplerState d : register(s2, space);
/*CHECK:
                              ^^^^^ expected a register space index after 'space'
                              ^^^^^ expected a register space index after 'space'
*/


void main()
{}
