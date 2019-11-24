#include "list.h"
#ifndef ListCodeIncluded
#define ListCodeIncluded

//#define Audit

#ifndef WINNT
extern "C" {
#endif
#include <stddef.h>
#ifndef WINNT
};
#endif

#include "list.h"

template<class T>
ListNode<T>::ListNode()
{
    prev = NULL;
    next = NULL;
}

template<class T>
ListNode<T>::~ListNode()
{
    prev = NULL;
    next = NULL;
}

template<class T>
List<T>::List()
{
    first = NULL;
    last = NULL;
    length = 0;
}

template<class T>
List<T>::~List()
{
    deleteNodes();
}

template<class T>
void List<T>::addFirst(T* ptr)
{
    ptr->next = first;
    ptr->prev = NULL;
    if (first != NULL) first->prev = ptr;
    first = ptr;
    if (last == NULL) last = ptr;
    length++;
}

template<class T>
void List<T>::addLast(T* ptr)
{
    ptr->prev = last;
    if (last != NULL) last->next = ptr;
    ptr->next = NULL;
    last = ptr;
    if (first == NULL) first = ptr;
    length++;
}

template<class T>
void List<T>::addAfter(T* ptr, T* place)
{
    if (place == NULL) addFirst(ptr); else
    {
        ptr->next = place->next;
        if (place->next != NULL)
            place->next->prev = ptr; else
            last = ptr;
        place->next = ptr;
        ptr->prev = place;
        length++;
    }
}

template<class T>
void List<T>::addBefore(T* ptr, T* place)
{
    if (place == NULL) addLast(ptr); else
    {
        ptr->prev = place->prev;
        if (place->prev != NULL)
            place->prev->next = ptr; else
            first = ptr;
        place->prev = ptr;
        ptr->next = place;
        length++;
    }
}

template<class T>
void List<T>::unchain(T* ptr)
{
#ifdef Audit
    T* tem = first;
    while (tem != NULL && tem != ptr) tem = tem->next;
    if (tem == NULL)
        exit(1);
#endif
    if (ptr->prev != NULL) ptr->prev->next = ptr->next; else
        first = ptr->next;
    if (ptr->next != NULL) ptr->next->prev = ptr->prev; else
        last = ptr->prev;
    ptr->prev = ptr->next = NULL;
    length--;
}

template<class T>
void List<T>::deleteNode(T* ptr)
{
    unchain(ptr);
    delete ptr;
}

template<class T>
void List<T>::deleteNodes()
{
    T* ptr = first;
    T* next;
    while (ptr != NULL)
    {
        next = ptr->next;
        deleteNode(ptr);
        ptr = next;
    }
}

//template<class T>
//int List<T>::empty()
//{
//  return (length==0);
//}

template<class T>
void List<T>::reverseList()
{
    T* ptr = first;
    while (ptr != NULL)
    {
        T* tem = ptr->prev;
        ptr->prev = ptr->next;
        ptr->next = tem;
        ptr = ptr->prev;
    }
    ptr = first;
    first = last;
    last = ptr;
}

template<class T>
void List<T>::swap(T* ptr1, T* ptr2)
// This function exchanges two nodes in a doubly linked list.  Its a mess.  :-)
// Don't bother trying to understand it, its beyond the capacity of Homo
// sapiens.
{
    T* ptr1Prev = ptr1->prev;
    T* ptr2Prev = ptr2->prev;
    T* ptr1Next = ptr1->next;
    T* ptr2Next = ptr2->next;
    T* ptr;

    if (ptr1Prev != NULL) ptr1Prev->next = ptr2;
    if (ptr1Next != NULL) ptr1Next->prev = ptr2;
    if (ptr2Prev != NULL) ptr2Prev->next = ptr1;
    if (ptr2Next != NULL) ptr2Next->prev = ptr1;
    ptr = ptr1->prev;
    ptr1->prev = ptr2->prev;
    ptr2->prev = ptr;
    ptr = ptr1->next;
    ptr1->next = ptr2->next;
    ptr2->next = ptr;
    if (first == ptr2) first = ptr1; else
        if (first == ptr1) first = ptr2;
    if (last == ptr2) last = ptr1; else
        if (last == ptr1) last = ptr2;
}

template<class T>
void List<T>::sort(int (*comp)(T*, T*))
// Run the quicksort procedure on the list
{
    if (length > 1) quickSort(comp, first, last, 0, length - 1);
}

template<class T>
void List<T>::quickSort(int (*comp)(T*, T*), T* mPtr, T* nPtr, int m, int n)
// This is the infamous quicksort procedure.  Its kind of strange looking,
// to deal with doubly linked lists, and because of these modifications
// probably doesn't work with the asymptotic time O(n log n), but I'm not
// sure on this.
{
    int i = m;
    int j = n;
    T* iPtr = mPtr;
    T* jPtr = nPtr;
    T* kPtr = mPtr;
#ifdef Debug
    printf("Sort %d %d,", m, n);
    mPtr->print();
    printf(" ");
    nPtr->print();
    printf("\n");
#endif
    if (m < n)      // If there are elements to sort
    {
        do {
            do {      // Shift pointers up and down the list
                i++;
                iPtr = iPtr->next;
            } while (i <= n && comp(kPtr, iPtr) > 0);
            while (j >= m && comp(jPtr, kPtr) > 0)
            {
                j--;
                jPtr = jPtr->prev;
            }
            if (i < j)  // Do we have to swap?
            {
                swap(iPtr, jPtr);     // Do it
                if (iPtr == nPtr) nPtr = jPtr; else if (jPtr == nPtr) nPtr = iPtr;
                if (iPtr == mPtr) mPtr = jPtr; else if (jPtr == mPtr) mPtr = iPtr;
                T* tem = iPtr;
                iPtr = jPtr;
                jPtr = tem;
#ifdef Debug
                printf("Loop: ");
                print();
#endif
            }
        } while (i < j);
        if (m < j)   // Yet another swap?
        {
            swap(mPtr, jPtr);     // Do it
            if (iPtr == nPtr) nPtr = jPtr; else if (jPtr == nPtr) nPtr = iPtr;
            if (iPtr == mPtr) mPtr = jPtr; else if (jPtr == mPtr) mPtr = iPtr;
            T* tem = mPtr;
            mPtr = jPtr;
            jPtr = tem;
#ifdef Debug
            printf("Swap: ");
            print();
#endif
            quickSort(comp, mPtr, jPtr->prev, m, j - 1);  // Sort one sublist
        }
        if (j < n) quickSort(comp, jPtr->next, nPtr, j + 1, n); // Sort other sublist
    }
#ifdef Debug
    printf("Done.\n");
#endif
}

template<class T>
void List<T>::append(List<T>* list)
{
    if (empty())
    {
        first = list->first;
        last = list->last;
    }
    else if (!list->empty())
    {
        last->next = list->first;
        list->first->prev = last;
        last = list->last;
    }
    list->first = NULL;
    list->last = NULL;
    length += list->length;
    list->length = 0;
}

#endif
