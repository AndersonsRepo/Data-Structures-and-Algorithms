/*
 * assign3_test.cpp
 * Test suite for assignment 3, mergesort on singly-linked lists.
 */
#include <cassert>

/* ===========================================================================
   BEGIN STANDARD PREAMBLE
   =========================================================================*/
   
/* ASSIGNMENT_URL
   Update this to the URL of the assignment. This will be used to generate links to 
   the test descriptions.
*/
#define ASSIGNMENT_URL "https://fccsci.twicetwo.com/cs133/assignment3.html"

#include <iostream>
#include <random>
#include <vector>
#include <functional>
#include <algorithm>
#include <climits>

#define ANSI_NORMAL() "\e[0m"
#define ANSI_GREEN()  "\e[0;92m"
#define ANSI_YELLOW() "\e[0;93m"
#define ANSI_RED()    "\e[0;91m"
#define ANSI_UL()     "\e[4m"

#if defined(__WIN64) || defined(__WIN32)

#include <windows.h>

void init_tests()
{
    // Enable UTF-8 in the terminal.
    // This could fail but I don't think it's important enough to worry about.
    SetConsoleOutputCP(CP_UTF8)

    // Enable ANSI in the terminal
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    // This might happen if stdout is redirected.
    if(console_handle == INVALID_HANDLE_VALUE)
        return;

    // This might also fail but I don't think we need to care about that.
    DWORD console_mode = 0;
    GetConsoleMode(console_handle, &console_mode)

    SetConsoleMode(console_handle, 
        console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    );

    // Let the student know that we can't perform memory tests on windows.
    std::cout << ANSI_YELLOW() 
              << "Running on Windows: memory-tracking tests will not be performed!" 
              << ANSI_NORMAL()
              << std::endl;
}

template<typename Fn>
bool call_check_alloc(Fn&& f)
{
    f();
    return true;
}

#else

/* Allocation tracking
   ---------------------------------------------------------------------------
   These functions are used to call student-defined code while tracking whether
   they do any dynamic allocation, either via `new`, or via the lower-level
   `malloc` and friends.

   This ability is *only* available on platforms where dlsym is available. 
   That's probably just G++/Clang.
*/
#include <new>
#include <dlfcn.h>

// Has an allocation occurred since we last cleared this flag?
bool allocation_occurred = false;

// Override malloc
extern "C" void* malloc(size_t size) 
{
    allocation_occurred = true;
    using malloc_func_t = void*(*)(size_t);
    malloc_func_t real_malloc = reinterpret_cast<malloc_func_t>(dlsym(RTLD_NEXT, "malloc"));

    return real_malloc(size);
}

// Override calloc
extern "C" void* calloc(size_t num, size_t size) 
{
    allocation_occurred = true;
    using calloc_func_t = void*(*)(size_t, size_t);
    calloc_func_t real_calloc = reinterpret_cast<calloc_func_t>(dlsym(RTLD_NEXT, "calloc"));
    return real_calloc(num, size);
}

// Override realloc
extern "C" void* realloc(void* ptr, size_t size) 
{
    allocation_occurred = true;
    using realloc_func_t = void*(*)(void*, size_t);

    realloc_func_t real_realloc = reinterpret_cast<realloc_func_t>(dlsym(RTLD_NEXT, "realloc"));
    return real_realloc(ptr, size);
}

// Override new
void* operator new(size_t size) 
{
    allocation_occurred = true;
    return malloc(size);
}

// Override new[]
void* operator new[](size_t size) 
{
    allocation_occurred = true;
    return malloc(size);
}

void init_tests()
{
    // No-op
}

/* call_check_alloc(f)
   Calls `f()` and returns true if `f()` allocated any dynamic memory during
   the call.
*/
template<typename Fn>
bool call_check_alloc(Fn&& f)
{
    allocation_occurred = false;
    f();

    return allocation_occurred;
}

#endif

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
            const int counter__ = __COUNTER__;                               \
            std::cout << "\n\u001b[31;1mTEST "                               \
                         "(" << __FILE__ << ":" << __LINE__ << ")" <<        \
                         " FAILED:\u001b[0m " << msg << '\n';                \
            std::cout << "(See \u001b[32;1m" << ASSIGNMENT_URL <<            \
                         "#test" << counter__ << ANSI_NORMAL() << " for info.)\n"; \
            return false;                                                    \
        }                                                                    \
    } while(false);                              

#else
// This part is used to list the checks. This will probably only work on GCC/Clang, 
// but students should never need to use this.
#define DO_PRAGMA(x)       _Pragma( #x )
#define PRINT(m)           DO_PRAGMA(GCC warning #m)
#define CHECK(cond, msg)                                                     \
    do {                                                                     \
        PRINT(CHECK msg)                                                     \
    } while(false);

