//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-lang hlsl -Gec

// With `-Gec`, the parser renames each global shader parameter and adds a mutable shadow
// variable under its original name. For a `cbuffer`, the `__transparent` modifier moves from
// the buffer variable to the shadow. The members of the `cbuffer` still conflict with other
// declarations of the same name (shader-slang/slang#13876).

int globalUniform;
/*CHECK:
    ^^^^^^^^^^^^^ see previous declaration of 'globalUniform'
*/

cbuffer CB
{
    int globalUniform;
/*CHECK:
        ^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^ declaration of 'globalUniform' conflicts with existing declaration
*/
    int unique;
}

// A declaration after the `cbuffer` is compared with the shadow's members as well, and the
// members of a second `cbuffer` are compared with those of the first.

cbuffer LaterCB
{
    int laterUniform;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'laterUniform'
*/
    int sharedMember;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'sharedMember'
*/
}

int laterUniform;
/*CHECK:
    ^^^^^^^^^^^^ warning E30204
    ^^^^^^^^^^^^ declaration of 'laterUniform' conflicts with existing declaration
*/

cbuffer SecondCB
{
    int sharedMember;
/*CHECK:
        ^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^ declaration of 'sharedMember' conflicts with existing declaration
*/
}

// With `-Gec`, the shadow of a `tbuffer` keeps the `TextureBuffer` type, and lookup still sees
// through it to the members.

int gecTBufferName;
/*CHECK:
    ^^^^^^^^^^^^^^ see previous declaration of 'gecTBufferName'
*/

tbuffer TB
{
    int gecTBufferName;
/*CHECK:
        ^^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^^ declaration of 'gecTBufferName' conflicts with existing declaration
*/
}

// A `cbuffer` in a function body is compared with the other declarations of that block too.

int functionWithGecBuffer()
{
    int gecLocalName = 1;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'gecLocalName'
*/
    cbuffer GecLocalBuffer
    {
        int gecLocalName;
/*CHECK:
            ^^^^^^^^^^^^ warning E30204
            ^^^^^^^^^^^^ declaration of 'gecLocalName' conflicts with existing declaration
*/
    }
    return 0;
}
