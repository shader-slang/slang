// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "core/slang-basic.h"
#include "core/slang-std-writers.h"
#include "slangi-options.h"

#include <memory>

namespace slang
{
struct IGlobalSession;
}

namespace TestServer
{

/// Compiles and executes HGVM tests while retaining compiler sessions and the native RHI host.
class HGVMTestTool
{
public:
    HGVMTestTool();
    ~HGVMTestTool();

    SlangResult execute(
        Slang::StdWriters* stdWriters,
        slang::IGlobalSession* globalSession,
        const SlangITool::Options& options);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace TestServer
