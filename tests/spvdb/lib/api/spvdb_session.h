#pragma once
// Complete definition of Session, needed wherever unique_ptr<Session> is destroyed.
// Included by spvdb.cpp and any consumer that stores/destroys a unique_ptr<Session>.
#include "spvdb.h"

namespace spvdb
{

struct Session
{
    std::shared_ptr<const SpvModule> module;
    std::string entry_name;
    Interpreter interp;
    bool started = false;

    Session(std::shared_ptr<const SpvModule> m, std::string name, SessionOptions opts)
        : module(m)
        , entry_name(std::move(name))
        , interp(m, opts)
    {
    }

    Result<void> ensure_started()
    {
        if (!started) {
            auto r = interp.begin(entry_name);
            if (!r)
                return r;
            started = true;
        }
        return Result<void> {};
    }
};

} // namespace spvdb
