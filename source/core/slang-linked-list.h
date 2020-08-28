#ifndef SLANG_CORE_LINKED_LIST_H
#define SLANG_CORE_LINKED_LIST_H

#include "../../slang.h"

#include "slang-allocator.h"

namespace Slang
{
template <typename T>
class LinkedList;

template <typename T>
class LinkedNode
{
    template <typename T1> friend class LinkedList;

private:
    LinkedNode<T>* prev = nullptr;
    LinkedNode<T>* next = nullptr;
    LinkedList<T>* list;

public:
    T Value;
    LinkedNode(LinkedList<T>* lnk)
        : list(lnk)
    {
    };
    LinkedNode<T>* GetPrevious() { return prev; };
    LinkedNode<T>* GetNext() { return next; };
    LinkedNode<T>* InsertAfter(const T& nData)
    {
        LinkedNode<T>* n = new LinkedNode<T>(list);
        n->Value = nData;
        n->prev = this;
        n->next = this->next;
        LinkedNode<T>* npp = n->next;
        if (npp)
        {
            npp->prev = n;
        }
        next = n;
        if (!n->next)
            list->tail = n;
        list->count++;
        return n;
    };
    LinkedNode<T>* InsertBefore(const T& nData)
    {
        LinkedNode<T>* n = new LinkedNode<T>(list);
        n->Value = nData;
        n->prev = prev;
        n->next = this;
        prev = n;
        LinkedNode<T>* npp = n->prev;
        if (npp)
            npp->next = n;
        if (!n->prev)
            list->head = n;
        list->count++;
        return n;
    };
    void Delete()
    {
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
        list->count--;
        if (list->head == this)
        {
            list->head = next;
        }
        if (list->tail == this)
        {
            list->tail = prev;
        }
        delete this;
    }
};

template <typename T> class LinkedList
{
    template <typename T1> friend class LinkedNode;

private:
    LinkedNode<T>*head, *tail;
    int count;

public:
    class Iterator
    {
    public:
        LinkedNode<T>*Current, *Next;
        void SetCurrent(LinkedNode<T>* cur)
        {
            Current = cur;
            if (Current)
                Next = Current->GetNext();
            else
                Next = 0;
        }
        Iterator() { Current = Next = 0; }
        Iterator(LinkedNode<T>* cur) { SetCurrent(cur); }
        T& operator*() const { return Current->Value; }
        Iterator& operator++()
        {
            SetCurrent(Next);
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator rs = *this;
            SetCurrent(Next);
            return rs;
        }
        bool operator!=(const Iterator& iter) const { return Current != iter.Current; }
        bool operator==(const Iterator& iter) const { return Current == iter.Current; }
    };
    Iterator begin() const { return Iterator(head); }
    Iterator end() const { return Iterator(0); }

public:
    LinkedList()
        : head(0)
        , tail(0)
        , count(0)
    {}
    ~LinkedList() { Clear(); }
    LinkedList(const LinkedList<T>& link)
        : head(0)
        , tail(0)
        , count(0)
    {
        this->operator=(link);
    }
    LinkedList(LinkedList<T>&& link)
        : head(0)
        , tail(0)
        , count(0)
    {
        this->operator=(_Move(link));
    }
    LinkedList<T>& operator=(LinkedList<T>&& link)
    {
        if (head != 0)
            Clear();
        head = link.head;
        tail = link.tail;
        count = link.count;
        link.head = 0;
        link.tail = 0;
        link.count = 0;
        for (auto node = head; node; node = node->GetNext())
            node->list = this;
        return *this;
    }
    LinkedList<T>& operator=(const LinkedList<T>& link)
    {
        if (head != 0)
            Clear();
        auto p = link.head;
        while (p)
        {
            AddLast(p->Value);
            p = p->GetNext();
        }
        return *this;
    }
    template <typename IteratorFunc> void ForEach(const IteratorFunc& f)
    {
        auto p = head;
        while (p)
        {
            f(p->Value);
            p = p->GetNext();
        }
    }
    LinkedNode<T>* GetNode(int x)
    {
        LinkedNode<T>* pCur = head;
        for (int i = 0; i < x; i++)
        {
            if (pCur)
                pCur = pCur->next;
            else
                SLANG_UNEXPECTED("Index out of range");
        }
        return pCur;
    };
    LinkedNode<T>* Find(const T& fData)
    {
        for (LinkedNode<T>* pCur = head; pCur; pCur = pCur->next)
        {
            if (pCur->Value == fData)
                return pCur;
        }
        return 0;
    };
    LinkedNode<T>* FirstNode() const { return head; };
    T& First() const
    {
        if (!head)
            SLANG_UNEXPECTED("LinkedList: index out of range.");
        return head->Value;
    }
    T& Last() const
    {
        if (!tail)
            SLANG_UNEXPECTED("LinkedList: index out of range.");
        return tail->Value;
    }
    LinkedNode<T>* LastNode() const { return tail; };
    LinkedNode<T>* AddLast(const T& nData)
    {
        LinkedNode<T>* n = new LinkedNode<T>(this);
        n->Value = nData;
        n->prev = tail;
        if (tail)
            tail->next = n;
        n->next = 0;
        tail = n;
        if (!head)
            head = n;
        count++;
        return n;
    };
    // Insert a blank node
    LinkedNode<T>* AddLast()
    {
        LinkedNode<T>* n = new LinkedNode<T>(this);
        n->prev = tail;
        if (tail)
            tail->next = n;
        n->next = 0;
        tail = n;
        if (!head)
            head = n;
        count++;
        return n;
    };
    LinkedNode<T>* AddFirst(const T& nData)
    {
        LinkedNode<T>* n = new LinkedNode<T>(this);
        n->Value = nData;
        n->prev = 0;
        n->next = head;
        if (head)
            head->prev = n;
        head = n;
        if (!tail)
            tail = n;
        count++;
        return n;
    };
    void Delete(LinkedNode<T>* n, int Count = 1)
    {
        LinkedNode<T>*n1, *n2 = 0, *tn;
        n1 = n->prev;
        tn = n;
        int numDeleted = 0;
        for (int i = 0; i < Count; i++)
        {
            n2 = tn->next;
            delete tn;
            tn = n2;
            numDeleted++;
            if (tn == 0)
                break;
        }
        if (n1)
            n1->next = n2;
        else
            head = n2;
        if (n2)
            n2->prev = n1;
        else
            tail = n1;
        count -= numDeleted;
    }
    void Clear()
    {
        for (LinkedNode<T>* n = head; n;)
        {
            LinkedNode<T>* tmp = n->next;
            delete n;
            n = tmp;
        }
        head = 0;
        tail = 0;
        count = 0;
    }
    List<T> ToList() const
    {
        List<T> rs;
        rs.Reserve(count);
        for (auto& item : *this)
        {
            rs.Add(item);
        }
        return rs;
    }
    int Count() { return count; }
};
} // namespace Slang
#endif