#endif

void OK() { std::cout << ANSI_GREEN() << "OK" << ANSI_NORMAL() << "\n"; } 

void SECTION(const char* s)
{
    std::cout << ANSI_GREEN() << "*** " << s << " ***" << ANSI_NORMAL() << "\n"; 
}

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

/* ===========================================================================
   END STANDARD PREAMBLE
   =========================================================================*/

#include <unordered_set>

/* node
   The standard node type used in the assignment.
*/
struct node 
{
    int value;
    node* next;
};

/* print
   This exists so that we can easily print lists, via just

    cout << print(l) << endl;
*/
struct print 
{
    print(node* n) : n(n) { }

    node* n;
};

std::ostream& operator<< (std::ostream& out, const print& p)
{
    node* n = p.n;

    out << "{ ";
    while(n != nullptr) {
        out << n->value << " ";
        n = n->next;
    }
    out << "}";
    return out;
}

node* mergesort(node* input);
node* mergesort(node* input, int length);
node* merge(node* left, node* right);

/* is_valid_list(head)
   Returns true if `head` is the head of a valid list (does not contain a 
   cycle) and, if true, sets `length` to the length of the list.
*/
bool is_valid_list(node* head, int& length)
{
    std::unordered_set<node*> seen_nodes;
    node* n = head;
    int l = 0;
    while(n != nullptr) {
        if(seen_nodes.count(n) == 0) {
            // Unseen node, add to set, increment l, move to next node
            seen_nodes.insert(n);
            ++l;
            n = n->next;
        }
        else {
            // Seen an already seen node for a second time; cycle
            return false;
        }
    }

    // Only if we get to the end of the list do we update length
    length = l;
    return true;
}

bool is_valid_list(node* head)
{
    int l;
    return is_valid_list(head, l);
}

/* safe_get_length(h)
   Get the length of a list; if the list is not valid, asserts.
*/
int safe_get_length(node* head)
{
    int l = -1;
    bool result = is_valid_list(head, l);
    assert(result == true);

    return l;
}

/* is_sorted(head)
   Returns true if head is properly sorted.
*/
bool is_sorted(node* head)
{
    assert(is_valid_list(head));

    node* n = head;
    while(n != nullptr and n->next != nullptr) {
        if(n->value > n->next->value)
            return false;

        n = n->next;
    }

    return true;
}

/* contains(head, data)
   Returns true if head contains the values in data, in the same order.
*/
bool contains(node* head, const std::vector<int>& data)
{
    assert(is_valid_list(head));

    node* n = head;
    for(int x : data) {
        if(n == nullptr or n->value != x)
            return false;
        n = n->next;
    }

    return true;
}

/* destroy(head)
   Destroy the given list. Student code does not have an opportunity to destroy
   the list, so we have to if we want the code to pass with no memory leaks.
*/
void destroy(node* head)
{
    node* n = head;
    while(n != nullptr) {
        node* nn = n->next;
        delete n;
        n = nn;
    }
}

node* make_list(const std::vector<int>& v)
{
    node sentinel{-10000, nullptr};
    node* tail = &sentinel;
    for(int x : v) {
        tail->next = new node{x, nullptr};
        tail = tail->next;
    }

    return sentinel.next;    
}

template<typename ... Args>
node* make_list(Args ... args)
{
    return make_list(
        std::vector<int>{ std::forward<Args>(args)... }
    );
}

/* get_nodes(l)
   Collects the node*s of the list into a vector.
*/
std::vector<node*> get_nodes(node* list)
{
    node* n = list;
    std::vector<node*> v;
    while(n != nullptr) {
        v.push_back(n);
        n = n->next;
    }

    return v;
}

bool has_nodes(node* list, const std::vector<node*>& nodes)
{
    node* n = list;
    size_t i = 0;
    while(n != nullptr and i != nodes.size()) {
        if(n != nodes[i])
            return false;
        n = n->next;
        ++i;
    }

    return n == nullptr and i == nodes.size();
}

bool no_common_nodes(node* a, node* b)
{
    auto na = get_nodes(a);
    auto nb = get_nodes(b);

    for(node* i : na)
        for(node* j : nb)
            if(i == j)
                return false;

    return true;
}

bool has_values(node* list, const std::vector<int>& values)
{
    node* n = list;
    size_t i = 0;
    while(n != nullptr and i != values.size()) {
        CHECK(n->value != values[i],
            "Incorrect value found in list: expected " <<
            values[i] << " but found " <<
            n->value << " (list = " << print(list) << ")"
        );
        n = n->next;
        ++i;
    }

    if(n != nullptr or i != values.size()) {
        CHECK(n == nullptr,
            "List is too long: expected end-of-list but found rest of list = " <<
            print(n)
        );
        CHECK(i >= values.size(),
            "List is too short: expected " << values[i] << 
            " but found end-of-list."
        );
    }

    return true;
}

