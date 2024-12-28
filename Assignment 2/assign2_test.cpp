/*
 * assign2_test.cpp
 * Test the doubly-linked list implementation
 *
 * TODO: 
 *     Tests for `at()`. 
 *     Test `empty()` for O(1)-ness
 *     Make O(1) tests more robust (less likely to report false positives)
 #     Add O(n) tests.
 */

/* ===========================================================================
   BEGIN STANDARD PREAMBLE
   =========================================================================*/
   
/* ASSIGNMENT_URL
   Update this to the URL of the assignment. This will be used to generate links to 
   the test descriptions.
*/
#define ASSIGNMENT_URL "https://fullcoll.instructure.com/courses/54267/external_tools/43298"

#include <algorithm>
#include <climits>
#include <ctime>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#define ANSI_NORMAL() "\e[0m"
#define ANSI_GREEN()  "\e[0;92m"
#define ANSI_YELLOW() "\e[0;93m"
#define ANSI_RED()    "\e[0;91m"
#define ANSI_UL()     "\e[4m"

bool grading_extra_credit = false;
int extra_credit_score = 0; // 0/1

void init_extra_credit()
{
    const char* ec_value = getenv("CS133_EXTRA_CREDIT");
    if(ec_value != nullptr) {
        grading_extra_credit = true;
        std::cout << "(Also checking extra credit...)" << std::endl;        
    }
}

#if defined(__WIN64) || defined(__WIN32)

#include <windows.h>
#include <ctime>

void return_extra_credit()
{
    // On windows, modifications to environment variables are scoped to the
    // application, so this mechanism cannot be used to return the extra
    // credit score.
}

void init_tests()
{
    // Enable UTF-8 in the terminal.
    // This could fail but I don't think it's important enough to worry about.
    SetConsoleOutputCP(CP_UTF8);

    // Enable ANSI in the terminal
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    // This might happen if stdout is redirected.
    if(console_handle == INVALID_HANDLE_VALUE)
        return;

    // This might also fail but I don't think we need to care about that.
    DWORD console_mode = 0;
    GetConsoleMode(console_handle, &console_mode);

    SetConsoleMode(console_handle, 
        console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    );

    init_extra_credit();
}

long long get_current_process_time()
{
    // On windows, we hope that std::clock() is precise enough to use for
    // benchmarking.
    return std::clock();
}

#else // *nix

#include <time.h>

void return_extra_credit()
{
    if(not grading_extra_credit)
        return;

    char ec_str[] = "CS133_EXTRA_CREDIT=0";
    const int ec_len = sizeof(ec_str) / sizeof(char);

    if(extra_credit_score == 1)
        ec_str[ec_len-2] = '1';

    putenv(ec_str);
}

void init_tests()
{
    init_extra_credit();
}

long get_current_process_time()
{
    // On *nix, we use clock_gettime with CLOCK_PROCESS_CPUTIME_ID to get 
    // high-resolution, current-process-only timing data.
    struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return long(ts.tv_sec) * 1000000000L + ts.tv_nsec;
}

#endif

#define SECTION(msg) do {} while(false);

/* CHECK(cond,msg)
   Check if cond == true; if so, print

       FAILED TEST #n: msg
       (See https://url for details)

   and then return false.   
*/
#ifndef LIST_ALL_CHECKS

#define CHECK(cond,msg)                                                      \
    do {                                                                     \
        if(cond) {                                                           \
            const int n = __COUNTER__;                                       \
            std::cout << "\n\u001b[31;1mTEST #" << n << " "                  \
                         "(" << __FILE__ << ":" << __LINE__ << ")" <<        \
                         " FAILED:" << ANSI_NORMAL() << " " << msg << '\n';  \
            std::cout << "(See \u001b[32;1m" << ASSIGNMENT_URL <<            \
                         "#test" << n << ANSI_NORMAL() << " for info.)\n";   \
            return false;                                                    \
        }                                                                    \
    } while(false); 

#define CHECK_WARN(cond,msg)                                                 \
    do {                                                                     \
        if(cond) {                                                           \
            const int n = __COUNTER__;                                       \
            std::cout << "\n" << ANSI_YELLOW() << "WARNING, TEST #" << n << " " \
                         "(" << __FILE__ << ":" << __LINE__ << ")" <<        \
                         " FAILED:" << ANSI_NORMAL() << " " << msg << '\n';  \
            std::cout << "(See \u001b[32;1m" << ASSIGNMENT_URL <<            \
                         "#test" << n << ANSI_NORMAL() << " for info.)\n";   \
            if(warnings_are_failure) return false;                           \
        }                                                                    \
    } while(false); 

#else
// This part is used to list the checks. This will probably only work on GCC/Clang, 
// but students should never need to use this.
#define DO_PRAGMA(x)       _Pragma( #x )
#define PRINT(m)           DO_PRAGMA(GCC warning #m)
#define CHECK(cond, msg)                                                     \
	do {                                                                     \
		PRINT(CHECK msg)                                         \
	} while(false);

#define CHECK_WARN(cond, msg)                                                \
    do {                                                                     \
        PRINT(CHECK msg)                                         \
    } while(false);    

// #define SECTION(msg) do { PRINT(CHECK SECTION msg) } while(false);

#endif

// If this is set to true, then CHECK_WARN will count a warning as a failure.
bool warnings_are_failure = false;

void OK() { std::cout << "\u001b[32m" << "OK" << ANSI_NORMAL() << "\n"; } 

