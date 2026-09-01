> TODO

Overload Resolution
----------------------

It is possible for an identifier expression to be _overloaded_, such that it refers to one or more candidate declarations with the same name.
If the expression appears in a context where the correct declaration to use can be disambiguated, then that declaration is used as the result of  the name expression; otherwise use of an overloaded name is an error at the use site.
