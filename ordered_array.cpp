/*
 * ordered_array.hpp
 * Definition of the ordered_array class. You can either create a separate 
 * ordered_array.cpp and put the method definitions there, or replace the 
 * method declarations below with definitions. E.g., replace
 *
 * 
 * In this assignment, you’ll implement a simple data structure which maintains a sorted array. 
 * This is essentially just a simple wrapper around a dynamically-allocated array of int (you can also use a vector)
 *  which allows the user to insert and remove values. 
 * The elements of the array should always be sorted in ascending order; 
 * this means that when you add new elements, you need to figure out where they go in the array, 
 * shift everything after them up, and then drop the new element into place. 
 * Likewise, when you remove an element, you have to find it, and then shift everything after it down.


 *     int size();
 *
 * with 
 * 
 *     int size() 
 *     {
 *         // Your code here...
 *     }
 *
 * and similarly for all the other functions
 */

#include "ordered_array.hpp"
#include<utility> //for std::swap
#include<stdexcept> //for std::out_of_range
#include<algorithm> // for sorting vector

    ordered_array::ordered_array(int cap)
    {
      max = cap;
      sz = 0;
      data.resize(max);
    }

    /* size()
       Returns the size (number of elements in the array).
    */

    int ordered_array::size() 
    {
      return sz;
    }

    /* capacity()
       Returns the maximum size of the array.
    */

    int ordered_array::capacity() const
    {
      return max;
    }

    /* insert(e)
       Insert e into the array. Note that it is OK to insert duplicates; if n 
       copies of a value are inserted into the array then n copies should appear
       in the array.

       If size() == capacity() then this does nothing.

       If e == -2147483648 then this does nothing (i.e., -2147483648 is not a
       valid value to insert).
    */
    void ordered_array::insert(int elem)
    {
      if((size() < max) && !(elem == -2147483648))
      {
         data[sz++] = elem;
         std::sort(data.begin(), data.begin() + sz);
      }
      
    }
    /* remove(e)
       Remove e from the array, if it exists. (If it does not exist, the
       array should be unchanged.) If multiple copies of e are present, only
       one should be removed.

       If e = -2147483648 then this does nothing.

    */
    void ordered_array::remove(int elem)
    {
      if(elem == -2147483648) return;
      
      //needs to use variables to keep track of place and reduce n time
      else if(exists(elem))
      {
         bool deleted = false;
         for(int i = 0; i < size(); ++i) 
         {  
            int ph = i+1;
            if((data[i] == elem) && (deleted == false)) {
               deleted = true;
               data[i] = data[ph];
            }
            else if(deleted == true)
            {
               data[i] = data[ph];
            }
         }

         if(deleted)
            sz--;
      }
    }

    /* exists(e)
       Returns true if e is present at least once in the array.

       If e == -2147483648 then this returns false.
    */
    bool ordered_array::exists(int elem)
    {
      for(int i = 0; i < size(); ++i) 
      {
         if(data[i] == elem) 
         {
            return true;
         }
      }
      return false;
    }

    /* at(i)
       Returns a *reference* to the element at index i. If i < 0 or i >= size(),
       then the function should throw a std::out_of_range exception (this is the
       same exception that std::vector::at would throw in this situation).

       Note that at() should *never* return -2147483648.
    */
    

    //Possibly redundant due to vectors at() function. Unsure though if it needs a reference symbol

    int& ordered_array::at(int i)
    {
      if(i >= 0 && i < size()) 
      {
         return data[i];
      }
      else 
         throw std::out_of_range("at[i] out of range!");
    }