/* make_random_vector(len)
   Returns a vector<int> of random values, where each entry is between 0 and
   INT_MAX. The optional second parameter lets you specify the seed to be used 
   for the RNG.
*/
std::vector<int> make_random_vector(
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
std::vector<unsigned> make_random_permutation(
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

// Timing functions

struct timing_data
{
    // Measured average, stddev for a fixed size
    double average;
    double stddev;

    int tests_performed;

    // Number of results > 2*stddev, out of tests_performed
    int outlier_count;

    // Average of all outliers. Note that this is only valid if 
    // outlier_count > 0.
    double outlier_average;

    operator bool ()
    {
        return 1.0 * outlier_count < 0.95 * tests_performed;
    }
};

/* check_o1<T>(fp)
   Attempts to check if the operation `fp()` runs in O(1) time. `fp` must 
   accept an argument of type T& and must 1) perform the operation to be tested
   and 2) increase the size of its argument by a fixed amount. 
*/
template<typename T, typename FPFunc>
timing_data check_o1(
    FPFunc&& fp, 
    int timing_test_count = 1000,
    int subsample_test_count = 100
)
{
    // 1. Get average runtime, stddev
    std::vector<double> samples;
    timing_data td;

    for(int i = 0; i < timing_test_count; ++i) {
        T test_object;

        auto start = get_current_process_time();
        for(int j = 0; j < subsample_test_count; ++j) {
            fp(test_object);
        }
        auto end = get_current_process_time();

        samples.push_back(end - start);
    }

    // Compute average
    double avg = 0.0;
    for(double d : samples) avg += d;
    avg /= samples.size();
    td.average = avg;

    double stddev = 0.0;
    for(double d : samples)
        stddev += (avg - d) * (avg - d);
    stddev /= samples.size();
    stddev = std::sqrt(stddev);
    td.stddev = stddev;

    //std::cout << "AVG = " << avg << ", STDDEV = " << stddev << std::endl;

    // 2. Measure, count number of outliers
    int outlier_count = 0;
    T test_object;
    const int tests = timing_test_count;
    td.tests_performed = tests;
    double outlier_average = 0.0;
    for(int i = 0; i < timing_test_count; ++i) {

        auto start = get_current_process_time();
        for(int j = 0; j < subsample_test_count; ++j) {
            fp(test_object);
        }
        auto end = get_current_process_time();

        double duration = end - start;
            
        if(duration > (avg + 2*stddev)) {
            outlier_count++;
            outlier_average += duration;

            //std::cout << "OUTLIER = " << duration << std::endl;
        }
    }

    td.outlier_count = outlier_count;
    td.outlier_average = outlier_average / outlier_count;

    return td;    
}

/* check_o1<T>(f, p)
   Attempts to determine if function f() runs in O(1) time. `f` must accept
   an argument of type T&, and *must not* change the size of its argument. 
   `p` must accept an argument of type T& and must increase the size of its
   argument by a fixed amount (typically by 1).

   T must be default-constructible.
*/
template<typename T, typename TimingFunc, typename PushFunc>
bool check_o1(
    TimingFunc&& f, 
    PushFunc&& p, 
    int timing_test_count = 1000,
    int subsample_test_count = 100
)
{
    // This works by first running f 100 times, in a loop, to compute the 
    // average runtime; at the beginning of each run of 100, we reset the test 
    // object to empty. This sets our benchmark. Then, we run f;p in a similar
    // loop, *without* resetting the test object (so that it grows in size) and 
    // check to see if the timing that results stays within two 
    // standard deviations of the above benchmark.

    // Function that runs f then p
    auto fp_func = [&f,&p](T& x)
    {
        f(x);
        p(x);
    };

    return check_o1<T>(
        fp_func,
        timing_test_count,
        subsample_test_count
    );
}

// Testing O(n): This is similar, except that we divide the timings by the size,
// to normalize them. We also allow the user to pass in the "initial" object,
// so that it can be larger than empty.

/* ===========================================================================
   END STANDARD PREAMBLE
   =========================================================================*/

#include "dlist.hpp"

// We include dlist twice, to check for functions in header files (which are not
// allowed), and to check for an include guard (which is required).
#include "dlist.hpp"

/* std_list
   This is left-over from an attempt to use a templated dlist to allow us to
   plug in other node types (so we can count allocations and such). We might do
   that again in the future, but it requires the implementation to be in the 
   .hpp file.
*/
using std_dlist = dlist;

// Prints the contents of the list in the forward direction; You can use limit to only print the
// first n elements.
char print_fwd (std_dlist& l, int limit = INT_MAX)
{
	using std::cout; 
	
	if(l.head() == nullptr) 
		cout << "<empty list>";
	else {
		// If the list does not end with nullptr, this will fail.
		for(std_dlist::node* n = l.head(); 
		    n != nullptr and limit > 0; 
		    n = n->next, --limit) 
		{
			if(n != l.head())
				cout << " -> ";

			cout.width(3);
			cout << n->value;
		}
	}

	return '\0';
}

char print_rev(std_dlist& l, int limit = INT_MAX)
{
	using std::cout;

	if(l.head() == nullptr) 
		cout << "<empty list>";
	else {
		for(std_dlist::node* n = l.tail(); 
		    n != nullptr and limit > 0; 
		    n = n->prev, --limit) 
		{
			if(n != l.tail())
				cout << " <- ";

			cout.width(3);
			cout << n->value;
		}
	}

	return '\0';
}

// Returns true if the list a is acyclic, in both the forward and reverse directions
bool acyclic(std_dlist& l)
{
	std::vector<std_dlist::node*> nodes;

	for(std_dlist::node* n = l.head(); n != nullptr; n = n->next) {

		// Check to see if n is already in the vector
		for(std_dlist::node* m : nodes)
			if(m == n)
				return false; // Same node seen twice

		// Otherwise, add to the collection of "seen" nodes and continue
		nodes.push_back(n);
	}

	nodes.clear();

	for(std_dlist::node* n = l.tail(); n != nullptr; n = n->prev) {

		for(std_dlist::node* m : nodes)
			if(m == n)
				return false;

		nodes.push_back(n);
	}

	return true;
}

/* ---------------------------------------------------------------------------
   Begin tests
   -------------------------------------------------------------------------- */

/* verify(l)
   Check the structure of the list.
*/
bool verify(std_dlist& l) 
{
    SECTION(verify);

	// Empty list
	if(l.head() == nullptr and l.tail() == nullptr) {
		CHECK(l.size() != 0,
			"Empty list (head == tail == nullptr) but list.size() = " << l.size());

		// I can't think of any other way the empty list could go wrong.
		return true;
	}

	CHECK(l.head() == nullptr and l.tail() != nullptr, 
		"head is nullptr but tail is not (if one is nullptr, both should be)");

	CHECK(l.head() != nullptr and l.tail() == nullptr,
		"tail is nullptr but head is not (if one is nullptr, both should be)");

	// These check for head != tail (which should indicate a list with > 1 elements)
	// but head->next or tail->prev == nullptr.
	CHECK(l.head()->next == nullptr and l.head() != l.tail(),
		"head->next == nullptr, but head != tail.");

	CHECK(l.tail()->prev == nullptr and l.head() != l.tail(),
		"tail->prev == nullptr, but head != tail.");

	// Single-element list, because head == tail.
    if(l.head() == l.tail()) {

		CHECK(l.size() != 1,
			"head == tail (!= nullptr), but size() != 1.\n" <<
			"List: " << print_fwd(l));

		// We only need to check this for head, because head == tail.
		CHECK(l.head()->next != nullptr,
			"head == tail (!= nullptr), but node->next != nullptr");

		CHECK(l.head()->prev != nullptr,
			"head == tail (!= nullptr), but node->prev != nullptr");

		// Again, don't need to check for tail, because head == tail.
	
        return true; 
    }
    else { // head != tail, should be a list of size > 1

		CHECK(l.size() <= 1,
			"head != tail, but list.size() = " << l.size());

        // Check outward-pointing pointers for nullness
        CHECK(l.head()->prev != nullptr, "head->prev != nullptr");
        CHECK(l.tail()->next != nullptr, "tail->next != nullptr");

        // In a multi-element list, head->next should be non-null,
        // as should tail->prev.

        CHECK(l.head()->next == nullptr,
        	"head != tail, but head->next == nullptr");

        CHECK(l.tail()->prev == nullptr,
        	"head != tail, but tail->prev == nullptr");

        // Check all internal pointers to make sure they line up.

        std_dlist::node*c = l.head();

        // Because head != tail, we know this will be true at least once. 
        while(c != l.tail()) {
			CHECK(c == nullptr, "Found a non-tail node with n->next == nullptr");

			// Only if we have moved beyond the head do we check these, because they refer to c->prev
			if(c != l.head()) {
				CHECK(c->prev == nullptr, "Found a non-head node with n->prev == nullptr");

				// If we did this at the head, we'd get a nullptr dereference
				CHECK(c->prev->next != c,
					"n->prev->next does not point back to n.");
			}

			CHECK(c->next->prev != c,
				"n->next->prev does not point back to n.");        

			CHECK(c->next == c->prev,
				"Found a node " << c->value << " where next == prev");

			CHECK(c->prev == c,
				"Found a node " << c->value << " where n->prev == n");

			CHECK(c->next == c,
				"Found a node " << c->value << " where n->next == n");

            c = c->next;
        }

        // Make sure we can traverse the list from both directions
        for(std_dlist::node* n = l.head(); n != l.tail(); n = n->next) {
			CHECK(n == nullptr,
				"Could not traverse the list from head to tail");
        }
        
        for(std_dlist::node* n = l.tail(); n != l.head(); n = n->prev) {
        	CHECK(n == nullptr,
        		"Could not traverse the list from tail to head");
        } 

		CHECK(not acyclic(l),
			"List has a cycle");

        return true; // Everything OK!
    }
}

// Returns true if l contains the same elements as v.
// This assumes that the list is traversable in the forward direction, ending with
// a nullptr.
bool contains_elements(std_dlist& l, std::vector<int> v)
{
	size_t i = 0;
	std_dlist::node* n = l.head();

	while(i != v.size() and n != nullptr) {

		if(v.at(i) != n->value)
			return false;

		++i;
		n = n->next;
	}

	// If they both don't end at the same point, then that's a fail.
	if((i == v.size()) != (n == nullptr))
		return false; 

	return true;
}

bool equal_lists(std_dlist& l1, std_dlist& l2)
{
    std_dlist::node* a = l1.head();
    std_dlist::node* b = l2.head();

    while(a != nullptr and b != nullptr) {
        if(a->value != b->value)
            return false;

        a = a->next;
        b = b->next;
    }

    return a == nullptr and b == nullptr;
}

/* separate_list_nodes(a,b)
   Returns true if none of the node pointers in a occur in b, and vice versa.
*/
bool separate_list_nodes(std_dlist& l1, std_dlist& l2)
{
    std::vector<std_dlist::node*> l1_nodes, l2_nodes;

    std_dlist::node* n = l1.head();
    while(n != nullptr) {
        l1_nodes.push_back(n);
        n = n->next;
    }

    n = l2.head();
    while(n != nullptr) {
        l2_nodes.push_back(n);
        n = n->next;
    }

    for(std_dlist::node* a : l1_nodes)
        for(std_dlist::node* b : l2_nodes)
            if(a == b)
                return false;

    return true;
}

// If the previous function returned false, this can be used to print the
// pair of elements where the vector and the list differ.
void show_mismatched_elements(std_dlist& l, std::vector<int> v)
{
	using std::cout; 
	size_t i = 0;
	std_dlist::node* n = l.head();

	while(i != v.size() and n != nullptr) {

		if(v.at(i) != n->value)
			cout << "list.at(" << i << ") = " << n->value << ", should be " << v.at(i); 

		++i;
		n = n->next;
	}

	// If they both don't end at the same point, then that's a fail.
	if(i == v.size() and n != nullptr) {
		cout << "list.at(" << i << ") = " << n->value << ", should be the end of the list";
	}
	else if(i < v.size() and n == nullptr) {
		cout << "end-of-list, should be " << v.at(i);
	}	
}

std::ostream& operator<< (std::ostream& out, std::vector<int> v)
{
	for(std::size_t i = 0; i < v.size(); ++i) {
		out.width(3);
		out << v.at(i) << (i < v.size() - 1 ? ",   " : ""); // Align with list printing
	}

	return out;
}

/*****************************************************************************
 * Test functions
 *****************************************************************************/

bool test_empty() 
{
    SECTION(empty);

    std::cout << "Checking empty list... ";
    std::cout.flush();
    std_dlist e;
    
    if(!verify(e))
        return false;

	CHECK(not e.empty(),
		"empty list is not .empty()");

	OK();

    return true;
}

bool test_insert() 
{
    SECTION(insert);

    using std::cout;

	// What should the list contain?
	std::vector<int> contents;

    cout << "Checking .insert() into empty list... ";
    std::cout.flush();
    std_dlist e;
    e.insert(e.head(), 100);
    contents.push_back(100);
    
    if(!verify(e))
        return false;

    CHECK(e.size() != 1,
    	"size() != 1 after inserting one element into an empty list");

    CHECK(e.head() == nullptr,
        "After inserting one element into an empty list, head() is nullptr");

    CHECK(e.tail() == nullptr,
        "After inserting one element into an empty list, tail() is nullptr");

    CHECK(e.head() != e.at(0),
        "After inserting one element into an empty list, head() != at(0)");

    CHECK(e.tail() != e.at(0),
        "After inserting one element into an empty list, tail() != at(0)");

    CHECK(e.at(0) == nullptr, 
        "After inserting one element into an empty list, .at(0) is nullptr");

	CHECK(e.at(0)->value != 100,
		"After inserting 100 into an empty list, l.at(0) = " << e.at(0)->value);
		
    OK();

    cout << "Checking .insert(nullptr) by inserting 0, 1, ..., 10; ";
    std::cout.flush();
    std_dlist l0;
    contents.clear();
    for(int i = 0; i < 10; ++i) {
        l0.insert(nullptr, i);
        contents.insert(contents.begin(), i);

        if(!verify(l0))
            return false;

        CHECK(l0.size() != i + 1,
            "After inserting " << i + 1 << " elements, list.size() = " << l0.size());

        CHECK(not contains_elements(l0, contents),
            "After inserting " << i << ", the contents of the list are incorrect\n" <<
            "List:           " << print_fwd(l0) << "\n" <<
            "Should contain: " << contents);
    }

    OK();

    cout << "Checking .insert() method after head() by inserting 0, 1, ..., 10; ";
    std::cout.flush();
    std_dlist l1;
    contents.clear();
    for(int i = 0; i < 10; ++i) {

        l1.insert(l1.head(), i);

        // vector::insert inserts before, while our insert inserts after, which 
        // requires a bit of an adjustment. 
        if(i == 0)
        	contents.push_back(i);
        else
        	contents.insert(contents.begin() + 1, i);
        
        if(!verify(l1))
            return false;

		CHECK(l1.size() != i + 1,
			"After inserting " << i + 1 << " elements, list.size() = " << l1.size());

		CHECK(not contains_elements(l1, contents),
			"After inserting " << i << ", the contents of the list are incorrect\n" <<
			"List:           " << print_fwd(l1) << "\n" 
			"Should contain: " << contents);       
    }
    
    OK();

    std::cout << "Checking .insert() at tail() by inserting 0, 1, ..., 9, 10; "; 
    std::cout.flush();   
    std_dlist l2;
    contents.clear();
    for(int i = 0; i < 10; ++i) {

        l2.insert(l2.tail(), i);
        contents.push_back(i);
        
        if(!verify(l2))
            return false;

		CHECK(l2.size() != i + 1,
			"After inserting " << i + 1 << " elements, list.size() = " << l2.size());

		CHECK(not contains_elements(l2, contents),
			"After inserting " << i << ", the contents of the list are incorrect\n" <<
			"List: " << print_fwd(l2));
			
    }
    
    OK();

    std::cout << "Checking .insert() in between 5 and 6 of 0, 1, ..., 5, 6, ..., 10; ";
    std::cout.flush();
    std_dlist l3;
    contents.clear();
    std_dlist::node* m = nullptr;
    for(int i = 0; i < 11; ++i) {
		// We know this is safe, because we've already tested insert at the tail.
        l3.insert(l3.tail(), i); 
        contents.push_back(i);
        // We used to save a pointer to the tail after inserting 5, but some
        // people implement insert(tail,x) by inserting a new node *before*
        // the tail as a copy of the tail, and then modifying the tail node,
        // so that once created, the tail never changes. That obviously breaks
        // this code, so below we access the 5th node directly.
        /*
        if(i == 5) 
            m = l3.tail(); // Save pointer to node 5, we'll insert after this later.      
        */
    }    

    m = l3.head()->next->next->next->next->next;

    // Actually insert some values in the middle
    for(int i = 100; i < 120; ++i) {

        l3.insert(m, i);

        // Again, we have to adjust for the fact that list inserts after
        contents.insert(contents.begin() + (5+1), i);
        
        if(!verify(l3))
            return false;

        CHECK(not contains_elements(l3, contents),
        	"After inserting " << i << ", the contents of the list are incorrect: \n" <<
        	"List:           " << print_fwd(l3) << "\n" <<
        	"Should contain: " << contents);
    }

    CHECK(l3.size() != 11 + 20,
    	"After inserting 31 elements, list.size() = " << l3.size());
    	
    OK();

    return true;
}

bool test_erase_node() 
{
    SECTION(erase_node);

    using std::cout;
    using std::endl;

    std_dlist::node* n = nullptr;
    std_dlist l;
    std::vector<int> contents;

    cout << "Checking erase(node*) from empty list... ";
    cout.flush();
    std_dlist e;
    e.erase(nullptr); // This will most likely crash if it fails at all
    if(!verify(e))
        return false;
        
    OK();

    // We know that insert works at this point, so we use it to construct
    // the list, and we don't bother to verify.
    for(int i = 0; i < 30; ++i) {
        l.insert(l.tail(), i);
        contents.push_back(i);
        if(i == 15)
            n = l.tail(); // Save pointer
    }

    cout << "Checking erase(node*) 10 elems. from the head of 0, 1, 2, ..., 29; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {

		int value = l.head()->value;
		(void)value;
     
        l.erase(l.head());
        contents.erase(contents.begin());
        
        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
        	"After erasing " << value << " at head, list contents are incorrect:\n" <<
        	"List:           " << print_fwd(l) << "\n" <<
        	"Should contain: " << contents);
    }
    
    OK();

    cout << "Checking erase(node*) 10 elems. from the tail of 10, 11, ..., 28, 29; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {

		int value = l.tail()->value; 
		(void)value;
        
        l.erase(l.tail());
        contents.pop_back();
        
        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
        	"After erasing " << value << " at tail, list contents are incorrect:\n" <<
        	"List:           " << print_fwd(l) << "\n" <<
        	"Should contain: " << contents);     
    }
    
    OK();

    cout << "Checking erase(node*) from the middle by removing 15... ";
    int value = n->value;
    (void)value;
    l.erase(n);
    contents.erase(contents.begin() + 5);
    
    if(!verify(l))
        return false;

    CHECK(not contains_elements(l, contents),
      	"After erasing " << value << " in the middle, list contents are incorrect:\n" <<
      	"List:           " << print_fwd(l) << "\n" <<
      	"Should contain: " << contents); 	

	OK();

	cout << "Erase(node*)-ing remaining elements... ";
	for(int i = 0; i < 9; ++i)
		l.erase(l.head());

	if(!verify(l))
		return false;

	CHECK(not l.empty(),
		"After erasing all elements, list is not empty()\n" <<
		"List: " << print_fwd(l) << "\n");
        
    OK();
    return true;
}

