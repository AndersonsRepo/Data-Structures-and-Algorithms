/*
 * assign1_test.cpp
 * CS 133 Assignment 1 test runner
 */

/* TODO list
   Randomize capacity of constructed arrays.
*/

#include <iostream>
#include <vector>
using std::cout;

#include <algorithm>
#include <functional>
#include <random>
#include <vector>

const int SENTINEL = -2147483648;

/* make_random_vector(len)
   Returns a vector<int> of random values, where each entry is between 0 and
   INT_MAX. The optional second parameter lets you specify the seed to be used 
   for the RNG.
*/
inline std::vector<int> make_random_vector(
    std::size_t len,
    int seed = 1) 
{
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution;
    auto gen = std::bind(distribution, generator);

    // Fill with random values
    std::vector<int> ret(len, 0);
    for(std::size_t i = 0; i < len; ++i)
        ret.at(i) = gen() % 100;

    return ret;
}

/* make_random_permutation(len)
   Returns a vector of length len containing a random permutation of the 
   integers 0...len-1. This can, of course, be used to randomly permute any
   vector of length len.
*/
inline std::vector<unsigned> make_random_permutation(
    std::size_t len,
    int seed = 1) 
{
    std::default_random_engine generator(seed);
    std::vector<unsigned> ret(len, 0);

    // Initialize vector to 0...len-1
    for(std::size_t i = 0; i < len; ++i) 
        ret.at(i) = i;

    std::shuffle(ret.begin(), ret.end(), generator);

    return ret;

}

#ifndef CHECK_SOLUTION

#include "ordered_array.hpp"

// Included twice to check for functions being defined in header files, non-use
// of #pragma once or include guard.
#include "ordered_array.hpp"

#else

#include "ordered_array_soln.hpp"

#endif

/* The basic tests that are performed are:
  1.  Construct an ordered_array with capacity 60. Check initial size and 
      capacity.
  2.  Inserting the integers -20...+20 in random order (checking the size after
      each insert, and using `exists` to make sure the newly inserted elements
      are present). 
  3.  Verify that exists() returns true for all values in -20...+20
  4.  Inserting duplicates of -20, 0, and 20. We then remove the duplicates,
      checking the size after each to make sure that the duplicates are really
      being added.
  5.  Remove the duplicates, again checking the size after each to make sure that
      `remove()` does not remove more than one element at a time.
  6.  Try to remove values 30, 31, ... which do not exist. Verify that size is
      unchanged.
  7.  Insert values 30, 31, ... until size() == capacity(). Verify that further
      elements are not inserted. 
  8.  Remove values 30, 31, ... 
  9.  Remove values -20...+20 in random order, checking the size after each. 
  10. Verify that size() == 0. 

  After each operation, we verify that the size is what is expected, and that
  the contents of the array are still sorted.
*/

// Helper for printing an ordered_array
void print_array(ordered_array& a) 
{
    for(int i = 0; i < a.size(); ++i) {
        std::cout << a.at(i);
        if(i != a.size()-1)
            std::cout << ", ";
    }
    cout << "\n";
}

bool is_sorted(ordered_array& a) 
{
    if(a.size() <= 1)
        return true;
    else {
        for(int i = 0; i < a.size() - 1; ++i)
            if(a.at(i) > a.at(i+1))
                return false;

        return true;
    }
}

bool hides_sentinel(ordered_array& a) 
{
    for(int i = 0; i < a.size(); ++i)
        if(a.at(i) == SENTINEL) {
            cout << "CHECK: .at(" << i << ") returns -2147483648.\n";
            return false;
        }

    return true;
}

// Returns number of copies of elem in a.
int count(ordered_array& a, int elem) 
{
    int c = 0;
    for(int i = 0; i < a.size(); ++i)
        if(a.at(i) == elem)
            c++;

    return c;
}

// Returns true if data and a have the same elements (same counts of all elements)
bool same_elements(const std::vector<int>& data, ordered_array& a) 
{
    for(int e : data) {
        int c = std::count(data.begin(), data.end(), e);
        if(count(a,e) != c)
            return false;
    }

    return true;
}

bool contains_elements(ordered_array& a, const std::vector<int>& d, int count) 
{
    for(int i = 0; i < count; ++i)
        if(!a.exists(d[i]))
            return false;

    return true;
}

