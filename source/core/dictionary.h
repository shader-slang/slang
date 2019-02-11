#ifndef CORE_LIB_DICTIONARY_H
#define CORE_LIB_DICTIONARY_H
#include "list.h"
#include "common.h"
#include "int-set.h"
#include "exception.h"
#include "slang-math.h"
#include "hash.h"

namespace Slang
{
	template<typename TKey, typename TValue>
	class KeyValuePair
	{
	public:
		TKey Key;
		TValue Value;
		KeyValuePair()
		{}
		KeyValuePair(const TKey & key, const TValue & value)
		{
			Key = key;
			Value = value;
		}
		KeyValuePair(TKey && key, TValue && value)
		{
			Key = _Move(key);
			Value = _Move(value);
		}
		KeyValuePair(TKey && key, const TValue & value)
		{
			Key = _Move(key);
			Value = value;
		}
		KeyValuePair(const KeyValuePair<TKey, TValue> & _that)
		{
			Key = _that.Key;
			Value = _that.Value;
		}
		KeyValuePair(KeyValuePair<TKey, TValue> && _that)
		{
			operator=(_Move(_that));
		}
		KeyValuePair & operator=(KeyValuePair<TKey, TValue> && that)
		{
			Key = _Move(that.Key);
			Value = _Move(that.Value);
			return *this;
		}
		KeyValuePair & operator=(const KeyValuePair<TKey, TValue> & that)
		{
			Key = that.Key;
			Value = that.Value;
			return *this;
		}
		int GetHashCode()
		{
			return GetHashCode(Key);
		}
	};

	template<typename TKey, typename TValue>
	inline KeyValuePair<TKey, TValue> KVPair(const TKey & k, const TValue & v)
	{
		return KeyValuePair<TKey, TValue>(k, v);
	}

	const float MaxLoadFactor = 0.7f;

	template<typename TKey, typename TValue>
	class Dictionary
	{
		friend class Iterator;
		friend class ItemProxy;
	private:
		inline int GetProbeOffset(int /*probeId*/) const
		{
			// quadratic probing
			return 1;
		}
	private:
		int bucketSizeMinusOne;
		int _count;
		IntSet marks;
		KeyValuePair<TKey, TValue>* hashMap;
		void Free()
		{
			if (hashMap)
				delete[] hashMap;
			hashMap = 0;
		}
		inline bool IsDeleted(int pos) const
		{
			return marks.Contains((pos << 1) + 1);
		}
		inline bool IsEmpty(int pos) const
		{
			return !marks.Contains((pos << 1));
		}
		inline void SetDeleted(int pos, bool val)
		{
			if (val)
				marks.Add((pos << 1) + 1);
			else
				marks.Remove((pos << 1) + 1);
		}
		inline void SetEmpty(int pos, bool val)
		{
			if (val)
				marks.Remove((pos << 1));
			else
				marks.Add((pos << 1));
		}
		struct FindPositionResult
		{
			int ObjectPosition;
			int InsertionPosition;
			FindPositionResult()
			{
				ObjectPosition = -1;
				InsertionPosition = -1;
			}
			FindPositionResult(int objPos, int insertPos)
			{
				ObjectPosition = objPos;
				InsertionPosition = insertPos;
			}

		};
		inline int GetHashPos(TKey& key) const
		{
			return ((unsigned int)(GetHashCode(key) * 2654435761)) % bucketSizeMinusOne;
		}
		FindPositionResult FindPosition(const TKey& key) const
		{
			int hashPos = GetHashPos(const_cast<TKey&>(key));
			int insertPos = -1;
			int numProbes = 0;
			while (numProbes <= bucketSizeMinusOne)
			{
				if (IsEmpty(hashPos))
				{
					if (insertPos == -1)
						return FindPositionResult(-1, hashPos);
					else
						return FindPositionResult(-1, insertPos);
				}
				else if (IsDeleted(hashPos))
				{
					if (insertPos == -1)
						insertPos = hashPos;
				}
				else if (hashMap[hashPos].Key == key)
				{
					return FindPositionResult(hashPos, -1);
				}
				numProbes++;
				hashPos = (hashPos + GetProbeOffset(numProbes)) & bucketSizeMinusOne;
			}
			if (insertPos != -1)
				return FindPositionResult(-1, insertPos);
			throw InvalidOperationException("Hash map is full. This indicates an error in Key::Equal or Key::GetHashCode.");
		}
		TValue & _Insert(KeyValuePair<TKey, TValue>&& kvPair, int pos)
		{
			hashMap[pos] = _Move(kvPair);
			SetEmpty(pos, false);
			SetDeleted(pos, false);
			return hashMap[pos].Value;
		}
		void Rehash()
		{
			if (bucketSizeMinusOne == -1 || _count >= int(MaxLoadFactor * bucketSizeMinusOne))
			{
				int newSize = (bucketSizeMinusOne + 1) * 2;
				if (newSize == 0)
				{
					newSize = 16;
				}
				Dictionary<TKey, TValue> newDict;
				newDict.bucketSizeMinusOne = newSize - 1;
				newDict.hashMap = new KeyValuePair<TKey, TValue>[newSize];
				newDict.marks.SetMax(newSize * 2);
				if (hashMap)
				{
					for (auto & kvPair : *this)
					{
						newDict.Add(_Move(kvPair));
					}
				}
				*this = _Move(newDict);
			}
		}