bool test_erase_index()
{
    SECTION(erase_index);

    using std::cout;
    using std::endl;

    // This is basically the same code as for test_erase_node(), just with the
    // pointers replaced by the equivalent indexes.
    std_dlist l;
    std::vector<int> contents;

    cout << "Checking erase(index) from empty list... ";
    cout.flush();
    std_dlist e;
    e.erase(0); // This will most likely crash if it fails at all
    if(!verify(e))
        return false;
        
    OK();

    // We know that insert() works at this point, so we use it to construct
    // the list, and we don't bother to verify.
    for(int i = 0; i < 30; ++i) {
        l.insert(l.tail(), i);
        contents.push_back(i);
    }

    cout << "Checking erase(index) 10 elems. from the head of 0, 1, 2, ..., 29; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {

        int value = l.head()->value;
        (void)value;
     
        l.erase(0);
        contents.erase(contents.begin());
        
        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After erasing " << value << " at head, list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);
    }
    
    OK();

    cout << "Checking erase(index) 10 elems. from the tail of 10, 11, ..., 28, 29; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {

        int value = l.tail()->value; 
        (void)value;
        
        // We want to avoid using l.size(); contents and l should have the same 
        // size, so we can use that as a proxy.
        l.erase(contents.size() - 1);
        contents.pop_back();
        
        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After erasing " << value << " at tail, list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);     
    }
    
    OK();

    cout << "Checking erase(index) from the middle by removing 15... ";
    l.erase(5);
    contents.erase(contents.begin() + 5);
    
    if(!verify(l))
        return false;

    CHECK(not contains_elements(l, contents),
        "After erasing " << 15 << " in the middle, list contents are incorrect:\n" <<
        "List:           " << print_fwd(l) << "\n" <<
        "Should contain: " << contents);    

    OK();

    cout << "Erase(index)-ing remaining elements... ";
    for(int i = 0; i < 9; ++i)
        l.erase(0);

    if(!verify(l))
        return false;

    CHECK(not l.empty(),
        "After erasing all elements, list is not empty()\n" <<
        "List: " << print_fwd(l) << "\n");
        
    OK();

    // ---- New tests for erase(index) ---
    // erase(i) has the additional requirements that if i < 0, we erase
    // the head, and if i >= size, we erase the tail.
    contents.clear();

    // Rebuild the list containing 0, 1, 2, ... 30
    for(int i = 0; i < 30; ++i) {
        l.insert(l.tail(), i);
        contents.push_back(i);
    }

    cout << "Testing erase(index) with negative i (should erase head)...";
    cout.flush();
    for(int i = 0; i < 10; ++i) {
        l.erase( -(i + 1)*7 );
        contents.erase(contents.begin());

        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After erasing at index " << (-(i + 1)*7) << ", list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);  
    }

    OK();

    cout << "Testing erase(index) with i >= size (should erase tail)...";
    cout.flush();

    for(int i = 0; i < 10; ++i) {
        l.erase( (i + 1)*7 + 1000 );
        contents.pop_back();

        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After erasing at index " << ((i + 1)*7 + 1000) << ", list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);  
    }

    OK();    

    return true;    
}