/* test_initial()
   Perform tests on an empty ordered array.
*/
bool test_initial(ordered_array& a, int cap) 
{
    if(a.size() != 0) {
        cout << "CHECK: Size of empty array is not 0.\n";
        return false;
    }

    if(!hides_sentinel(a)) 
        return false;

    // Note that vector::reserve() theoretically can set capacity to anything
    // >= its parameter, but in practice, it tends to use the exact capacity
    // given to it.  
    if(a.capacity() != cap) {
        cout << "CHECK: Capacity of empty array is not " << cap << ".\n";
        return false;
    }

    // If the array is empty, accessing any .at() element should throw. 
    try {
        a.at(0);
        cout << "CHECK: Out-of-range at() must throw std::out_of_range (returned " << a.at(0) << ")\n";
        return false;
    }
    catch(std::out_of_range& e) {
        // Correct behavior
    }
    catch(...) {
        cout << "CHECK: Out-of-range at() threw some other exception\n";
    }

    a.remove(0); // Does not exist
    if(a.size() != 0) {
        cout << "CHECK: removing elements from empty array should not change anything.\n";
        return false;
    }

    return true;
}

bool test_insert(ordered_array& a) 
{
    // Initialize test data (values in range -20..+20)
    std::vector<unsigned> perm = make_random_permutation(41);
    std::vector<int> data(41);

    for(std::size_t i = 0; i < data.size(); ++i)
        data[i] = static_cast<int>(perm[i]) - 20;

    int size = a.size(); // expected size

	for(int x : data)
		cout << x << " ";
	cout << "\n";

	// cout << "Inserting: "
    for(int e : data) {
    	cout << e << " ";
        a.insert(e); size++;

        if(a.size() != size) {
            cout << "\nCHECK: After " << size << " insert()s, array size() is incorrect (= " 
                 << a.size() << ").\n";
            return false;
        }

        if(!a.exists(e)) {
            cout << "\nCHECK: After inserting " << e << " element e does not exist().\n";
            cout << "       Array contains: ";
            print_array(a);
            return false;
        }

        if(!is_sorted(a)) {
            cout << "CHECK: After inserting " << e << " array is no longer sorted.\n";
            cout << "       Array contains: ";
            print_array(a);
            return false;
        }

        if(!contains_elements(a, data, size)) {
            cout << "CHECK: After insert(), array is missing some prev. element(s).\n";
            cout << "       Array contains: ";
            print_array(a);
            return false;
        }

        if(!hides_sentinel(a)) 
            return false;
    }

    for(int i = -20; i <= 20; ++i) {
        if(!a.exists(i)) {
            cout << "\nCHECK: Value " << i << " was inserted previous but does not exist in array.\n";
            cout << "       Array contains: ";
            print_array(a);
            return false;
        }
    }

    a.insert(SENTINEL);
    if(a.exists(SENTINEL)) {
        cout << "CHECK: Inserting -2147483648 should do nothing.\n";
        cout << "       Array contains: ";
        print_array(a);
        
        return false;
    }
    
    if(a.size() != size) {
        cout << "CHECK: Inserting -2147483648 should not change size.\n";
        return false;        
    }
    
    if(!contains_elements(a, data, size)) {
        cout << "CHECK: Inserting -2147483648 should not add/remove elements.\n";
        cout << "       Array contains: ";
        print_array(a);

        return false;         
    }

    return true; // All tests successful
}

bool test_duplicates(ordered_array& a) 
{
    std::vector<int> data = {-20, 0, 0, 20, 20, 20 };
    int size = a.size(); // Starting size

    // First we insert some duplicates...
    for(int e : data) {
        a.insert(e); size++;
        if(a.size() != size) {
           cout << "CHECK: Inserting a duplicate did not increase size().\n";
           return false; 
        }
        if(!is_sorted(a)) {
            cout << "CHECK: after inserting a duplicate the array is no longer sorted.\n";
            cout << "       Array contains: ";
            print_array(a);            
            return false;
        }

        if(!hides_sentinel(a)) 
            return false;
    }

    // Count to make sure there are the right number of each
    int zeroes = 0, twenties = 0, negtwen = 0;
    for(int i = 0; i < a.size(); ++i)
        if(a.at(i) == 0)
            zeroes++;
        else if(a.at(i) == 20) 
            twenties++;
        else if(a.at(i) == -20)
            negtwen++;   

    if(zeroes != 3 || twenties != 4 || negtwen != 2) {
        cout << "CHECK: incorrect number of duplicates found.\n";
        cout << "       Array contains: ";
        print_array(a);
        
        return false;
    }
    // And then we remove them
    for(int e : data) {
        a.remove(e); size--;
        if(a.size() != size) {
            cout << "CHECK: Removing a duplicate results in incorrect size (= " 
                 << a.size() << ", expected " << size << ").\n";
            return false;

        }
        if(!is_sorted(a)) {
            cout << "CHECK: after removing a duplicate the array is no longer sorted.\n";
            cout << "       Array contains: ";
            print_array(a);
            
            return false;
        }

        if(!hides_sentinel(a)) 
            return false;
    }

    return true;
}

