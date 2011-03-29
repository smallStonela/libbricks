#pragma once

#include "bricks/collections.h"

namespace Bricks {
	template<typename F> class Delegate;
}

namespace Bricks { namespace Collections { namespace Internal {
	struct IteratorTypeBase
	{
		mutable bool state;
		operator bool() const { return state; }
		IteratorTypeBase() : state(true) { }
	};
	template<typename T>
	struct IteratorType : public IteratorTypeBase
	{
		const T value;
		IteratorType(T const &value) : IteratorTypeBase(), value(value) { }
	};
	template<typename T>
	inline IteratorType<T*> IteratorContainer(T& t) { return IteratorType<T*>(&t); }
	template<typename T>
	inline T& IteratorContainer(const T& dummy, const IteratorTypeBase& t) { return *const_cast<T*&>(static_cast<const IteratorType<T*>&>(t).value); }
} } }
#define foreach_container Bricks::Collections::Internal::IteratorContainer
#define foreach_iterator(list) ((list).GetIterator())
#define foreach_basetype const Bricks::Collections::Internal::IteratorTypeBase&
#define foreach(val, list) for (foreach_basetype iter = foreach_container(foreach_iterator(list)); foreach_container(foreach_iterator(list), iter).MoveNext() && iter.state;) if (!(iter.state = false)) for (val = foreach_container(foreach_iterator(list), iter).GetCurrent(); !iter.state; iter.state = true)

namespace Bricks { namespace Collections {
	class InvalidIteratorException : public Exception
	{
	public:
		InvalidIteratorException(const String& message = String::Empty) : Exception(message) { }
	};

	template<typename T> class Collection;

	template<typename T>
	class Iterator
	{
	public:
		virtual T& GetCurrent() const = 0;
		virtual bool MoveNext() = 0;
		virtual Collection< T >& GetAllObjects() { Throw(NotImplementedException); };
	};

	template<typename T>
	class Iterable
	{
	public:
		virtual Iterator< T >& GetIterator() const = 0;

		void Iterate(const Delegate<bool(T&)>& delegate);
	};
} }
