//TEST:SIMPLE:

// `where nonempty(...)` must participate in generic signature matching,
// so these declarations are distinct overloads instead of redeclarations.

void typePackOverload<each T>() {}
void typePackOverload<each T>() where nonempty(T) {}

void valuePackOverload<let each D : int>() {}
void valuePackOverload<let each D : int>() where nonempty(D) {}
