// Second input for debug-function-scope-line-collide.slang, and the file that its `#line` directive
// names. No //TEST: directive.
//
// The collision is the point: because this file is itself an input, it has a compilation unit of its
// own, so a lookup keyed on the remapped source would find this unit and bind a function that was
// compiled elsewhere.

float collideOtherFn(float x)
{
    return x + 1.0;
}