bool test_remove()
{
    SECTION(remove);

    // This code is based on test_erase_node, but using the values to be
    // erased instead of pointers.

    using std::cout;
    using std::endl;

    std_dlist::node* n = nullptr;
    std_dlist l;
    std::vector<int> contents;

    cout << "Checking remove(x) from empty list... ";
    cout.flush();
    std_dlist e;
    e.remove(12); // This will most likely crash if it fails at all
    if(!verify(e))
        return false;
        
    OK();

    // We know that insert works at this point, so we use it to construct
    // the list, and we don't bother to verify.
    for(int i = 0; i < 30; ++i) {
        l.insert(l.tail(), i);
        contents.push_back(i);
        if(i == 15)
            n = l.tail(); // Save pointer
    }

    cout << "Checking remove(x) 10 elems. from the head of 0, 1, 2, ..., 29; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {

        int value = l.head()->value;  
        (void)(value);      
     
        l.remove(contents.front());
        contents.erase(contents.begin());
        
        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After removing " << value << " at head, list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);
    }
    
    OK();

    cout << "Checking remove(x) 10 elems. from the tail of 10, 11, ..., 28, 29; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {

        int value = l.tail()->value; 
        (void)value;
        
        l.remove(contents.back());
        contents.pop_back();
        
        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After removing " << value << " at tail, list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);     
    }
    
    OK();

    cout << "Checking remove(15) from the middle ... ";
    int value = n->value;
    (void)value;
    l.remove(15);
    contents.erase(contents.begin() + 5);
    
    if(!verify(l))
        return false;

    CHECK(not contains_elements(l, contents),
        "After removing " << value << " in the middle, list contents are incorrect:\n" <<
        "List:           " << print_fwd(l) << "\n" <<
        "Should contain: " << contents);    

    OK();

    cout << "Removing(x)-ing remaining elements... ";
    for(int i = 0; i < 9; ++i)
        l.remove(l.tail()->value);

    if(!verify(l))
        return false;

    CHECK(not l.empty(),
        "After removing all elements, list is not empty()\n" <<
        "List: " << print_fwd(l) << "\n");
        
    OK();

    // ---- New tests for remove() ----
    contents.clear();
    for(int i = 0; i < 30; ++i) {
        l.insert(l.tail(), i);
        contents.push_back(i);
    }

    cout << "Checking remove(x) where x does not exist...";
    for(int i = -1; i > -10; --i) {
        // There are no negative numbers in l, so this shouldn't do anything.
        l.remove(i);

        if(!verify(l))
            return false;

        CHECK(not contains_elements(l, contents),
            "After removing " << i << " (which does not exist), list contents are incorrect:\n" <<
            "List:           " << print_fwd(l) << "\n" <<
            "Should contain: " << contents);  
    }

    OK();

    cout << "Checking remove(x) where duplicates of x exist...";
    dlist l2;
    l2.insert(l2.tail(), 1);
    l2.insert(l2.tail(), 2);
    l2.insert(l2.tail(), 3);
    l2.insert(l2.tail(), 2);
    l2.insert(l2.tail(), 4);

    l2.remove(2); 

    if(!verify(l2))
        return false;

    contents = { 1, 3, 2, 4 };

    CHECK(not contains_elements(l2, contents),
        "After removing " << 2 << ", list contents are incorrect:\n" <<
        "List:           " << print_fwd(l2) << "\n" <<
        "Should contain: " << contents);     
 
    OK();

    return true;
}

