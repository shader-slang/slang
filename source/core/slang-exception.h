#ifndef SLANG_CORE_EXCEPTION_H
#define SLANG_CORE_EXCEPTION_H

#include "slang-common.h"
#include "slang-string.h"

namespace Slang
{
	class Exception
	{
	public:
		String Message;
		Exception()
		{}
		Exception(const String & message)
			: Message(message)
		{
		}

        virtual ~Exception()
        {}
	};

	class IndexOutofRangeException : public Exception
	{
	public:
		IndexOutofRangeException()
		{}
		IndexOutofRangeException(const String & message)
			: Exception(message)
		{
		}

	};

	class InvalidOperationException : public Exception
	{
	public:
		InvalidOperationException()
		{}
		InvalidOperationException(const String & message)
			: Exception(message)
		{
		}

	};
		
	class ArgumentException : public Exception
	{
	public:
		ArgumentException()
		{}
		ArgumentException(const String & message)
			: Exception(message)
		{
		}

	};

	class KeyNotFoundException : public Exception
	{
	public:
		KeyNotFoundException()
		{}
		KeyNotFoundException(const String & message)
			: Exception(message)
		{
		}
	};
	class KeyExistsException : public Exception
	{
	public:
		KeyExistsException()
		{}
		KeyExistsException(const String & message)
			: Exception(message)
		{
		}
	};

	class NotSupportedException : public Exception
	{
	public:
		NotSupportedException()
		{}
		NotSupportedException(const String & message)
			: Exception(message)
		{
		}
	};

	class NotImplementedException : public Exception
	{
	public:
		NotImplementedException()
		{}
		NotImplementedException(const String & message)
			: Exception(message)
		{
		}
	};

	class InvalidProgramException : public Exception
	{
	public:
		InvalidProgramException()
		{}
		InvalidProgramException(const String & message)
			: Exception(message)
		{
		}
	};

	class InternalError : public Exception
	{
	public:
		InternalError()
		{}
		InternalError(const String & message)
			: Exception(message)
		{
		}
	};

    class AbortCompilationException : public Exception
    {
    public:
        AbortCompilationException()
        {}
        AbortCompilationException(const String & message)
            : Exception(message)
        {
        }
    };
}

#endif
