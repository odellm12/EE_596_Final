#ifndef LIST_H
#define LIST_H

#ifndef WINNT
extern "C" {
#endif
#include <stdio.h>
#ifndef WINNT
};
#endif

template <class T>
class ListNode
{
    //  friend List<T>;

public:
    T* prev, * next;

    ListNode();
    ~ListNode();
};

template<class T>
class List
{
public:
    T* first, * last;
    int length;

    List();
    ~List();
    void addBefore(T* ptr, T* place);
    void addAfter(T* ptr, T* place);
    void addFirst(T* ptr);
    void addLast(T* ptr);
    void deleteNode(T* ptr);
    void deleteNodes();
    void unchain(T* ptr);
    int empty() { return (first == NULL); }
    void reverseList();
    void swap(T* ptr1, T* ptr2);
    void sort(int (*comp)(T*, T*));
    void quickSort(int (*comp)(T*, T*), T* mPtr, T* nPtr, int m, int n);
    void append(List<T>* list);
};

#endif