bool test_push_back() 
{
    SECTION(push_back);

    using std::cout;
    using std::endl;

    std_dlist e;

    cout << "Checking push_back with 0, 1, ..., 19; ";
    cout.flush();
    for(int i = 0; i < 20; ++i) {

        e.push_back(i);
        if(!verify(e))
            return false;

		CHECK(e.tail()->value != i,
			"Last push_back'd value = " << i << ", but tail = " << e.tail()->value);

		CHECK(e.size() != i + 1,
			"After push_back'ing " << i + 1 << " values, size is " << e.size());
    }

    CHECK(not contains_elements(e, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19}),
    	"After push_back'ing 0, 1, 2, ..., 18, 19, list does not contain correct elements:\n"
    	"List: " << print_fwd(e));

    OK();
    return true;
}

bool test_push_front() 
{
    SECTION(push_front);

    using std::cout;
    using std::endl;

    std_dlist e;

    cout << "Checking push_front with 0, 1, 2, ..., 18, 19; ";
    cout.flush();
    for(int i = 0; i < 20; ++i) {
        e.push_front(i);
        
        if(!verify(e))
            return false;

		CHECK(e.head()->value != i,
			"After push_front'ing " << i << ", head->value = " << e.head()->value);

		CHECK(e.size() != i+1,
			"After push_front'ing " << i+1 << " values, size() = " << e.size());
    }

	CHECK(not contains_elements(e, {19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0}),
		"After push_front'ing 0, 1, 2, ..., 18, 19, list does not contain correct elements: \n"
		"List: " << print_fwd(e));
    
    OK();
    return true;
}