/* is_merged(lefts, rights, merged)
   Checks to see if the list `merged` is the result of merging the nodes from
   `lefts` and `rights`. This checks both to make sure that the merged list
   does not include any nodes other than those in lefts/rights, and also that
   it merged them correctly with respect to the values they contain.
*/
bool is_merged(
    const std::vector<node*>& lefts, 
    const std::vector<node*>& rights,
    node* merged
)
{
    size_t i = 0, j = 0;
    node* n = merged;

    CHECK(not is_valid_list(n),
        "Result of merge() is not a valid list (contains a cycle?)"
    );

    while(i < lefts.size() and j < rights.size()) {
        CHECK(n == nullptr, 
            "Merged list does not contain all the values it should (ended too early)");

        if(lefts[i]->value < rights[j]->value) {
            CHECK(n->value != lefts[i]->value,
                "Wrong value in merged list: expected " << 
                lefts[i]->value << 
                ", found " << 
                n->value <<
                " (merged list = " << print(merged) << ")"
            );

            CHECK(n != lefts[i],
                "Invalid node found in merged list (all merged nodes must come from the left/right input lists).");

            ++i; 
            n = n->next;
        }
        else {
            CHECK(n->value != rights[j]->value,
                "Wrong value in merged list: expected " << 
                rights[j]->value <<
                ", found " <<
                n->value
            );

            CHECK(n != rights[j],
                "Invalid node found in merged list (all merged nodes must come from the left/right input lists).");

            ++j;
            n = n->next;
        }
    }

    // Now we've reached the end of lefts or rights, so we check the remainder
    // of the other.
    while(i < lefts.size()) {
        CHECK(n == nullptr, 
            "Merged list does not contain all the values it should (ended too early)");
        CHECK(n->value != lefts[i]->value,
            "Wrong value in merged list: expected " << 
            lefts[i]->value << 
            ", found " <<
            n->value
        );

        CHECK(n != lefts[i],
            "Invalid node found in merged list (all merged nodes must come from the left/right input lists).");

        ++i; 
        n = n->next; 
    }

    while(j < rights.size()) {
        CHECK(n == nullptr, 
            "Merged list does not contain all the values it should (ended too early)");
        CHECK(n->value != rights[j]->value,
            "Wrong value in merged list: expected " << 
            rights[j]->value <<
            ", found " <<
            n->value
        );

        CHECK(n != rights[j],
            "Invalid node found in merged list (all merged nodes must come from the left/right input lists).");

        ++j;
        n = n->next;        
    }

    CHECK(n != nullptr,
        "Merged list contains more elements than the input left/right lists "
        "(extra value(s) = " << print(n) << ")"
    );

    return true;
}

/* ===========================================================================
   Begin tests
   ===========================================================================*/

bool test_merge()
{
    std::cout << "Testing merge() with empty lists...";
    node* left = nullptr;
    node* right = nullptr;

    node* output = merge(left, right);
    CHECK(output != nullptr,
        "Merging two empty lists must return an empty list.")

    OK();
    left = right = output = nullptr;

    std::cout << "Testing merge() with right list empty (left = 1,2,3)...";
    left = make_list(1,2,3);
    auto left_nodes = get_nodes(left);
    output = merge(left, right);

    CHECK(
        not has_nodes(output, left_nodes),
        "Output list has incorrect node pointers (all output nodes must exist in the input lists).");
    if(not has_values(output, {1,2,3}))
        return false;

    destroy(output);
    OK();
    left = right = output = nullptr;

    std::cout << "Testing merge() with left list empty (right = 1,2,3)...";
    left = nullptr;
    right = make_list(1,2,3);
    auto right_nodes = get_nodes(right);
    output = merge(left, right);

    CHECK(
        not has_nodes(output, right_nodes),
        "Output list has incorrect node pointers (all output nodes must exist in the input lists).");
    if(not has_values(output, {1,2,3}))
        return false;

    destroy(output);
    left = right = output = nullptr;
    OK();

    // ------------------------------------------------------------------------

// Helper macro for testing merges.
#define CHECK_MERGE(msg, left_values, right_values)                               \
    std::cout << msg;                                                             \
    left = make_list left_values;                                                 \
    right = make_list right_values;                                               \
    std::cout << "(left = " << print(left);                                       \
    std::cout << "; right = " << print(right) << ")";                             \
    left_nodes = get_nodes(left);                                                 \
    right_nodes = get_nodes(right);                                               \
    output = merge(left, right);                                                  \
    CHECK(not is_merged(left_nodes, right_nodes, output),                         \
        "Result is not merged");                                                  \
    destroy(output);                                                              \
    OK();  

    CHECK_MERGE(
        "Testing merge() with left list < right list (same lengths)...",
        (1,2,3),
        (6,7,8)
    );

    CHECK_MERGE(
        "Testing merge() with left list > right list (same lengths)...",
        (6,7,8),
        (1,2,3)
    );
    
    CHECK_MERGE(
        "Testing merge() with left list < right list (different lengths)...",
        (1,2,3,4,5),  
        (6,7,8)
    );

    CHECK_MERGE(
        "Testing merge() with left list > right list (different lengths)...",
        (6,7,8),
        (1,2,3,4,5)
    ); 

    CHECK_MERGE(
        "Testing merge() with left list < right list (different lengths)...",
        (1,2,3),  
        (6,7,8,9,10)
    );

    CHECK_MERGE(
        "Testing merge() with left list > right list (different lengths)...",
        (6,7,8,9,10),
        (1,2,3,4,5)
    );

    CHECK_MERGE(
        "Testing merge() with alternating sequences (same lengths)...",
        (1,3,5,7),
        (2,4,6,8)
    );

    CHECK_MERGE(
        "Testing merge() with alternating sequences (same lengths)...",
        (2,4,6,8),
        (1,3,5,7)
    );

    CHECK_MERGE(
        "Testing merge() with alternating sequences (different lengths)...",
        (1,3,5,7,9,11),
        (2,4,6,8)
    );

    CHECK_MERGE(
        "Testing merge() with alternating sequences (different lengths)...",
        (2,4,6,8),
        (1,3,5,7,9,11)
    );

    CHECK_MERGE(
        "Testing merge() with alternating sequences (different lengths)...",
        (1,3,5,7),
        (2,4,6,8,10,12)
    );

    CHECK_MERGE(
        "Testing merge() with alternating sequences (different lengths)...",
        (2,4,6,8,10,12),
        (1,3,5,7)
    );

    #undef CHECK_MERGE

    return true;
}

