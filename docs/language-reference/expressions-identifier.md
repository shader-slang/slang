Identifier Expressions
----------------------

An _identifier expression_ consists of a single identifier:

```hlsl
someName
```

When evaluated, this expression looks up `someName` in the environment of the expression and yields the value of a declaration with a matching name.

An identifier expression is an l-value if the declaration it refers to is mutable.

### Overloading

It is possible for an identifier expression to be _overloaded_, such that it refers to one or more candidate declarations with the same name.
If the expression appears in a context where the correct declaration to use can be disambiguated, then that declaration is used as the result of  the name expression; otherwise use of an overloaded name is an error at the use site.

### Implicit Lookup

It is possible for a name expression to refer to nested declarations in two ways:

* In the body of a method, a reference to `someName` may resolve to `this.someName`, using the implicit `this` parameter of the method

* When a global-scope `cbuffer` or `tbuffer` declaration is used, `someName` may refer to a field declared inside the `cbuffer` or `tbuffer`