bool test_removal_nonexistent(ordered_array& a) 
{
    // Remove some elements that don't exist
    int size = a.size();

    // Make a copy of a.
    std::vector<int> orig(a.size());
    for(int i = 0; i < a.size(); ++i)
        orig.at(i) = a.at(i);

    for(int i = 30; i <= 40; ++i) {    
        a.remove(i);
        if(a.size() != size) {
            cout << "CHECK: Removing non-existent element changed size (";
            cout << "expected: " << size << ", actual: " << a.size() << ")\n";
            return false;
        }

        for(int j = 0; j < a.size(); ++j)
            if(a.at(j) != orig.at(j)) {
                cout << "CHECK: Removing non-existent elements changed existing elements.\n";
				cout << "       Array contains: ";
				print_array(a);                
                return false;
            }

        if(!hides_sentinel(a)) 
            return false;
    }

    a.remove(SENTINEL);
    if(a.size() != size) {
        cout << "CHECK: removing -2147483648 changed size (should do nothing).\n";
        return false;
    }
    for(int j = 0; j < a.size(); ++j)
        if(a.at(j) != orig.at(j)) {
            cout << "CHECK: Removing -2147483648 changed existing elements.\n";
            return false;
        } 
    if(!hides_sentinel(a)) 
        return false;   

    return true;
}

bool test_capacity(ordered_array& a) 
{
    int count = a.capacity() - a.size(); // How many to insert
    for(int i = 30; i < 30 + count; ++i)
        a.insert(i);

    if(a.size() != a.capacity()) {
        cout << "CHECK: array should be full, but isn't (";
        cout << "expected size: " << a.capacity() << ", actual size: " << a.size()
             << ")\n";
        return false;
    }

    // Try inserting elements and verify that size does not change, elements are
    // not added.
    int size = a.size();
    for(int i = 50; i <= 60; ++i) {
        a.insert(i); 
        if(a.size() != size) {
            cout << "CHECK: Inserting when full changed size (";
            cout << "expected size: " << size << ", actual size: " << a.size()
                 << ")\n";
            return false;
        }

        if(a.exists(i)) {
            cout << "CHECK: Inserting when full should not change array contents.\n";
            cout << "       Array contains: ";
            print_array(a);
            
            return false;
        }

        if(!hides_sentinel(a)) 
            return false;
    }

    // Remove the inserted elements
    for(int i = 30; i < 30 + count; ++i) {
        a.remove(i); 
    }

    if(a.size() != a.capacity() - count) {
        cout << "CHECK: Size is wrong after removing " << count << " elements (";
        cout << "expected size: " << a.capacity() - count << ", actual size: " 
             << a.size() << ")\n";
        return false;
    }

    return true;
}

bool test_removal(ordered_array& a) 
{
    std::vector<int> data(41);
    for(int i = -20; i <= 20; ++i)
        data[i + 20] = i;

    int size = a.size(); // Expected size
    for(int e : data) {
        a.remove(e); size--;

        if(a.size() != size) {
            cout << "CHECK: After removing existing element, size is incorrect (";
            cout << "expected size: " << size << ", actual size: " << a.size()
                 << ")\n";
            return false;
        }

        if(a.exists(e)) {
            cout << "CHECK: After removing unique element, element still exists.\n";
            cout << "       Array contains: ";
            print_array(a);
            
            return false;
        }

        if(!is_sorted(a)) {
            cout << "CHECK: After removing element, array is no longer sorted.\n";
            cout << "       Array contains: ";
            print_array(a);
            
            return false;
        }

        if(!hides_sentinel(a)) 
            return false;
    }

    return true;
}

bool test_final(ordered_array& a) 
{
    if(a.size() != 0) {
        cout << "CHECK: Size of empty array is not 0.\n";
        return false;
    }

    if(!hides_sentinel(a)) 
        return false;

    return true;
}

bool test_all() 
{
    ordered_array a(60);

    return test_initial(a,60) &&
           test_insert(a) &&
           test_duplicates(a) &&
           test_removal_nonexistent(a) &&
           test_capacity(a) &&
           test_removal(a) &&
           test_final(a);
}

int main() 
{
    cout << "---- Beginning ordered_array tests ----\n";
    if(test_all()) {
        cout << "---- All tests successful! ----\n";
        return 0;
    }
    else
        return 1;
}
