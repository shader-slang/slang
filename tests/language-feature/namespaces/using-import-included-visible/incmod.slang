module incmod;

// Primary file `__include`s a second file; its public decls live in a `FileDecl`
// child of this module and must stay visible through `import` (shader-slang/slang#11443).
__include incmod_inc;
