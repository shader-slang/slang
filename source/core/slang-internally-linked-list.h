// slang-internally-linked-list.h
#ifndef SLANG_INTERNALLY_LINKED_LIST_H
#define SLANG_INTERNALLY_LINKED_LIST_H

// This file provides support for the idiom of a linked
// list of values where the "next" pointer is stored in
// the values themselves (thus requiring no additional
// allocation for list nodes, at the price of any given
// value only being able to appear in a single list).

#include "slang-basic.h"

namespace Slang
{

/// A linked list where the elements are themselves the nodes.
///
/// The type parameter `T` should be a type that publicly
/// inherits from `InternallyLinkedList<T>::Node`.
///
template<typename T>
struct InternallyLinkedList
{
public:
    struct Node
    {
    public:
        Node() {}

    private:
        friend struct InternallyLinkedList<T>;
        T* _next = nullptr;
    };

    struct Iterator
    {
    public:
        Iterator() {}

        Iterator(T* node)
            : _node(node)
        {
        }

        T* operator*() const { return _node; }

        void operator++() { _node = static_cast<Node const*>(_node)->_next; }

        bool operator!=(Iterator const& that) const { return _node != that._node; }

    private:
        T* _node = nullptr;
    };

    Iterator begin() { return Iterator(_first); }

    Iterator end() { return Iterator(); }

    T* getFirst() const { return _first; }

    T* getLast() const { return _last; }

    void add(T* element)
    {
        SLANG_ASSERT(element != nullptr);
        if (!_last)
        {
            SLANG_ASSERT(_first == nullptr);

            _first = element;
            _last = element;
        }
        else
        {
            SLANG_ASSERT(_first != nullptr);

            _last->_next = element;
            _last = element;
        }
    }

    void insertAfter(T* existingElement, T* newElement)
    {
        SLANG_ASSERT(existingElement != nullptr);
        SLANG_ASSERT(newElement != nullptr);
        if (existingElement == _last)
        {
            add(newElement);
        }
        else
        {
            newElement->_next = existingElement->_next;
            existingElement->_next = newElement;
        }
    }

    void append(InternallyLinkedList<T> const& other)
    {
        if (!other._first)
        {
        }
        else if (!_last)
        {
            _first = other._first;
            _last = other._last;
        }
        else
        {
            SLANG_ASSERT(_first != nullptr);

            _last->_next = other._first;
            _last = other._last;
        }
    }

private:
    T* _first = nullptr;
    T* _last = nullptr;
};

template<typename T>
using InternallyLinkedListNode = InternallyLinkedList<T>::Node;

} // namespace Slang

#endif