		bool AddIfNotExists(KeyValuePair<TKey, TValue>&& kvPair)
		{
			Rehash();
			auto pos = FindPosition(kvPair.Key);
			if (pos.ObjectPosition != -1)
				return false;
			else if (pos.InsertionPosition != -1)
			{
				_count++;
				_Insert(_Move(kvPair), pos.InsertionPosition);
				return true;
			}
			else
				throw InvalidOperationException("Inconsistent find result returned. This is a bug in Dictionary implementation.");
		}
		void Add(KeyValuePair<TKey, TValue>&& kvPair)
		{
			if (!AddIfNotExists(_Move(kvPair)))
				throw KeyExistsException("The key already exists in Dictionary.");
		}
		TValue& Set(KeyValuePair<TKey, TValue>&& kvPair)
		{
			Rehash();
			auto pos = FindPosition(kvPair.Key);
			if (pos.ObjectPosition != -1)
				return _Insert(_Move(kvPair), pos.ObjectPosition);
			else if (pos.InsertionPosition != -1)
			{
				_count++;
				return _Insert(_Move(kvPair), pos.InsertionPosition);
			}
			else
				throw InvalidOperationException("Inconsistent find result returned. This is a bug in Dictionary implementation.");
		}
	public:
		class Iterator
		{
		private:
			const Dictionary<TKey, TValue> * dict;
			int pos;
		public:
			KeyValuePair<TKey, TValue> & operator *() const
			{
				return dict->hashMap[pos];
			}
			KeyValuePair<TKey, TValue> * operator ->() const
			{
				return dict->hashMap + pos;
			}
			Iterator & operator ++()
			{
				if (pos > dict->bucketSizeMinusOne)
					return *this;
				pos++;
				while (pos <= dict->bucketSizeMinusOne && (dict->IsDeleted(pos) || dict->IsEmpty(pos)))
				{
					pos++;
				}
				return *this;
			}
			Iterator operator ++(int)
			{
				Iterator rs = *this;
				operator++();
				return rs;
			}
			bool operator != (const Iterator & _that) const
			{
				return pos != _that.pos || dict != _that.dict;
			}
			bool operator == (const Iterator & _that) const
			{
				return pos == _that.pos && dict == _that.dict;
			}
			Iterator(const Dictionary<TKey, TValue> * _dict, int _pos)
			{
				this->dict = _dict;
				this->pos = _pos;
			}
			Iterator()
			{
				this->dict = 0;
				this->pos = 0;
			}
		};

