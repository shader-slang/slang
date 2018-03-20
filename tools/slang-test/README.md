slang-test
==========

Slang test is a command line tool that is used to coordinate tests via other command line tools. It can be thought of as the 'hub' running multiple tests, and accumulating the results. In the distribution tests are held in the 'tests' directory. Inside this directory there are tests grouped together via other directories. Inside those directories are the actual tests themselves. The tests exist as .hlsl, .slang and .glsl and other files. The top line of each of these files describe what kind of test will be performed with a specialized comment. 

Test Categories
===============

There are the following test categories

* full
* quick
* smoke
* render
* compute
* vulkan

A test may be in one or more categories. The categories are specified in the test line, for example: 
//TEST(smoke,compute):COMPARE_COMPUTE:

Command line options
====================

** bindir ** 

Specifies the directory where executables will be found. 

"E:\git\slang-jsmall-nvidia\bin\windows-x64\Debug\\" 

Eg -bindir "windows-x64\Debug\\"

** -platform ** 

The parameter after be x64, or x86.

Eg. -platform x64

** debug ** 

A flag that if set means a debug executable/s will be used for testing.

** release **

A flag that if set means a release executables will be used for testing.

** category ** 

The parameter controls what kinds of tests will be run.

* render
* 

** exclude **

Used to specify categories to be excluded during a test.

** appveyor **

A flag that makes output suitable for the appveyor automated test suite.

** travis **

A flag that makes output suitable for the travis automated test suite.

Test types
==========

Test types are controlled via a comment at the top of a test file, that starts with //TEST:
This is then immediately followed by the test type which is one of the following

* SIMPLE 
	** Calls the slangc compiler with options after the comment 
* REFLECTION
	** Runs 'slang-reflection-test' passing in the options as given after the command
* COMPARE_HLSL
	** Runs the slangc compiler, forcing dxbc output and compares with file post fixed with '.expected'
* COMPARE_HLSL_RENDER
	** Runs 'render-test' rendering two images - one for hlsl (expected), and one for slang saving to .png files. The images must match for the test to pass. 
* COMPARE_HLSL_CROSS_COMPILE_RENDER
	** Runs 'render-test' rendering two images - one from slag, and the other -glsl-cross. The images must match for the test to pass.
* COMPARE_HLSL_GLSL_RENDER
	** Runs 'render-test' rendering two images - one with -hlsl-rewrite and the other -glsl-rewrite. The images must match for test to pass.
* COMPARE_COMPUTE
	** Runs 'render-test' producing a compute result written as a text file. Text file contents must be identical.
* COMPARE_COMPUTE_EX
	** Same as COMPARE_COMPUTE, but allows specific parameters to be specified.
* HLSL_COMPUTE
	** Runs 'render-test' with "-hlsl-rewrite -compute" options. Text files are compared. 
* COMPARE_RENDER_COMPUTE
	** Runs 'render-test' with "-slang -gcompute" options. Text files are compared. 
* COMPARE_GLSL
	** Runs the slangc compiler compiling through slang, and without and comparing output in spirv assembly.
* CROSS_COMPILE
	** Compiles as glsl pass through and then through slang and comparing output
* EVAL
	** Runs 'slang-eval-test' - which runs code on slang VM

