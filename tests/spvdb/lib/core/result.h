#pragma once
#include <string>
#include <utility>

namespace spvdb
{

struct Error
{
    std::string message;
};

// Minimal std::expected<T, Error> equivalent for C++20.
template<typename T> struct Result
{
    Result(T val)
        : ok_(true)
    {
        new (&storage_) T(std::move(val));
    }
    Result(Error e)
        : ok_(false)
        , error_(std::move(e))
    {
    }

    static Result err(std::string msg)
    {
        return Result(Error {std::move(msg)});
    }

    bool ok() const
    {
        return ok_;
    }
    explicit operator bool() const
    {
        return ok_;
    }

    T &value()
    {
        return *reinterpret_cast<T *>(&storage_);
    }
    const T &value() const
    {
        return *reinterpret_cast<const T *>(&storage_);
    }

    T &operator*()
    {
        return value();
    }
    T *operator->()
    {
        return &value();
    }
    const T &operator*() const
    {
        return value();
    }
    const T *operator->() const
    {
        return &value();
    }

    const Error &error() const
    {
        return error_;
    }

    ~Result()
    {
        if (ok_)
            reinterpret_cast<T *>(&storage_)->~T();
    }

    Result(const Result &o)
        : ok_(o.ok_)
        , error_(o.error_)
    {
        if (ok_)
            new (&storage_) T(*reinterpret_cast<const T *>(&o.storage_));
    }
    Result(Result &&o)
        : ok_(o.ok_)
        , error_(std::move(o.error_))
    {
        if (ok_)
            new (&storage_) T(std::move(*reinterpret_cast<T *>(&o.storage_)));
    }
    Result &operator=(Result o)
    {
        this->~Result();
        new (this) Result(std::move(o));
        return *this;
    }

private:
    bool ok_;
    Error error_;
    alignas(T) unsigned char storage_[sizeof(T)];
};

template<> struct Result<void>
{
    Result()
        : ok_(true)
    {
    }
    Result(Error e)
        : ok_(false)
        , error_(std::move(e))
    {
    }

    static Result err(std::string msg)
    {
        return Result(Error {std::move(msg)});
    }

    bool ok() const
    {
        return ok_;
    }
    explicit operator bool() const
    {
        return ok_;
    }
    const Error &error() const
    {
        return error_;
    }

private:
    bool ok_;
    Error error_;
};

} // namespace spvdb
