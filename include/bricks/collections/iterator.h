#pragma once

#include "bricks/core/object.h"
#include "bricks/core/returnpointer.h"
#include "bricks/core/exception.h"

namespace Bricks {
	template<typename T> class Delegate;
}

namespace Bricks { namespace Collections {
	namespace Internal {
		class IteratorBase { };
		class IterableBase { };
		class IterableFastBase { };
	}

	class InvalidIteratorException : public Exception
	{
	public:
		InvalidIteratorException(const String& message = String::Empty) : Exception(message) { }
	};

	template<typename T> class Collection;

	template<typename T>
	class Iterator : public Object, public Internal::IteratorBase
	{
	public:
		typedef T IteratorType;

		virtual T& GetCurrent() const = 0;
		virtual bool MoveNext() = 0;
		virtual ReturnPointer< Collection< T > > GetAllObjects() { BRICKS_FEATURE_THROW(NotImplementedException()); };
	};

	template<typename T>
	class Iterable : public Internal::IterableBase
	{
	public:
		typedef T IteratorType;

		virtual ReturnPointer< Iterator< IteratorType > > GetIterator() const = 0;

		void Iterate(const Delegate<bool(IteratorType&)>& delegate) const;
		void Iterate(const Delegate<void(IteratorType&)>& delegate) const;
	};

	template<typename T>
	class IterableFast : public Internal::IterableFastBase
	{
	public:
		typedef T IteratorFastType;

		virtual T GetIteratorFast() const = 0;
	};
} }

namespace Bricks { namespace Collections { namespace Internal {
	template<typename T>
	struct IteratorType
	{
		template<typename U> IteratorType(const U& list, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableBase, U>::Value>::Type* dummy = NULL) : state(true), iter(list.GetIterator()) { }
		template<typename U> IteratorType(const U* list, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableBase, U>::Value>::Type* dummy = NULL) : state(true), iter(list->GetIterator()) { }
		template<typename U> IteratorType(const Pointer<U>& list, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableBase, U>::Value>::Type* dummy = NULL) : state(true), iter(list->GetIterator()) { }

		template<typename U> IteratorType(const U& iter, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, U>::Value>::Type* dummy = NULL) : state(true), iter(&iter) { }
		template<typename U> IteratorType(const U* iter, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, U>::Value>::Type* dummy = NULL) : state(true), iter(iter) { }
		template<typename U> IteratorType(const Pointer<U>& iter, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, U>::Value>::Type* dummy = NULL) : state(true), iter(iter) { }

		bool state;
		AutoPointer<Iterator<T> > iter;
		inline bool MoveNext() const { return iter->MoveNext(); }
		inline T& GetCurrent() const { return iter->GetCurrent(); }
	};
	template<typename T>
	struct IteratorFastType
	{
		template<typename U> IteratorFastType(const U& list, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableFastBase, U>::Value>::Type* dummy = NULL) : state(true), iter(list.GetIteratorFast()) { }
		template<typename U> IteratorFastType(const U* list, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableFastBase, U>::Value>::Type* dummy = NULL) : state(true), iter(list->GetIteratorFast()) { }
		template<typename U> IteratorFastType(const Pointer<U>& list, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableFastBase, U>::Value>::Type* dummy = NULL) : state(true), iter(list->GetIteratorFast()) { }

		template<typename U> IteratorFastType(const U& iter, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, U>::Value>::Type* dummy = NULL) : state(true), iter(iter) { }
		template<typename U> IteratorFastType(const U* iter, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, U>::Value>::Type* dummy = NULL) : state(true), iter(*iter) { }
		template<typename U> IteratorFastType(const Pointer<U>& iter, typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, U>::Value>::Type* dummy = NULL) : state(true), iter(*iter) { }

		bool state;
		T iter;
		inline bool MoveNext() const { return const_cast<T&>(iter).MoveNext(); }
		inline typename T::IteratorType& GetCurrent() const { return iter.GetCurrent(); }
	};

	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableBase, T>::Value && !SFINAE::IsCompatibleType<IterableFastBase, T>::Value, IteratorType<typename T::IteratorType> >::Type IteratorContainerType(const T& t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableBase, T>::Value && !SFINAE::IsCompatibleType<IterableFastBase, T>::Value, IteratorType<typename T::IteratorType> >::Type IteratorContainerType(const T* t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableBase, T>::Value && !SFINAE::IsCompatibleType<IterableFastBase, T>::Value, IteratorType<typename T::IteratorType> >::Type IteratorContainerType(const Pointer<T>& t);

	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, T>::Value && SFINAE::IsSameType<Iterator<typename T::IteratorType>, T>::Value, IteratorType<T> >::Type IteratorContainerType(const T& t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, T>::Value && !SFINAE::IsSameType<Iterator<typename T::IteratorType>, T>::Value, IteratorFastType<T> >::Type IteratorContainerType(const T& t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, T>::Value, IteratorType<typename T::IteratorType> >::Type IteratorContainerType(const T* t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IteratorBase, T>::Value, IteratorType<typename T::IteratorType> >::Type IteratorContainerType(const Pointer<T>& t);

	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableFastBase, T>::Value, IteratorFastType<typename T::IteratorFastType> >::Type IteratorContainerType(const T& t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableFastBase, T>::Value, IteratorFastType<typename T::IteratorFastType> >::Type IteratorContainerType(const T* t);
	template<typename T> static typename SFINAE::EnableIf<SFINAE::IsCompatibleType<IterableFastBase, T>::Value, IteratorFastType<typename T::IteratorFastType> >::Type IteratorContainerType(const Pointer<T>& t);
} } }
#define BRICKS_FOR_EACH(val, list) for (typeof(Bricks::Collections::Internal::IteratorContainerType(list)) __bricks_iter(list); __bricks_iter.MoveNext() && __bricks_iter.state;) if (!(__bricks_iter.state = false)) for (val = __bricks_iter.GetCurrent(); !__bricks_iter.state; __bricks_iter.state = true)

namespace Bricks { namespace Collections {
	template<typename T> inline void Iterable<T>::Iterate(const Delegate<bool(Iterable<T>::IteratorType&)>& delegate) const {
		BRICKS_FOR_EACH (IteratorType& t, *this) {
			if (!delegate.Call(t))
				break;
		}
	}

	template<typename T> inline void Iterable<T>::Iterate(const Delegate<void(Iterable<T>::IteratorType&)>& delegate) const {
		BRICKS_FOR_EACH (IteratorType& t, *this)
			delegate.Call(t);
	}
} }

#define foreach BRICKS_FOR_EACH