bool test_pop_front() 
{
    SECTION(pop_front);

    using std::cout;
    using std::endl;

    cout << "Checking pop_front from empty list... ";
    cout.flush();
    std_dlist l;
    l.pop_front();
    if(!verify(l))
        return false;
        
    OK();

    std_dlist e;

    // Construct list using insert. 0-9
    for(int i = 0; i < 10; ++i)
        e.insert(e.tail(), i);

    cout << "Checking pop_front from 0, 1, ..., 8, 9; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {
        e.pop_front();
        if(!verify(e))
            return false;

        CHECK(e.size() != 9 - i,
        	"List should have " << 9-i << " elements, but size() = " << e.size());        	
    }

	CHECK(not e.empty(),
		"After pop_front'ing all elements, the list is not empty().\n" <<
		"List: " << print_fwd(e));
    
    OK();

    return true;
}

bool test_pop_back() 
{
    SECTION(pop_back);

    using std::cout;
    using std::endl;

    cout << "Checking pop_back from empty list... ";
    cout.flush();
    std_dlist l;
    l.pop_back();
    if(!verify(l))
        return false;
        
    OK();

    std_dlist e;

    // Construct list using insert. 0-9
    for(int i = 0; i < 10; ++i)
        e.insert(e.tail(), i);

    cout << "Checking pop_back from 0, 1, ..., 8, 9; ";
    cout.flush();
    for(int i = 0; i < 10; ++i) {
        e.pop_back();
        
        if(!verify(e))
            return false;

        CHECK(e.size() != 9-i,
        	"List should have " << 9 - i << " elements, but size() = " << e.size());    
    }

    CHECK(not e.empty(),
        "After pop_back'ing all elements, the list is not empty().\n" <<
        "List: " << print_fwd(e));
    
    OK();
    return true;
}

