#pragma once

#include "bricks/config.h"

#if !BRICKS_CONFIG_STL
#error libbricks must be configured to use the STL
#endif

#include "bricks/collections/deque_internal.h"

namespace Bricks { namespace Collections {
	template<typename T>
	class Stack : public Deque<T>
	{
	public:
		Stack(ValueComparison<T>* comparison = autonew OperatorValueComparison<T>()) : Deque<T>(comparison) { }
		Stack(const Stack<T>& stack, ValueComparison<T>* comparison = autonew OperatorValueComparison< T >()) : Deque<T>(stack, comparison) { }
		Stack(Iterable<T>* iterable, ValueComparison<T>* comparison = autonew OperatorValueComparison<T>()) : Deque<T>(iterable, comparison) { }

		virtual void Push(const T& value) { this->queue.push_front(value); }
		virtual void Pop() { if (this->queue.empty()) BRICKS_FEATURE_RELEASE_THROW(QueueEmptyException()); this->queue.pop_front(); }
		virtual T PopItem() { if (this->queue.empty()) BRICKS_FEATURE_RELEASE_THROW(QueueEmptyException()); T value = this->queue.front(); this->queue.pop_front(); return value; }
		virtual T& Peek() { if (this->queue.empty()) BRICKS_FEATURE_RELEASE_THROW(QueueEmptyException()); return this->queue.front(); }
		virtual const T& Peek() const { if (this->queue.empty()) BRICKS_FEATURE_RELEASE_THROW(QueueEmptyException()); return this->queue.front(); }
	};
} }
