// An `import` in a source file that is not the last one on the slangc command line must
// still be visible (issue #6221). SIMPLE prepends this test file before the directive
// arguments, so source1.slang stays before source2.slang.

//TEST:SIMPLE:tests/language-feature/modules/multiple-source-files/source2.slang

module source1;

import A;

[shader("compute")]
void computeMain1() { int a = test(); }