bool test_to_vector()
{
    SECTION(to_vector);

    using std::cout;
    using std::endl;

    cout << "Checking to_vector() from empty list... ";
    cout.flush();   

    std_dlist e;
    std::vector<int> empty = e.to_vector();

    CHECK(e.empty() == false,
        "After running to_vector() on the empty list, the resulting vector is not empty: " <<
        "vector contains " << empty);

    OK();

    cout << "Checking to_vector() on 0, 1, 2, ... 10... (list constructed with push_back) ";
    std_dlist l1;
    for(int i = 0; i < 10; ++i) {
        l1.push_back(i);

        if(not verify(l1))
            return false;
    } 

    std::vector<int> result = l1.to_vector();
    CHECK((result != std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}),
        "to_vector() did not return 0, 1, 2, ... 9\n" <<
        "result contains " << result
    );

    OK();

    return true;
}

bool test_copy()
{
    SECTION(copy);

    // TODO: Sometimes students forget to initialized the ->prev of the 
    // copied nodes (if they implement copy manually, rather than using the
    // other functions they have). We need to check that.

    using std::cout;
    using std::endl;

    cout << "Checking copy constructor on 0, 1, 2, ..., 9... ";
    cout.flush();
    std_dlist l1;
    for(int i = 0; i < 10; ++i)
        l1.push_back(i);

    std_dlist l1_copy = l1;

    if(not verify(l1_copy))
        return false;

    CHECK(not contains_elements(l1, {0,1,2,3,4,5,6,7,8,9}),
        "Original list was modified as a result of copying!"
    );

    CHECK(not equal_lists(l1, l1_copy),
        "After copy-constructing a copy of 0, 1, 2, ..., 9, copy has the wrong values:\n" <<
        "Should contain: " << print_fwd(l1) << "\n" <<
        "Copy contains:  " << print_fwd(l1_copy));

    CHECK((not separate_list_nodes(l1, l1_copy)),
        "After copy-constructing a copy of 0, 1, 2, ..., 9, copy shares node* with the original");

    CHECK(l1.size() != l1_copy.size(),
        "After copy-construction, size of copy is " << l1_copy.size() <<
        " but size of the original is " << l1.size() << " (these should be the same)");    

    OK();

    cout << "Checking copy assignment (a = b, same size)... ";
    cout.flush();

    std_dlist a,b;
    for(int i = 0; i < 10; ++i) {
        a.push_back(i);      // 0, 1, 2, ..., 9
        b.push_back(i + 10); // 10, 11, ..., 19
    }

    a = b;

    CHECK((not equal_lists(a, b)),
        "After a = b, a is different from b\n" <<
        "a = " << print_fwd(a) << "\n" <<
        "b = " << print_fwd(b)
    );

    CHECK(not separate_list_nodes(a,b),
        "After a = b, a and b share some node pointers."
    );

    CHECK(a.size() != b.size(),
        "After a = b, a and b have different sizes (" <<
        "a.size() = " << a.size() << ", b.size() = " << b.size() <<
        "; both should be 10)"
    );

    CHECK(not contains_elements(a, std::vector<int>{10,11,12,13,14,15,16,17,18,19}),
        "After a = b, a should contain 10, 11, 12, ..., 19 but does not\n" <<
        "a = " << print_fwd(a)
    );

    OK();

    cout << "Checking copy-assignment (a = b, different sizes)... ";
    cout.flush();

    std_dlist c,d;
    for(int i = 0; i < 10; ++i) {
        c.push_back(i);      // 0, 1, ..., 9 
        d.push_back(2*i);    // 0, 1, 2, ..., 17, 18, 19
        d.push_back(2*i+1);
    }

    c = d;

    CHECK((not equal_lists(c, d)),
        "After a = b, a is different from b\n" <<
        "a = " << print_fwd(c) << "\n" <<
        "b = " << print_fwd(d)
    );

    CHECK(not separate_list_nodes(c,d),
        "After a = b, a and b share some node pointers."
    );

    CHECK(c.size() != d.size(),
        "After a = b, a and b have different sizes (" <<
        "a.size() = " << c.size() << ", b.size() = " << d.size() <<
        "; both should be 20)"
    );

    CHECK(not contains_elements(c, 
        std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19}),
        "After a = b, a should contain 0, 1, 2, ..., 19 but does not\n" <<
        "a = " << print_fwd(c)
    );

    OK();    

    cout << "Checking self-assignment (a = a); this might crash on failure... ";

    std_dlist e;
    for(int i = 0; i < 10; ++i) e.push_back(i);

    e = e;

    CHECK(e.size() != 10,
        "After self-assignment (a = a), size is incorrect; expected size 10, but " <<
        "a.size() = " << e.size()
    );

    CHECK(e.empty(),
        "After self-assignment (a = a), a is empty."
    );

    OK();

    return true;
}

