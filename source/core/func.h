#ifndef CORELIB_FUNC_H
#define CORELIB_FUNC_H

#if 0

#include "SmartPointer.h"

namespace CoreLib
{
	namespace Basic
	{
		template<typename TResult, typename... Arguments>
		class FuncPtr
		{
		public:
			virtual TResult operator()(Arguments...) = 0;
			virtual bool operator == (const FuncPtr<TResult, Arguments...> *)
			{
				return false;
			}
			virtual ~FuncPtr() {}
		};

		template<typename TResult, typename... Arguments>
		class CdeclFuncPtr : public FuncPtr<TResult, Arguments...>
		{
		public:
			typedef TResult (*FuncType)(Arguments...);
		private:
			FuncType funcPtr;
		public:
			CdeclFuncPtr(FuncType func)
				:funcPtr(func)
			{
			}

			virtual TResult operator()(Arguments... params) override
			{
				return funcPtr(params...);
			}

			virtual bool operator == (const FuncPtr<TResult, Arguments...> * ptr) override
			{
				auto cptr = dynamic_cast<const CdeclFuncPtr<TResult, Arguments...>*>(ptr);
				if (cptr)
					return funcPtr == cptr->funcPtr;
				else
					return false;
			}
		};

		template<typename Class, typename TResult, typename... Arguments>
		class MemberFuncPtr : public FuncPtr<TResult, Arguments...>
		{
		public:
			typedef TResult (Class::*FuncType)(Arguments...);
		private:
			FuncType funcPtr;
			Class * object;
		public:
			MemberFuncPtr(Class * obj, FuncType func)
				: funcPtr(func), object(obj)
			{
			}

			virtual TResult operator()(Arguments... params) override
			{
				return (object->*funcPtr)(params...);
			}

			virtual bool operator == (const FuncPtr<TResult, Arguments...> * ptr) override
			{
				auto cptr = dynamic_cast<const MemberFuncPtr<Class, TResult, Arguments...>*>(ptr);
				if (cptr)
					return funcPtr == cptr->funcPtr && object == cptr->object;
				else
					return false;
			}
		};

		template<typename F, typename TResult, typename... Arguments>
		class LambdaFuncPtr : public FuncPtr<TResult, Arguments...>
		{
		private:
			F func;
		public:
			LambdaFuncPtr(const F & _func)
				: func(_func)
			{}
			virtual TResult operator()(Arguments... params) override
			{
				return func(params...);
			}
			virtual bool operator == (const FuncPtr<TResult, Arguments...> * /*ptr*/) override
			{
				return false;
			}
		};

		template<typename TResult, typename... Arguments>
		class Func
		{
		private:
			RefPtr<FuncPtr<TResult, Arguments...>> funcPtr;
		public:
			Func(){}
			Func(typename CdeclFuncPtr<TResult, Arguments...>::FuncType func)
			{
				funcPtr = new CdeclFuncPtr<TResult, Arguments...>(func);
			}
			template<typename Class>
			Func(Class * object, typename MemberFuncPtr<Class, TResult, Arguments...>::FuncType func)
			{
				funcPtr = new MemberFuncPtr<Class, TResult, Arguments...>(object, func);
			}
			template<typename TFuncObj>
			Func(const TFuncObj & func)
			{
				funcPtr = new LambdaFuncPtr<TFuncObj, TResult, Arguments...>(func);
			}
			Func & operator = (typename CdeclFuncPtr<TResult, Arguments...>::FuncType func)
			{
				funcPtr = new CdeclFuncPtr<TResult, Arguments...>(func);
				return *this;
			}
			template<typename Class>
			Func & operator = (const MemberFuncPtr<Class, TResult, Arguments...> & func)
			{
				funcPtr = new MemberFuncPtr<Class, TResult, Arguments...>(func);
				return *this;
			}
			template<typename TFuncObj>
			Func & operator = (const TFuncObj & func)
			{
				funcPtr = new LambdaFuncPtr<TFuncObj, TResult, Arguments...>(func);
				return *this;
			}
			bool operator == (const Func & f)
			{
				return *funcPtr == f.funcPtr.Ptr();
			}
			bool operator != (const Func & f)
			{
				return !(*this == f);
			}
			TResult operator()(Arguments... params)
			{
				return (*funcPtr)(params...);
			}
		};

		// template<typename... Arguments>
		// using Procedure = Func<void, Arguments...>;

		template<typename... Arguments>
		class Procedure : public Func<void, Arguments...>
		{
		private:
			RefPtr<FuncPtr<void, Arguments...>> funcPtr;
		public:
			Procedure(){}
			Procedure(const Procedure & proc)
			{
				funcPtr = proc.funcPtr;
			}
			Procedure(typename CdeclFuncPtr<void, Arguments...>::FuncType func)
			{
				funcPtr = new CdeclFuncPtr<void, Arguments...>(func);
			}
			template<typename Class>
			Procedure(Class * object, typename MemberFuncPtr<Class, void, Arguments...>::FuncType func)
			{
				funcPtr = new MemberFuncPtr<Class, void, Arguments...>(object, func);
			}
			template<typename TFuncObj>
			Procedure(const TFuncObj & func)
			{
				funcPtr = new LambdaFuncPtr<TFuncObj, void, Arguments...>(func);
			}
			Procedure & operator = (typename CdeclFuncPtr<void, Arguments...>::FuncType func)
			{
				funcPtr = new CdeclFuncPtr<void, Arguments...>(func);
				return *this;
			}
			template<typename Class>
			Procedure & operator = (const MemberFuncPtr<Class, void, Arguments...> & func)
			{
				funcPtr = new MemberFuncPtr<Class, void, Arguments...>(func);
				return *this;
			}
			template<typename TFuncObj>
			Procedure & operator = (const TFuncObj & func)
			{
				funcPtr = new LambdaFuncPtr<TFuncObj, void, Arguments...>(func);
				return *this;
			}
			Procedure & operator = (const Procedure & proc)
			{
				funcPtr = proc.funcPtr;
			}
			void Clear()
			{
				funcPtr = nullptr;
			}
			void operator()(Arguments... params)
			{
				if (funcPtr)
					(*funcPtr)(params...);
			}
		};
	}
}

#endif

#endif