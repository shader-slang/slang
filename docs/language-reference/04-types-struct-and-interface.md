Structure Types
---------------

Structure types are introduced with `struct` declarations, and consist of an ordered sequence of named and typed fields:

```hlsl
struct S
{
    float2 f;
    int3 i;
}
```

### Standard Layout

The _standard layout_ for a structure type uses the following algorithm:

* Initialize variables `size` and `alignment` to zero and one, respectively
* For each field `f` of the structure type:
  * Update `alignment` to be the maximum of `alignment` and the alignment of `f`
  * Set `size` to the smallest multiple of `alignment` not less than `size`
  * Set the offset of field `f` to `size`
  * Add the size of `f` to `size`

When this algorithm completes, `size` and `alignment` will be the size and alignment of the structure type.

Most target platforms do not use the standard layout directly, but it provides a baseline for defining other layout algorithms.
Any layout for structure types must guarantee an alignment at least as large as the standard layout.

### C-Style Layout

C-style layout for structure types differs from standard layout by adding an additional final step:

* Set `size` the smallest multiple of `alignment` not less than `size`

This mirrors the layout rules used by typical C/C++ compilers.

### D3D Constant Buffer Layout

D3D constant buffer layout is similar to standard layout with two differences:

* The initial alignment is 16 instead of one

* If a field would have _improper straddle_, where the interval `(fieldOffset, fieldOffset+fieldSize)` (exclusive on both sides) contains any multiple of 16, *and* the field offset is not already a multiple of 16, then the offset of the field is adjusted to the next multiple of 16

This Type
---------

Within the body of a structure or interface declaration, the keyword `This` may be used to refer to the enclosing type.
Inside of a structure type declaration, `This` refers to the structure type itself.
Inside of an interface declaration, `This` refers to the concrete type that is conforming to the interface (that is, the type of `this`).