// Did we have a big-O related test failure?
bool had_timing_failure = false;

bool test_o1_operations()
{
    SECTION(big_o1);

    // The O(1) operations are insert(), erase(), push/pop_*, and empty()
    // We don't test the erase/pop functions because they reduce the size.

    using std::cout;
    using std::endl;

    cout << "Checking O(1) operations (this may take some time)..." << endl;

    auto do_insert = [](dlist& l) { l.insert(l.head(), 1); };
    auto do_push_back = [](dlist& l) { l.push_back(1); };
    auto do_push_front = [](dlist& l) { l.push_front(1); };
    
    cout << "Checking if insert() is O(1)...";
    cout.flush();
    auto result1 = check_o1<dlist>(do_insert);
    had_timing_failure &= bool(result1);
    CHECK_WARN(
        (not result1),
        "insert() does not appear to run in O(1) time \n" <<
        "(avg = " << result1.average << ", stddev = " << result1.stddev <<
        ", " << result1.outlier_count << "/" << result1.tests_performed <<
        " tests exceeded 2*stddev; outlier average = " <<
        result1.outlier_average << ")"
    );    
    OK();

    cout << "Checking if push_back() is O(1)...";
    cout.flush();
    auto r2 = check_o1<dlist>(do_push_back);
    had_timing_failure &= bool(r2);
    CHECK_WARN(
        (not r2),
        "push_back() does not appear to run in O(1) time\n" <<
        "(avg = " << r2.average << ", stddev = " << r2.stddev <<
        ", " << r2.outlier_count << "/" << r2.tests_performed <<
        " tests exceeded 2*stddev; outlier average = " <<
        r2.outlier_average << ")"
    );
    OK();

    cout << "Checking if push_front() is O(1)...";
    cout.flush();
    auto r3 = check_o1<dlist>(do_push_front);
    had_timing_failure &= bool(r3);
    CHECK_WARN(
        (not r3),
        "push_front() does not appear to run in O(1) time\n" <<
        "(avg = " << r3.average << ", stddev = " << r3.stddev <<
        ", " << r3.outlier_count << "/" << r3.tests_performed <<
        " tests exceeded 2*stddev; outlier average = " <<
        r3.outlier_average << ")"
    );
    OK();

    if(had_timing_failure)
        cout << "Some O(1) tests failed; this does not necessarily mean your "
                "functions are not O(1). Try re-running the test suite; only "
                "if you get consistent failure should you be concerned." << endl;

    return true;    
}

int main(int argc, char** argv) 
{
    init_tests();

    bool run_big_o_tests = 
        argc <= 1 or 
        std::string(argv[1]) != "--skip-big-o";

    std::cout << "\u001b[37;1m*** Starting dlist tests ***" << ANSI_NORMAL() << std::endl;

	bool result = true;
    result = test_empty() and result;
    result = test_insert() and result;
    result = test_erase_node() and result;
    result = test_erase_index() and result;
    result = test_remove() and result;
    result = test_push_back() and result;
    result = test_push_front() and result;
    result = test_pop_front() and result;
    result = test_pop_back() and result;
    result = test_to_vector() and result;
    result = test_copy() and result;

    if(run_big_o_tests)
        result = test_o1_operations() and result;

    return_extra_credit();

    if(not result) 
    	return had_timing_failure ? 7 : 1;   // At least one test did not pass.	    
    else {
    	// All tests passed
        std::cout << "\u001b[32;1m*** All tests passed! ***" << ANSI_NORMAL() << std::endl;   
    	return 0;
   	}
}
