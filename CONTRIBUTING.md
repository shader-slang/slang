# Shader-Slang Open Source Project

## Contribution Guide

Thank you for considering contributing to the Shader-Slang project! We welcome your help to improve and enhance our project. Please take a moment to read through this guide to understand how you can contribute.

This document is to guide you to contribute to the project. This document is intended to be easy to follow without sending the readers to other pages and links. You can simply copy and paste command lines described on this document.

* Contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant the rights to use your contribution.
* When you submit a pull request, a CLA bot will determine whether you need to sign a CLA. Simply follow the instructions provided.
* Please read and follow the contributor [Code of Conduct](CODE_OF_CONDUCT.md).
* Bug reports and feature requests should go through the GitHub issue tracker
* Changes should ideally come in as small pull requests on top of master, coming from your own personal fork of the project
* Large features that will involve multiple contributors or a long development time should be discussed in issues, and broken down into smaller pieces that can be implemented and checked in in stages

## Table of Contents
1. [Contribution Process](#contribution-process)
   - [Forking the Repository](#forking-the-repository)
   - [Cloning your Fork](#cloning-your-fork)
   - [Creating a Branch](#creating-a-branch)
   - [Build Slang from Source](#build-slang-from-source)
   - [Making Changes](#making-changes)
   - [Testing](#testing)
   - [Commit to the Branch](#commit-to-the-branch)
   - [Push to Forked Repository](#push-to-forked-repository)
   - [Request Pull](#request-pull)
1. [Update your Repository](#update-your-repository)
1. [Code Style](#code-style)
1. [Issue Tracking](#issue-tracking)
1. [Communication](#communication)
1. [License](#license)

## Contribution Process

### Forking the Repository
Navigate to the [Shader-Slang repository](https://github.com/shader-slang/slang).
Click on the "Fork" button in the top right corner to create a copy of the repository in your GitHub account.
This document will assume that the name of your forked repository is "slang".
Make sure your "Actions" are enabled. Visit your forked repository and click on the "Actions" tab and enable the actions. 

### Cloning your Fork
1. Clone your fork locally, replacing "USER-NAME" in the command below with your actual username..
   ```
   $ git clone --recursive --tags https://github.com/USER-NAME/slang.git
   $ cd slang
   ```

1. Fetch tags by adding the original repository as an upstream.
   It is important to have tags in your forked repository, because our workflow/action uses the information for the build process. But the tags are not fetched by default when you fork a repository in github. You need to add the original repository as an upstream and fetch tags manually.
   ```
   $ git remote add upstream https://github.com/shader-slang/slang.git
   $ git fetch --tags upstream
   ```

   You can check whether the tags are fetched properly with the following command.
   ```
   $ git tag -l
   ```

1. Push tags to your forked repository
   The tags are fetched to your local machine but it hasn't been pushed to the forked repository yet. You need to push tags to your forked repository with the following command.
   ```
   $ git push --tags origin
   ```

### Creating a Branch
Create a new branch for your contribution:
```
$ git checkout -b feature/your-feature-name
```

### Build Slang from Source
Please follow the instructions of how to [Building Slang from Source](docs/building.md).

For a quick reference, follow the instructions below.

#### Windows
Download and install CMake from [CMake.org/download](https://cmake.org/download)

Run CMake with the following command to generate a Visual Studio 2022 Solution:
```
C:\git\slang> cmake.exe --preset vs2022 # For VisualStudio 2022
C:\git\slang> cmake.exe --preset vs2019 # For VisualStudio 2019
```

Open `build/slang.sln` with VisualStudio IDE and build it for "x64".

Or you can build with a following command:
```
C:\git\slang> cmake.exe --build --preset release
```

#### Linux
Install CMake and Ninja.
```
$ sudo apt-get install cmake ninja-build
```
> Warning: Currently the required CMake version is 3.25 or above.

Run CMake with a following command to generate Makefile:
```
$ cmake --preset default
```

Build with a following command:
```
$ cmake --build --preset release
```

#### MacOS
Install XCode from AppStore.

Install CMake and Ninja; we recommend using [homebrew](https://brew.sh/) for installing them.
```
brew install ninja
brew install cmake
```

Run CMake with a following command to generate Makefile:
```
$ cmake --preset default
```

Build with a following command:
```
$ cmake --build --preset release
```

#### Building with a local build of slang-llvm
slang-llvm is required to run slang-test properly.
Follow the instructions below if you wish to build slang-llvm locally.
```
$ external/build-llvm.sh --source-dir build/slang-llvm_src --install-prefix build/slang-llvm_install
```

You need to use the following command to re-generate Makefile,
```
$ cmake --preset default --fresh -DSLANG_SLANG_LLVM_FLAVOR=USE_SYSTEM_LLVM -DLLVM_DIR=build/slang-llvm_install/lib/cmake/llvm -DClang_DIR=build/slang-llvm_install/lib/cmake/clang
```

Build with a following command:
```
$ cmake --build --preset release
```

### Making Changes
Make your changes and ensure to follow our [Design Decisions](docs/design/README.md).

### Testing
Test your changes thoroughly to ensure they do not introduce new issues. This is done by building and running a "slang-test" from the repository root directory. For more details about "slang-test", please refer to a [Documentation on testing](tools/slang-test/README.md).

> Note: slang-test is meant to launch from the root of the repository. It uses a hard-coded directory name "tests/" that is expected to exist in the current working directory.

> Note: One of the options for `slang-test.exe` is `-api`, and it takes an additional keyword to specify which API to test. When the option is `-api all-cpu`, as an example, it means it tests all APIs except CPU. The minus sign (-) after `all` means "exclude" and you can "include" with plus sign (+) like `-api gl+dx11`.

If you are familiar with Workflow/Actions in github, you can check [Our Workflows](.github/workflows). "Test Slang" section in [ci.yml](.github/workflows/ci.yml) is where "slang-test" runs.

For a quick reference, follow the instructions below.

#### Windows
1. Download and install VulkanSDK from [LunarG SDK page](https://www.lunarg.com/vulkan-sdk).
1. Set an environment variable to enable SPIR-V validation in Slang compiler,
   ```
   C:\git\slang> set SLANG_RUN_SPIRV_VALIDATION=1
   ```
1. Run slang-test with multiple threads. This may take 10 minutes or less depending on the performance of your computer.
   ```
   C:\git\slang> build\Release\bin\slang-test.exe -use-test-server -server-count 8
   ```
   > Note: if you increase `-server-count` more than 16, you may find some of tests randomly fail. This is a known issue on the graphics driver side.
1. Check whether the test is finished as expected.

#### Linux
1. Install Vulkan-SDK by following the [Instructions in LunarG ](https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html).
   ```
   $ sudo apt update
   $ sudo apt install vulkan-sdk
   ```
1. Run slang-test with multiple threads. This may take 10 minutes or less depending on the performance of your computer.
   ```
   $ ./build/Release/bin/slang-test -use-test-server -server-count 8
   ```
1. Check whether the test is finished as expected.

### Commit to the Branch
Commit your changes to the branch with a descriptive commit message:
```
$ git commit
```

It is important to have a descriptive commit message. Unlike comments inside of the source code, the commit messages don't spoil over time because it is tied to a specific change and it can be reviewed by many people many years later.

Here is a good example of a commit message
> Add user authentication feature
> 
> Fixes #1234
> 
> This commit introduces a new user authentication feature. It includes changes to the login page, user database, and session management to provide secure user authentication.

### Push to Forked Repository
Push your branch to your forked repository with the following command.
```
$ git push origin feature/your-feature-name
```

### Request Pull

Open a pull request against `shader-slang/slang`.

Code formatting can be automatically fixed on your branch by commenting `/format`, a bot will proceed to open a PR targeting *your* branch.

For the Pull Request, you will need to write a PR message. This message is for a set of commits you are requesting to pull. Try to make it brief because the actual details should be in the commit messages of each commit.
 
The PR requires an approval from people who have permissions. They will review the changes before approve the Pull. During this step, you will get feedbacks from other people and they may request you to make some changes. When you need to make adjustments, you can commit new changes to the branch in your forlked repository that already has the changes in PR process. When new commits are added to the branch, they will automatically appear in the PR.

## Addressing Code Reviews and Keep Branch In-Sync
After your pull request is submitted, you will receive code reviews from the community within 24 hours. Follow-up changes that address review comments should be pushed to your pull request branch as additional commits. When your branch is out of sync with top-of-tree, submit a merge commit to keep them in-sync. Do not rebase and force push after the PR is created to keep the change history during the review process.

Use these commands to sync your branch:

```
$ git fetch upstream master
$ git merge upstream/master # resolve any conflicts here
$ git submodule update --recursive
```

The Slang repository uses the squash strategy for merging pull requests, which means all your commits will be squashed into one commit by GitHub upon merge.

## Labeling Breaking Changes

All pull requests must be labeled as either `pr: non-breaking` or `pr: breaking change` before it can be merged to the main branch. If you are already a committer, you are expected to label your PR when you create it. If you are not yet a committer, a reviewer will do this for you.

A PR is considered to introduce a breaking change if an existing application that uses Slang may no longer compile or behave the same way with the change. Typical examples of breaking changes include:

- Changes to `slang.h` that modifies the Slang API in a way that breaks binary compatibility.
- Changes to the language syntax or semantics that may cause existing Slang code to not compile or produce different run-time result. For example changing the overload resolution rules.
- Removing or renaming an existing intrinsic from the core module.


## Code Style
Follow our [Coding conventions](docs/design/coding-conventions.md) to maintain consistency throughout the project.

Here are a few highlights
1. Indent by four spaces. Don't use tabs except in files that require them (e.g., Makefiles).
1. Don't use the STL containers, iostreams, or the built-in C++ RTTI system.
1. Don't use the C++ variants of C headers (e.g., use `<stdio.h>` instead of `<cstdio>`).
1. Don't use exceptions for non-fatal errors (and even then support a build flag to opt out of exceptions).
1. Types should use UpperCamelCase, values should use lowerCamelCase, and macros should use `SCREAMING_SNAKE_CASE` with a prefix `SLANG_`.
1. Global variables should have a `g` prefix, non-const static class members can have an `s` prefix, constant data (in the sense of static const) should have a `k` prefix, and an `m_` prefix on member variables and a `_` prefix on member functions are allowed.
1. Prefixes based on types (e.g., p for pointers) should never be used.
1. In function parameter lists, an `in`, `out`, or `io` prefix can be added to a parameter name to indicate whether a pointer/reference/buffer is intended to be used for input, output, or both input and output.
1. Trailing commas should always be used for array initializer lists.
1. Try to write comments that explain the "why" of your code more than the "what".

## Issue Tracking
We track all our work with GitHub issues. Check the [Issues](https://github.com/shader-slang/slang/issues) for open issues. If you find a bug or want to suggest an enhancement, please open a new issue.

If you're new to the project or looking for a good starting point, consider exploring issues labeled as [Good first bug](https://github.com/shader-slang/slang/issues?q=is%3Aissue+is%3Aopen+label%3AGoodFirstBug). These are beginner-friendly bugs that provide a great entry point for new contributors.

## Communication
Join our [Discussions](https://github.com/shader-slang/slang/discussions).

## License
By contributing to Shader-Slang, you agree that your contributions will be licensed under the MIT License. The full text of the License can be found in the [LICENSE](LICENSE) file in the root of the repository.