		Iterator begin() const
		{
			int pos = 0;
			while (pos < bucketSizeMinusOne + 1)
			{
				if (IsEmpty(pos) || IsDeleted(pos))
					pos++;
				else
					break;
			}
			return Iterator(this, pos);
		}
		Iterator end() const
		{
			return Iterator(this, bucketSizeMinusOne + 1);
		}
	public:
		void Add(const TKey & key, const TValue & value)
		{
			Add(KeyValuePair<TKey, TValue>(key, value));
		}
		void Add(TKey && key, TValue && value)
		{
			Add(KeyValuePair<TKey, TValue>(_Move(key), _Move(value)));
		}
		bool AddIfNotExists(const TKey & key, const TValue & value)
		{
			return AddIfNotExists(KeyValuePair<TKey, TValue>(key, value));
		}
		bool AddIfNotExists(TKey && key, TValue && value)
		{
			return AddIfNotExists(KeyValuePair<TKey, TValue>(_Move(key), _Move(value)));
		}
		void Remove(const TKey & key)
		{
			if (_count == 0)
				return;
			auto pos = FindPosition(key);
			if (pos.ObjectPosition != -1)
			{
				SetDeleted(pos.ObjectPosition, true);
				_count--;
			}
		}
		void Clear()
		{
			_count = 0;

			marks.Clear();
		}

