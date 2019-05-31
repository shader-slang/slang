#ifndef SLANG_CORE_MATH_H
#define SLANG_CORE_MATH_H

#include <math.h>

namespace Slang
{
	class Math
	{
	public:
        // Use to fix type punning issues with strict aliasing
        union FloatIntUnion
        {
            float fvalue;
            int ivalue;

            inline static FloatIntUnion makeFromInt(int i) { FloatIntUnion cast; cast.ivalue = i; return cast; }
            inline static FloatIntUnion makeFromFloat(float f) { FloatIntUnion cast; cast.fvalue = f; return cast; }
        };

        
		static const float Pi;
		template<typename T>
		static T Min(const T& v1, const T&v2)
		{
			return v1<v2?v1:v2;
		}
		template<typename T>
		static T Max(const T& v1, const T&v2)
		{
			return v1>v2?v1:v2;
		}
		template<typename T>
		static T Min(const T& v1, const T&v2, const T&v3)
		{
			return Min(v1, Min(v2, v3));
		}
		template<typename T>
		static T Max(const T& v1, const T&v2, const T&v3)
		{
			return Max(v1, Max(v2, v3));
		}
		template<typename T>
		static T Clamp(const T& val, const T& vmin, const T&vmax)
		{
			if (val < vmin) return vmin;
			else if (val > vmax) return vmax;
			else return val;
		}

		static inline int FastFloor(float x)
		{
			int i = (int)x;
			return i - (i > x);
		}

		static inline int FastFloor(double x)
		{
			int i = (int)x;
			return i - (i > x);
		}

		static inline int IsNaN(float x)
		{
#ifdef _M_X64
			return _isnanf(x);
#else
			return isnan(x);
#endif
		}

		static inline int IsInf(float x)
		{
			return isinf(x);
		}

		static inline unsigned int Ones32(unsigned int x)
		{
			/* 32-bit recursive reduction using SWAR...
				but first step is mapping 2-bit values
				into sum of 2 1-bit values in sneaky way
			*/
			x -= ((x >> 1) & 0x55555555);
			x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
			x = (((x >> 4) + x) & 0x0f0f0f0f);
			x += (x >> 8);
			x += (x >> 16);
			return(x & 0x0000003f);
		}

		static inline unsigned int Log2Floor(unsigned int x)
		{
			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);
			x |= (x >> 8);
			x |= (x >> 16);
			return(Ones32(x >> 1));
		}

		static inline unsigned int Log2Ceil(unsigned int x)
		{
			int y = (x & (x - 1));
			y |= -y;
			y >>= (32 - 1);
			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);
			x |= (x >> 8);
			x |= (x >> 16);
			return(Ones32(x >> 1) - y);
		}
		/*
		static inline int Log2(float x)
		{
			unsigned int ix = (unsigned int&)x;
			unsigned int exp = (ix >> 23) & 0xFF;
			int log2 = (unsigned int)(exp) - 127;

			return log2;
		}
		*/
	};
    inline int FloatAsInt(float val)
	{
        return Math::FloatIntUnion::makeFromFloat(val).ivalue; 
	}
    inline float IntAsFloat(int val)
	{
        return Math::FloatIntUnion::makeFromInt(val).fvalue; 
	}

	inline unsigned short FloatToHalf(float val)
	{
        const auto x = FloatAsInt(val);
        
		unsigned short bits = (x >> 16) & 0x8000;
		unsigned short m = (x >> 12) & 0x07ff;
		unsigned int e = (x >> 23) & 0xff;
		if (e < 103)
			return bits;
		if (e > 142)
		{
			bits |= 0x7c00u;
			bits |= e == 255 && (x & 0x007fffffu);
			return bits;
		}
		if (e < 113)
		{
			m |= 0x0800u;
			bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
			return bits;
		}
		bits |= ((e - 112) << 10) | (m >> 1);
		bits += m & 1;
		return bits;
	}

	inline float HalfToFloat(unsigned short input)
	{
		static const auto magic = Math::FloatIntUnion::makeFromInt((127 + (127 - 15)) << 23);
		static const auto was_infnan = Math::FloatIntUnion::makeFromInt((127 + 16) << 23);
        Math::FloatIntUnion o;
		o.ivalue = (input & 0x7fff) << 13;     // exponent/mantissa bits
		o.fvalue *= magic.fvalue;                 // exponent adjust
		if (o.fvalue >= was_infnan.fvalue)        // make sure Inf/NaN survive
			o.ivalue |= 255 << 23;
		o.ivalue |= (input & 0x8000) << 16;    // sign bit
		return o.fvalue;
	}

	class Random
	{
	private:
		unsigned int seed;
	public:
		Random(int seed)
		{
			this->seed = seed;
		}
		int Next() // random between 0 and RandMax (currently 0x7fff)
		{
			return ((seed = ((seed << 12) + 150889L) % 714025) & 0x7fff);
		}
		int Next(int min, int max) // inclusive min, exclusive max
		{
			unsigned int a = ((seed = ((seed << 12) + 150889L) % 714025) & 0xFFFF);
			unsigned int b = ((seed = ((seed << 12) + 150889L) % 714025) & 0xFFFF);
			unsigned int r = (a << 16) + b;
			return min + r % (max - min);
		}
		float NextFloat()
		{
			return ((Next() << 15) + Next()) / ((float)(1 << 30));
		}
		float NextFloat(float valMin, float valMax)
		{
			return valMin + (valMax - valMin) * NextFloat();
		}
		static int RandMax()
		{
			return 0x7fff;
		}
	};
}

#endif 
