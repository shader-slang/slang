# Slang Test

Slang Test is a command line tool that is used to coordinate tests via other tools. The actual executable is 'slang-test'. It is typically run from the test.bat script in the root directory of the project.

Slang Test can be thought of as the 'hub' running multiple tests and accumulating the results. In the distribution tests are held in the 'tests' directory. Inside this directory there are tests grouped together via other directories. Inside those directories are the actual tests themselves. The tests exist as .hlsl, .slang and .glsl and other file extensions. The top line of each of these files describe what kind of test will be performed with a specialized comment '//TEST'. 

Most command line options are prefixed by - for both switches and parameter options. On the command line you can specify a 'free parameter', and this value specifies the prefix of the name of the test to run. Note that such a prefix includes the path, with each directory separated via '/'.

An example command line:

```
slang-test -bindir E:\slang\bin\windows-x64\Debug\\ -category full tests/compute/array-param
```

* The -bindir value means that the tools slang-test will use the binaries found in this directory. 
* The -category full means that all tests can be run.
* The final 'free parameter' is 'tests/compute/array-param' and means only tests starting with this string will run.

Most types of test use 'test tools' to implement actual tests. There are currently 3 'tools' that are typically used 

* slangc
* render-test
* slang-reflection-test

These are typically implemeted as dlls/shared libraries that are loaded when a test is needed. Sometimes it is necessary or useful to just call one of these test tools directly with the parameters the tool takes. This can be achieved by giving the tool as a 'sub command' name on the command line. All of the parameters after the tool name will be passed directly to the tool. For example

 ```
slang-test -bindir E:\slang\bin\windows-x64\Debug\\ slangc tests/compute/array-param.slang
```

Will run the 'slangc' tool with the parameters listed after 'slangc' on the command line. Any parameters before the sub command will be parsed as usual by slang-test, and if not applicable to invoking the tool will be ignored. bindir will be used for finding the tool directory. This is by design so that the sub command invocation can just be placed after the normal slang-test commands, and removed when no longer needed. 

The command line can control which tests are run with a couple of switches

* -api - Overall controls over which apis will be tested against 
* -synthesizedTestApi - Controls which apis will have tests synthesized for them from other apis. Tests can be synthesized for dx12 and vulkan.

The parameter afterwards is an 'api expression' used to control which apis will or wont be used. This is somewhat like a mathematical expression with only + and - operations and api names. If the first operation is + or - it will be applied to whatever the default is, otherwise the defaults are ignored.

* vk - just for vulkan
* +vk - Whatever the defaults are including vulkan
* -dx12 - Whatever the defaults are excluding vulkan
* all - for all apis, none - for no apis
* all-vk - all apis but not vulkan (vk)
* all-vk-dx12 - all apis but not vulkan (vk) or directx12 (dx12)
* gl+dx11 - just on opengl (gl) and directx11 (dx11)
* +none - same as defaults 

So if you wanted to test for all apis, except opengl you'd put on the command line '-api all-gl'

The different APIs are 

* OpenGL - gl,ogl,opengl
* Vulkan - vk,vulkan
* DirectD3D12 - dx12,d3d12
* DirectD3D11 -dx11,d3d11


It may also be necessary to have the working directory the root directory of the slang distribution - in the example above this would be "E:\slang\". 

## Test Categories

There are the following test categories

* full
* quick
* smoke
* render
* compute
* vulkan
* compatibility-issue

A test may be in one or more categories. The categories are specified in the test line, for example: 
//TEST(smoke,compute):COMPARE_COMPUTE:

## Command line options

### bindir 

Specifies the directory where executables will be found. 

Eg -bindir "windows-x64\Debug\\"

### category 

The parameter controls what kinds of tests will be run. Categories are listed at the 'test categories' section.

### exclude 

Used to specify categories to be excluded during a test.

### appveyor

A flag that makes output suitable for the appveyor automated test suite.

### travis 

A flag that makes output suitable for the travis automated test suite.

### Other Command Line Options

The following flags/paramteres can be passed but will be ignored by the tool

* debug (flag)
* release (flag)
* platform (parameter)

## Test Types

Test types are controlled via a comment at the top of a test file, that starts with //TEST:
This is then immediately followed by the test type which is one of the following

* SIMPLE 
	* Calls the slangc compiler with options after the comment 
* REFLECTION
	* Runs 'slang-reflection-test' passing in the options as given after the command
* COMPARE_HLSL
	* Runs the slangc compiler, forcing dxbc output and compares with file post fixed with '.expected'
* COMPARE_HLSL_RENDER
	* Runs 'render-test' rendering two images - one for hlsl (expected), and one for slang saving to .png files. The images must match for the test to pass. 
* COMPARE_HLSL_CROSS_COMPILE_RENDER
	* Runs 'render-test' rendering two images - one from slag, and the other -glsl-cross. The images must match for the test to pass.
* COMPARE_HLSL_GLSL_RENDER
	* Runs 'render-test' rendering two images - one with -hlsl-rewrite and the other -glsl-rewrite. The images must match for test to pass.
* COMPARE_COMPUTE
	* Runs 'render-test' producing a compute result written as a text file. Text file contents must be identical.
* COMPARE_COMPUTE_EX
	* Same as COMPARE_COMPUTE, but allows specific parameters to be specified.
* HLSL_COMPUTE
	* Runs 'render-test' with "-hlsl-rewrite -compute" options. Text files are compared. 
* COMPARE_RENDER_COMPUTE
	* Runs 'render-test' with "-slang -gcompute" options. Text files are compared. 
* COMPARE_GLSL
	* Runs the slangc compiler compiling through slang, and without and comparing output in spirv assembly.
* CROSS_COMPILE
	* Compiles as glsl pass through and then through slang and comparing output
* EVAL
	* Runs 'slang-eval-test' - which runs code on slang VM

