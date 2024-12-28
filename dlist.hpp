#pragma once
/*
  dlist.hpp
  Doubly-linked lists of ints
 */
#include <vector>

/* dlist
   Doubly-linked list class. 
*/
class dlist {
  public:
    struct node
    {
        int value;
        node* next;
        node* prev;
    };
  
    /* head(), tail()
       Returns the head/tail nodes of the list. These functions are provided
       for you.
    */
    node* head() const { return hd; }
    node* tail() const { return tl; }

    /* constructor
       You do not need to add any code to the constructor, since lists start
       as empty.
    */
    dlist() { }

    // **************** Implement ALL the following functions ****************

    /* destructor
       Delete all nodes in the list.

       Must run in O(n) time.
    */
    ~dlist() 
    {
      while(tl != nullptr && hd != nullptr)
         {
            pop_back();
         }
    }
    /* copy constructor
       Construct a copy of the `original` list. The copy must not share any
       node pointers with yeahthe original list.

       Must run in O(n) time.
    */
    dlist(const dlist& original)
    {
      node* o = original.hd;
      for(int i = 0; i < original.size(); ++i)
      {
         push_back(o->value);
         o = o->next;
      }
    }
     
    /* a = b  (copy assignment)
       Copy all values in `b` into `a`. After the copy, `a` and `b` must not
       share any node pointers. 

       Must run in O(n) time.
    */
    dlist& operator= (const dlist& original)
    {
      if(this != &original)
      {
         while(tl != nullptr)
            {
               this->pop_back();
            }
         node* o = original.hd;
         for(int i = 0; i < original.size(); ++i)
         {
            push_back(o->value);
            o = o->next;
         }
      }
      return *this;
    }

    /* at(i)
       Returns a pointer to the i-th node (head = index 0, tail = size() - 1).
       If i < 0, return the head. If i >= size, return the tail.
       
       Must run in O(i) time.
    */
    node* at(int i) const
    { 
      if(i < 0) return hd;
      if(i >=sz) return tl;

      node* n;
      if(i < sz / 2)
      {
         n = hd;
         for(int j = 0; j < i && n != nullptr; ++j)
         {
            n = n->next;
         }
      }
      else 
      {
         n = tl;
         for(int j = sz - 1; j > i && n != nullptr; --j)
         {
            n = n->prev;
         }
      }
      return n;
    }

    /* insert(a, value)
       Insert a new node after `a`, containing the `value`. If 
       `a == nullptr` then insert the new node *before* the head.

       Must run in O(1) time.
    */
    void insert(node *a, int value)
    {
      node* n = new node;
      n->value = value;
      if(hd == nullptr && tl == nullptr) //list is empty
      {
         //new node is head and tail
         hd = n;
         tl = n;
         n->next = nullptr;
         n->prev = nullptr;
      }
      else if(a == nullptr)//inserting at the beginning before head
      {
         //New node is the new head
         n->next = hd;
         n->prev = nullptr;
         hd->prev = n;
         hd = n;
      }
      else //if not inserted at beginning(*)
      {
         n->next = a->next; //n's next = b
         n->prev = a;

         if(a->next != nullptr)//in middle of list
         {
            a->next->prev = n;        
         }    
         a->next = n;  

         if(a == tl) //if at very end of list
         {
            tl = n;
         }
      }
      ++sz;
   }
    /* erase(which)
       Delete the given node. If which == nullptr, then this function does 
       nothing.

       Must run in O(1) time.
    */
    void erase(node* which)
    {
      if(which == nullptr){ //which does not exist
         return;
      }

      if(which->prev != nullptr  && which->next !=nullptr) //which is in the middle
      {
         which->prev->next = which->next;
         which->next->prev = which->prev;
         delete which;
      }
      else if(which == hd && which->next != nullptr){ //which is the head
         hd = which->next;
         hd->prev = nullptr;
         delete which;
      }
      else if(which == tl && which->prev != nullptr) 
      {
         tl = which->prev;
         tl->next = nullptr;
         delete which;
      }
      else if(which->prev == nullptr && which->next == nullptr)
         {
            hd = which->next;
            tl = which->prev;
            delete which;
         }
         --sz;
    }
    /* erase(i)
       Erase the node at index i (indexes work as for at()). If i < 0, 
       erase the head, if i >= size(), erase the tail.

       Must run in O(i) time.
    */
    void erase(int i)
    {
      erase(at(i));
    }

    /* remove(x)
       Remove the *first* node whose value is `x`. If `x` does not occur in the 
       list, then nothing should be removed. If multiple nodes have `x` as
       their values, only the first (closest to the head) should be removed.

       Must run in O(n) time.
    */
    void remove(int x)
    {
      for(int i = 0; i < size(); ++i)
      {
         node* n = at(i);
         if(n->value == x)
         {
            erase(n);
            break;
         }
      }
    }

    /* push_back(value)
       Add a new node containing value at the *end* of the list. 
       Must run in O(1) time.
    */
    void push_back(int value)
    {
      insert(tl,value);
    }

    /* push_front(value)
       Add a new node containing value before the head of the list. 

       Must run in O(1) time.
    */
    void push_front(int value)
    {
      insert(nullptr, value);
    }

    /* pop_front()
       Remove the first (head) element. If the list is empty, do nothing.

       Must run in O(1) time.
    */
    void pop_front() 
    {
      erase(hd);
    }

    /* pop_back()
       Remove the last (tail) element. If the list is empty, do nothing.

       Must run in O(1) time.
    */
    void pop_back()
    {
      erase(tl);
    }

    /* size()
       Returns the size of the list. 

       Must run in O(n) in the worst case.
    */
    int size() const
    {
      return sz;
    }

    /* empty()
       Returns true if the list is empty.
    
       Must run in O(1) time.
    */
    bool empty() const
    {
      return size() == 0;
    }

    /* to_vector()
       Converts the list to a vector containing the same values and returns it.

       Must run in O(n) time.
    */
    std::vector<int> to_vector() const
    {
      std::vector<int> vec;
      node* current = hd;
      while(current != nullptr)
      {
         vec.push_back(current->value);
         current = current->next;
      }
      return vec;
    }

  private:
    node* hd = nullptr; 
    node* tl = nullptr; 
    int sz = 0;
    // Add any other private members you need
    
};