bool test_mergesort()
{
    {
        // This tests specifically calling mergesort(..., 0) with the input
        // list not empty. Sometimes students use length <= 1 as the base case,
        // always returning a new node, which is not correct when length == 0.
        // If we just test the single-argument mergesort() function, we don't
        // catch that. We should add more tests for the mergesort(node*,int)
        // overload.

        std::cout << "Testing mergesort with length = 0... ";
        std::cout.flush();
        node* l = make_list({ 1, 2, 3, 4, 5 });
        node* result = mergesort(l, 0);
 
        CHECK(result != nullptr,
            "After calling mergesort(list, 0), returned list is not empty (nullptr)");
        destroy(l);

        OK();
    }

    // The main tests here are sorted-ness, and not sharing node*s with the 
    // input lists.
    int test_length = 0;

    while(test_length <= 100) {
        // How many tests do we do at this length?
        const int num_tests = test_length < 8 ? (1 << test_length) : 128 ;

        std::cout << "Testing mergesort with length = " << test_length << "... ";
        std::cout.flush();

        for(int i = 0; i < num_tests; ++i) {

            // Build a random list of length test_length.
            std::vector<int> data = make_random_vector(test_length);
            node* l = make_list(make_random_vector(test_length));
            node* result = mergesort(l);

            bool is_valid = is_valid_list(result);
            CHECK(not is_valid,
                "After mergesort-ing, result is not a valid list");

            if(is_valid) {

                int actual_length = safe_get_length(result);
                CHECK(actual_length != test_length,
                    "After mergesort-ing, result has incorrect length " <<
                    "(expected " << test_length << 
                    ", actual length = " << actual_length
                );

                CHECK(not is_sorted(result),
                    "After mergesort-ing list, result is not sorted\n" <<
                    "Input list: " << print(l) << "\n" <<
                    "Result:     " << print(result) << "\n"
                );

                std::sort(data.begin(), data.end());
                CHECK(not contains(result, data),
                    "After mergesort-ing, result does not contain correct values\n" <<
                    "Result = " << print(result) << "\n"
                );

                CHECK(not no_common_nodes(result, l),
                    "After mergesort-ing, result shares some nodes with the input list"
                );

                destroy(result);
                destroy(l);
            }
        }

        OK();

        if(test_length <= 16)
            ++test_length;
        else
            // Beyond 16, we use exponential growth, so that it doesn't take
            // too long.
            test_length = int(1.5*test_length) + 1;
    }

    return true;
}

int main()
{
    init_tests();

    std::cout << "\u001b[37;1m*** Starting merge/mergesort tests ***" << ANSI_NORMAL() << std::endl;

    bool result = true;
    result = test_merge() and result;
    result = test_mergesort() and result;

    // return_extra_credit();

    if(not result)
        return 1;

    std::cout << "\u001b[32;1m*** All tests passed! ***" << ANSI_NORMAL() << std::endl;   
    return 0;
}