        TValue* TryGetValueOrAdd(const TKey& key, const TValue& value)
        {
            Rehash();
            auto pos = FindPosition(key);
            if (pos.ObjectPosition != -1)
            {
                return &hashMap[pos.ObjectPosition].Value;
            }
            else if (pos.InsertionPosition != -1)
            {
                // Make pair
                KeyValuePair<TKey, TValue> kvPair(_Move(key), _Move(value));
                _count++;
                _Insert(_Move(kvPair), pos.InsertionPosition);
                return nullptr;
            }
            else
                throw InvalidOperationException("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }

		bool ContainsKey(const TKey& key) const
		{
			if (bucketSizeMinusOne == -1)
				return false;
			auto pos = FindPosition(key);
			return pos.ObjectPosition != -1;
		}
		bool TryGetValue(const TKey& key, TValue& value) const
		{
			if (bucketSizeMinusOne == -1)
				return false;
			auto pos = FindPosition(key);
			if (pos.ObjectPosition != -1)
			{
				value = hashMap[pos.ObjectPosition].Value;
				return true;
			}
			return false;
		}
		TValue* TryGetValue(const TKey& key) const
		{
			if (bucketSizeMinusOne == -1)
				return nullptr;
			auto pos = FindPosition(key);
			if (pos.ObjectPosition != -1)
			{
				return &hashMap[pos.ObjectPosition].Value;
			}
			return nullptr;
		}

		class ItemProxy
		{
		private:
			const Dictionary<TKey, TValue> * dict;
			TKey key;
		public:
			ItemProxy(const TKey& _key, const Dictionary<TKey, TValue>* _dict)
			{
				this->dict = _dict;
				this->key = _key;
			}
			ItemProxy(TKey&& _key, const Dictionary<TKey, TValue>* _dict)
			{
				this->dict = _dict;
				this->key = _Move(_key);
			}
			TValue & GetValue() const
			{
				auto pos = dict->FindPosition(key);
				if (pos.ObjectPosition != -1)
				{
					return dict->hashMap[pos.ObjectPosition].Value;
				}
				else
					throw KeyNotFoundException("The key does not exists in dictionary.");
			}
			inline TValue & operator()() const
			{
				return GetValue();
			}
			operator TValue&() const
			{
				return GetValue();
			}
			TValue & operator = (const TValue & val) const
			{
				return ((Dictionary<TKey, TValue>*)dict)->Set(KeyValuePair<TKey, TValue>(_Move(key), val));
			}
			TValue & operator = (TValue && val) const
			{
				return ((Dictionary<TKey, TValue>*)dict)->Set(KeyValuePair<TKey, TValue>(_Move(key), _Move(val)));
			}
		};
		ItemProxy operator [](const TKey & key) const
		{
			return ItemProxy(key, this);
		}
		ItemProxy operator [](TKey && key) const
		{
			return ItemProxy(_Move(key), this);
		}
		int Count() const
		{
			return _count;
		}
	private:
		template<typename... Args>
		void Init(const KeyValuePair<TKey, TValue> & kvPair, Args... args)
		{
			Add(kvPair);
			Init(args...);
		}
	public:
		Dictionary()
		{
			bucketSizeMinusOne = -1;
			_count = 0;
			hashMap = nullptr;
		}
		template<typename Arg, typename... Args>
		Dictionary(Arg arg, Args... args)
		{
			Init(arg, args...);
		}
		Dictionary(const Dictionary<TKey, TValue>& other)
			: bucketSizeMinusOne(-1), _count(0), hashMap(nullptr)
		{
			*this = other;
		}
		Dictionary(Dictionary<TKey, TValue>&& other)
			: bucketSizeMinusOne(-1), _count(0), hashMap(nullptr)
		{
			*this = (_Move(other));
		}
		Dictionary<TKey, TValue>& operator = (const Dictionary<TKey, TValue>& other)
		{
			if (this == &other)
				return *this;
			Free();
			bucketSizeMinusOne = other.bucketSizeMinusOne;
			_count = other._count;
			hashMap = new KeyValuePair<TKey, TValue>[other.bucketSizeMinusOne + 1];
			marks = other.marks;
			for (int i = 0; i <= bucketSizeMinusOne; i++)
				hashMap[i] = other.hashMap[i];
			return *this;
		}
		Dictionary<TKey, TValue> & operator = (Dictionary<TKey, TValue>&& other)
		{
			if (this == &other)
				return *this;
			Free();
			bucketSizeMinusOne = other.bucketSizeMinusOne;
			_count = other._count;
			hashMap = other.hashMap;
			marks = _Move(other.marks);
			other.hashMap = 0;
			other._count = 0;
			other.bucketSizeMinusOne = -1;
			return *this;
		}
		~Dictionary()
		{
			Free();
		}
	};

	class _DummyClass
	{};

	template<typename T, typename DictionaryType>
	class HashSetBase
	{
	protected:
		DictionaryType dict;
	private:
		template<typename... Args>
		void Init(const T & v, Args... args)
		{
			Add(v);
			Init(args...);
		}
	public:
		HashSetBase()
		{}
		template<typename Arg, typename... Args>
		HashSetBase(Arg arg, Args... args)
		{
			Init(arg, args...);
		}
		HashSetBase(const HashSetBase & set)
		{
			operator=(set);
		}
		HashSetBase(HashSetBase && set)
		{
			operator=(_Move(set));
		}
		HashSetBase & operator = (const HashSetBase & set)
		{
			dict = set.dict;
			return *this;
		}
		HashSetBase & operator = (HashSetBase && set)
		{
			dict = _Move(set.dict);
			return *this;
		}
	public:
		class Iterator
		{
		private:
			typename DictionaryType::Iterator iter;
		public:
			Iterator() = default;
			T & operator *() const
			{
				return (*iter).Key;
			}
			T * operator ->() const
			{
				return &(*iter).Key;
			}
			Iterator & operator ++()
			{
				++iter;
				return *this;
			}
			Iterator operator ++(int)
			{
				Iterator rs = *this;
				operator++();
				return rs;
			}
			bool operator != (const Iterator & _that) const
			{
				return iter != _that.iter;
			}
			bool operator == (const Iterator & _that) const
			{
				return iter == _that.iter;
			}
			Iterator(const typename DictionaryType::Iterator & _iter)
			{
				this->iter = _iter;
			}
		};
		Iterator begin() const
		{
			return Iterator(dict.begin());
		}
		Iterator end() const
		{
			return Iterator(dict.end());
		}
	public:
		int Count() const
		{
			return dict.Count();
		}
		void Clear()
		{
			dict.Clear();
		}
		bool Add(const T& obj)
		{
			return dict.AddIfNotExists(obj, _DummyClass());
		}
		bool Add(T && obj)
		{
			return dict.AddIfNotExists(_Move(obj), _DummyClass());
		}
		void Remove(const T & obj)
		{
			dict.Remove(obj);
		}
		bool Contains(const T & obj) const
		{
			return dict.ContainsKey(obj);
		}
	};
	template <typename T>
	class HashSet : public HashSetBase<T, Dictionary<T, _DummyClass>>
	{};
}

#endif
