#ifndef CORELIB_TYPETRAITS_H
#define CORELIB_TYPETRAITS_H

namespace CoreLib
{
	namespace Basic
	{
		struct TraitResultYes
		{
			char x;
		};
		struct TraitResultNo
		{
			char x[2];
		};

		template <typename B, typename D>
		struct IsBaseOfTraitHost
		{
			operator B*() const { return nullptr; }
			operator D*() { return nullptr; }
		};

		template <typename B, typename D>
		struct IsBaseOf
		{
			template <typename T>
			static TraitResultYes Check(D*, T) { return TraitResultYes(); }
			static TraitResultNo Check(B*, int) { return TraitResultNo(); }
			enum { Value = sizeof(Check(IsBaseOfTraitHost<B, D>(), int())) == sizeof(TraitResultYes) };
		};

		template<bool B, class T = void>
		struct EnableIf {};

		template<class T>
		struct EnableIf<true, T> { typedef T type; };

		template <typename B, typename D>
		struct IsConvertible
		{
			static TraitResultYes Use(B) {};
			static TraitResultNo Use(...) {};
			enum { Value = sizeof(Use(*(D*)(nullptr))) == sizeof(TraitResultYes) };
		};
	}
}

#endif
