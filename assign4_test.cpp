/*
 * assign4_test.cpp
 * Tests for assignment 4: AVL trees.
 */
/* ===========================================================================
   BEGIN STANDARD PREAMBLE
   =========================================================================*/
   
/* ASSIGNMENT_URL
   Update this to the URL of the assignment. This will be used to generate links to 
   the test descriptions.
*/
#define ASSIGNMENT_URL "https://fccsci.twicetwo.com/cs133/assignment4.html"

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
#define ANSI_RED()    "\e[1;91m"
#define ANSI_UL()     "\e[4m"

#define ANSI_W_BKG()  "\e[0;100m"

#define ANSI_RED_W_BKG() "\e[1;91;100m"

#if defined(__WIN64) || defined(__WIN32)

#include <windows.h>
#include <ctime>

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
}

long long get_current_process_time()
{
    // On windows, we hope that std::clock() is precise enough to use for
    // benchmarking.
    return std::clock();
}

#else // *nix

#include <time.h>

void init_tests()
{
    // No-op
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
            const int _count_ = __COUNTER__;                                 \
            std::cout << "\n\u001b[31;1mTEST #" << _count_ << " "            \
                         "(" << __FILE__ << ":" << __LINE__ << ")" <<        \
                         " FAILED:" << ANSI_NORMAL() << " " << msg << '\n';  \
            std::cout << "(See \u001b[32;1m" << ASSIGNMENT_URL <<            \
                         "#test" << _count_ << ANSI_NORMAL() << " for info.)\n";\
            return false;                                                    \
        }                                                                    \
    } while(false); 

#define CHECK_WARN(cond,msg)                                                 \
    do {                                                                     \
        if(cond) {                                                           \
            const int _count_ = __COUNTER__;                                       \
            std::cout << "\n" << ANSI_YELLOW() << "WARNING, TEST #" << _count_ << " " \
                         "(" << __FILE__ << ":" << __LINE__ << ")" <<        \
                         " FAILED:" << ANSI_NORMAL() << " " << msg << '\n';  \
            std::cout << "(See \u001b[32;1m" << ASSIGNMENT_URL <<            \
                         "#test" << _count_ << ANSI_NORMAL() << " for info.)\n";   \
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
    for(std::size_t i = 0; i < len; ++i) {

        // For tree purposes, we don't want our vector to contain duplicates.
        int x = 0;
        do {
            x = gen();

            // restrict values to the range -9 ... 99 (fits in two char cells)
            x = (x % 109) - 9;
        } while(std::find(ret.begin(), ret.end(), x) != ret.end());

        ret.at(i) = x;
    }

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

#include <algorithm>
#include <map>
#include <queue>
#include <set>

// The server only has G++ 6, which lacks std::variant, so if it's not available
// we fall back to Boost.Variant.
#if __has_include(<variant>)

    #include <variant>

    template<typename ... Ts>
    using variant = std::variant<Ts...>;

    template <typename T, typename... Ts>
    bool holds_alt(const variant<Ts...>& v) noexcept
    {
        return std::holds_alternative<T>(v);
    }

    template<typename T, typename... Ts>
    const T& getv(const variant<Ts...>& v) noexcept
    {
        return std::get<T>(v);
    }

#else

    #include <boost/variant.hpp>

    template<typename ... Ts>
    using variant = boost::variant<Ts...>;

    /* holds_alt<T>(v)
       Returns true if the variant v currently holds a value of type T.
       Boost.Variant doesn't provide a function equivalent to std::holds_alternative,
       so this is our version.
    */
    template <typename T, typename... Ts>
    bool holds_alt(const variant<Ts...>& v) noexcept
    {
        return boost::get<T>(&v) != nullptr;
    }

    template<typename T, typename... Ts>
    const T& getv(const variant<Ts...>& v) noexcept
    {
        return boost::get<T>(v);
    }

#endif

#ifndef CHECK_ASSIGN_SOLN
    #include "avl_tree.hpp"
    #include "avl_tree.hpp"
#else
    #include "avl_tree_soln.hh"
#endif

// Enable cycle checking?
const bool cycle_check = true;

/******************************************************************************
 Begin Assignment 4 Test code
 ******************************************************************************/


bool operator==(const avl_tree& a, const avl_tree& b);

std::ostream& operator<< (std::ostream&, const avl_tree& t);


#define METHOD_IS_IMPLEMENTED(name)                                           \
    try {                                                                     \
        t.name();                                                             \
        has_ ## name ## _fn = true;                                           \
    }                                                                         \
    catch(not_implemented&) { }

#define METHOD_IS_IMPLEMENTED2(name, arg)                                     \
    try {                                                                     \
        t.name(arg);                                                          \
        has_ ## name ## _fn = true;                                           \
    }                                                                         \
    catch(not_implemented&) { }    


#define LINE_HORIZ() "─"
#define LINE_VERT()  "│"
#define CORNER_UL()  "┘"
#define CORNER_LL()  "┐"
#define CORNER_UR()  "└"
#define CORNER_LR()  "┌"
#define TEE_ULR()    "┴"

class text_grid {
  public:
    text_grid(int w, int h) :
        attrs(h, std::vector<std::string>(w, "")),
        grid(h, std::vector<std::string>(w, "")),        
        w(w),
        h(h)
    { 
        assert( w >= 1 );
        assert( h >= 1 );
    }

    text_grid(std::string s) :
        text_grid(s.length(), 1)
    { 
        print(0,0,s);
    }

    int width() const
    {
        return w;
    }

    int height() const
    {
        return h;
    }

    /* set(x,y,c)
       Set the character to print at (x,y).
    */
    void set(int x, int y, char c)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());

        set(x,y,std::string(1,c));
    }

    void set(int x, int y, std::string s)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());        
        grid.at(y).at(x) = s;
    }

    void set(int x, int y, char c, std::string a)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height()); 

        set(x,y,std::string(1,c),a);
    }

    void set(int x, int y, std::string s, std::string a)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());        
        grid.at(y).at(x) = s;

        if(not a.empty())
            attrs.at(y).at(x) = a;
    }

    void clear_attr(int x, int y)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());        
        attrs.at(y).at(x) = "";
    }

    /* print(x,y,s)
       Print the string s with its left anchored at (x,y). This
       prints one character per cell.
    */
    void print(int x, int y, std::string s)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());        
        for(char c : s)
            set(x++,y,c);
    }

    void print(int x, int y, std::string s, std::string a)
    {
        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());

        for(char c : s)
            set(x++,y,c,a);
    }

    /* print(x,y,n)
       Print the node n centered at (x,y). Nodes are 7 cells wide, so they will
       extend from x-3 to x+3. Nodes are printed as either

                                #VVVVV#

       where VVVVV is the value of the node, or as

                                #VV#HH#

       where HH is the height of the node, if heights are printed.

       If n == nullptr then this prints

                                #<NUL>#
    */
    void print(
        int x, int y, 
        avl_tree::node* n, 
        bool show_height = false,
        bool mark_value = false,
        bool mark_height = false
    )
    {
        // Center nodes.
        x -= node_width/2;

        assert(x >= 0 and x < width());
        assert(y >= 0 and y < height());

        if(n == nullptr) {
            print(x, y, " <NUL> ", ANSI_W_BKG());
            return;
        }

        std::string val = std::to_string(n->key);

        if(val.length() > 2) {
            val.resize(2);
            val.back() = '~'; // Show that value was truncated
        }

        if(not show_height) {
            // Print just the value

            // Center value
            std::string text = val.length() == 2 ? "   " + val + "  " : 
                               val.length() == 1 ? "   " + val + "   ": 
                                                   "   ?   " ; // Should never happen

            print(
                x, y, 
                text, 
                (mark_value ? ANSI_RED_W_BKG() : ANSI_W_BKG()) 
            );
        }
        else { // Show height as well

            // This prints as ␣VV␣HH␣ but mark_value and mark_height might
            // be set separately, so we can't just print the entire text all
            // at once.

            std::string hgt = std::to_string(n->height);
            if(hgt.length() > 2) {
                hgt.resize(2);
                hgt.back() = '~';
            }

            // Make both value and height strings have length == 2
            if(val.length() < 2)
                val = std::string(2 - val.length(), ' ') + val;
            if(hgt.length() < 2)
                hgt = std::string(2 - hgt.length(), ' ') + hgt;

            print(x, y, " ", ANSI_W_BKG());

            print(x + 1, y, val,  
                (mark_value ? ANSI_RED_W_BKG() : ANSI_W_BKG()) );

            print(x + 3, y, " ", ANSI_W_BKG());

            print(x + 4, y, hgt,
                (mark_height ? ANSI_RED_W_BKG() : ANSI_W_BKG()) ); 
                
            print(x + 6, y, " ", ANSI_W_BKG());               
        }
    }

    /* hconnect(x0,y0,x1,y1)
       Draws a horizontal connector between (x0, (y0+y1)/2) and
       (x1, (y0+y1)/2). It is assumed that the corner at x0 points up, and the
       corner at x1 points down.
    */
    void hconnect(int x0, int y0, int x1, int y1, std::string attr = "")
    {
        int y = (y1 + y0) / 2;

        int start_x = x0 < x1 ? x0 : x1;
        int end_x = x1 > x0 ? x1 : x0;

        for(int x = start_x; x < end_x; ++x)
            set(x, y, LINE_HORIZ(), attr);   

        // Have to check x0 < x1 to know whether the corner is left or right.
        if(x0 < x1) {
            set(x1, y, CORNER_LL());
            set(x0, y, CORNER_UR());
        }
        else {
            set(x1, y, CORNER_LR());
            set(x0, y, CORNER_UL());            
        }
    }

    /* connect(x0,y0,x1,y1)
       Draw a connector line between (x0,y0) - (x1,x1). Note that we assume 
       all connectors start and end vertically, that is, we are only capable
       of drawing lines like

         (x0,y0)
            │
            └─────────────┐             <------ mid
                          │
                       (x1,y1)

       The one special case is purely horizontal lines.

       TODO: This doesn't work correctly.
    */
    void connect(int x0, int y0, int x1, int y1, std::string attr = "")
    {
        // Ensure that (x0,y0) is above/left of (x1,y1)
        if(y1 < y0)
            std::swap(y1,y0);
        if(x1 < x0)
            std::swap(x1,x0);

        if(y1 == y0) {
            // Pure horizontal case
            for(int x = x0; x <= x1; ++x)
                set(x,y0,LINE_HORIZ(), attr);
        }
        else {
            // Vertical midpoint; this is where the horizontal section will
            // be drawn.
            int vmid = y0 + (y1 - y0) / 2;

            // Draw vertical section 1
            for(int y = y0; y < vmid; ++y)
                set(x0, y, LINE_VERT(), attr);

            // Draw horizontal section
            for(int x = x0+1; x < x1; ++x)
                set(x, vmid, LINE_HORIZ(), attr);            

            // Draw vertical section 2
            if(vmid != y1)
                for(int y = vmid; y <= y1; ++y)
                    set(x1,y,LINE_VERT(), attr);

            // Draw the corners at the ends of the horizontal section
            set(x0,vmid,CORNER_UR(), attr);
            set(x1,vmid,CORNER_LL(), attr);
        }
    }

    static const int node_width = 7;

    // Tree-printing sometimes adds extra space to the right side of the image,
    // so this function removes it
    void right_crop()
    {
        // Find the max width of any row
        int max_w = -1;
        for(const std::vector<std::string>& row : grid) {
            int col = 0;
            int last_col = -1;
            for(const std::string& s : row) {
                if(not s.empty())
                    last_col = col;

                col++;
            }

            if(last_col+1 > max_w)
                max_w = last_col+1;
        }

        // Resize all rows
        for(std::vector<std::string>& row : grid)
            row.resize(max_w);
        for(std::vector<std::string>& row : attrs)
            row.resize(max_w);

        w = max_w;
    }

  private:
    std::vector<std::vector<std::string>> attrs;    
    std::vector<std::vector<std::string>> grid;
    int w, h;

    friend std::ostream& operator<< (std::ostream& out, const text_grid& g)
    {
        out << CORNER_LR();
        for(int x = 0; x < g.width(); ++x)
            out << LINE_HORIZ();
        out << CORNER_LL() << std::endl;

        for(int y = 0; y < g.h; ++y) {
            out << LINE_VERT();
            for(int x = 0; x < g.w; ++x) {
                std::string s = g.grid.at(y).at(x);
                if(s.empty())
                    s = " ";

                out << g.attrs.at(y).at(x)
                    << s
                    << ANSI_NORMAL();
            }

            out << LINE_VERT() << std::endl;
        }

        out << CORNER_UR();
        for(int x = 0; x < g.width(); ++x)
            out << LINE_HORIZ();
        out << CORNER_UL() << std::endl;

        const bool print_debug_info = false;
        if(print_debug_info) {
            out << " * Width: " << g.w << std::endl
                << " * Height: " << g.h << std::endl;
        }

        return out;
    }
};
 

class assign4_test_runner {
  public:

    // -----------------------------------------------------------------------

    enum class traversal_order
    {
        PREORDER,
        INORDER,
        POSTORDER,
        LEVELORDER
    };

    /* traverse(t, fn)
       Calls `fn(n,p,left_cycle,right_cycle,level)` for each node `n` in the tree,
       with `p` being the node's actual parent. `left_cycle/right_cycle` will
       be set to true if n->left/n->right occur somewhere else in the tree.
       In this case, the traversal *will not* traverse into n->left/n->right.

       This function is safe to call on trees that may contain a downward
       cycle (it won't get stuck in an infinite loop).
    */
    template<
        traversal_order order = traversal_order::PREORDER,        
        bool want_nullptr = false,
        typename Fn = void         
    >
    static void traverse(
        avl_tree::node* n, 
        Fn&& f,
        std::set<avl_tree::node*>& seen_nodes, 
        avl_tree::node* p = nullptr,
        int level = 0
    )
    {
        static_assert(order != traversal_order::LEVELORDER,
            "traverse(node*) must not be called with LEVELORDER");

        if(n == nullptr) {
            if(want_nullptr)
                f(n,p,false,false,level);
        }
        else if(seen_nodes.count(n) != 0) {
            // This should never happen, since we catch it early below.
            assert(false);
        }
        else {
            seen_nodes.insert(n);

            bool wrong_left = 
                n->left != nullptr and seen_nodes.count(n->left) > 0;
            bool wrong_right =
                n->right != nullptr and seen_nodes.count(n->right) > 0;

            if (order == traversal_order::PREORDER) {
                f(n,p,wrong_left, wrong_right, level);

                if(not wrong_left)
                    traverse<order,want_nullptr>(n->left, std::forward<Fn>(f), seen_nodes, n, level+1);

                if(not wrong_right)
                    traverse<order,want_nullptr>(n->right, std::forward<Fn>(f), seen_nodes, n, level+1);
            }
            else if (order == traversal_order::INORDER) {
                if(not wrong_left)
                    traverse<order,want_nullptr>(n->left, std::forward<Fn>(f), seen_nodes, n, level+1);

                f(n,p,wrong_left, wrong_right, level);

                if(not wrong_right)
                    traverse<order,want_nullptr>(n->right, std::forward<Fn>(f), seen_nodes, n, level+1);

            }
            else if (order == traversal_order::POSTORDER) {
                if(not wrong_left)
                    traverse<order,want_nullptr>(n->left, std::forward<Fn>(f), seen_nodes, n, level+1);

                if(not wrong_right)
                    traverse<order,want_nullptr>(n->right, std::forward<Fn>(f), seen_nodes, n, level+1);

                f(n,p,wrong_left, wrong_right, level);
            }
        }
    }

    template<
        bool want_nullptr = false,
        typename Fn = void
    >
    static void traverse_levelorder(const avl_tree& t, Fn&& f)
    {
        struct queue_item
        {
            avl_tree::node* n;
            avl_tree::node* p;
            int level;
        };

        std::queue<queue_item> q;
        std::set<avl_tree::node*> seen_nodes;

        if((t.rt == nullptr and want_nullptr) or 
            t.rt != nullptr) 
            q.push({t.rt, nullptr, 0});

        if(t.rt != nullptr)
            seen_nodes.insert(t.rt);

        while(not q.empty()) {
            queue_item e = q.front();
            q.pop();

            bool wrong_left = e.n->left != nullptr and
                seen_nodes.count(e.n->left) > 0;

            bool wrong_right = e.n->right != nullptr and
                seen_nodes.count(e.n->right) > 0;

            f(e.n, e.p, wrong_left, wrong_right, e.level);

            if(want_nullptr or e.n->left != nullptr)
                if(not wrong_left) {
                    q.push({ e.n->left, e.n, e.level+1 });
                    seen_nodes.insert(e.n->left);
                }

            if(want_nullptr or e.n->right != nullptr)
                if(not wrong_right) {
                    q.push({ e.n->right, e.n, e.level+1 });
                    seen_nodes.insert(e.n->right);
                }
        }

    }

    template<
        traversal_order order = traversal_order::PREORDER,        
        bool want_nullptr = false,
        typename Fn = void   
    >
    static void traverse(const avl_tree& t, Fn&& f)
    {
        std::set<avl_tree::node*> seen_nodes;
        if (order != traversal_order::LEVELORDER)
            traverse<order, want_nullptr, Fn>(
                t.rt, 
                std::forward<Fn>(f), 
                seen_nodes, 
                nullptr
            );
        else
            traverse_levelorder<want_nullptr,Fn>(
                t, 
                std::forward<Fn>(f)
            );
    }

    // ------------------------------------------------------------------------

    // These record whether the named functions are implemented or not.
    bool has_height_fn = false;
    bool has_empty_fn = false;
    bool has_find_fn = false;
    bool has_insert_fn = false;
    bool has_size_fn = false;
    bool has_copy_fn = false;
    bool has_merge_with_fn = false;
    bool has_rotate_fn = false;

    bool start()
    {
        bool result = true;
        std::cout << "\u001b[37;1m*** Starting AVL tree tests ***" << ANSI_NORMAL() << std::endl;

        // First, record which methods are implemented for later testing.
        {
            avl_tree t;
            
            METHOD_IS_IMPLEMENTED(height);
            METHOD_IS_IMPLEMENTED(empty);
            METHOD_IS_IMPLEMENTED2(find,0);
            METHOD_IS_IMPLEMENTED2(insert,0);
            METHOD_IS_IMPLEMENTED2(size,nullptr);
            METHOD_IS_IMPLEMENTED2(merge_with, {});

            // copy and destroy are used by the copy constructor/assignment and 
            // destructor, respectively, which get called automatically, so we
            // can't detect their implementation by having them throw. destroy()
            // is detected by running inside valgrind, while we check for copy
            // manually
            {
                avl_tree original;
                original.rt = new avl_tree::node{12, nullptr, nullptr, nullptr, 0};

                avl_tree copy = original;

                // 
                if(copy.rt != nullptr) 
                    has_copy_fn = true;

                delete original.rt;
                original.rt = nullptr;
            }

            {
                avl_tree t;
                // check for rotate
                t.rt = new avl_tree::node{ 10, nullptr, nullptr, nullptr, 2 };
                t.rt->left = new avl_tree::node{ 5, nullptr, nullptr, t.rt, 1 };

                METHOD_IS_IMPLEMENTED2(rotate, t.rt->left);
            }

            // We don't have a reliable way to check for destroy(), but the
            // test runner script will check for memory leaks, so that will 
            // catch it.
        }

        // We need rotate to work to create any trees
        result = result & test_rotate();

        // We need rotate in order to do insert.
        bool insert_basic = false, insert_full = false;        
        bool insert_result = test_insert(insert_basic, insert_full);
        result = result & insert_result;

        // Lots of other tests rely on us having a minimally working insert function.
        if(not insert_basic)
            has_insert_fn = false;

        // Once we have insert we can construct trees to test the other 
        // functions in.
        result = result & test_find();
        result = result & test_size();
        result = result & test_height();
        result = result & test_empty();

        result = result & test_copy();

        // Extra credit
        test_merge_with();

        return result;        
    }

    bool test_rotate()
    {
        if(not has_rotate_fn) {
            std::cout << "rotate() not implemented, skipping." << std::endl;
            return false;
        }

        std::cout << "Testing rotate()...";

        // For rotate(), there are basically eight test cases we need to 
        // test: left/right when the parent *is not* the root, and left/right
        // when the parent *is* the root (to make sure the root is correctly
        // updated. For each of those, we need to test with Y being nullptr,
        // and not being nullptr. For each test case, we need to check that
        // the rotate produced the correct result, that the result is a valid
        // tree (which includes checking that heights and parents were updated
        // correctly).

        // Also, sometimes people implement rotate() assuming that the tree is
        // a BST (i.e., they use a comparison between c->key and p->key to 
        // determine which rotate to perform) so it's important that the trees
        // we use to test be correct BSTs.

        // When we call check_tree on the result of a rotation, we disable
        // balance checking, because a few of the trees we test against are not
        // balanced (intentionally).

        #define CHECK_ROTATE(tree, child, result)                              \
            {                                                                  \
                const avl_tree& rotated_result = rotated(tree,child);          \
                CHECK(not check_tree(rotated_result, false),                   \
                    "After rotating " << child << " the result tree is not valid.\n" \
                    "Input tree:\n" << tree << "\nExpected result:\n" << result << \
                    "Actual result:\n" << rotated_result);                     \
                CHECK( not (rotated(tree, child) == result),                   \
                    "After rotating " << child << " the result is incorrect.\n" \
                    "Input tree:\n" << tree << "\nExpected result:\n" << result << \
                    "Actual result:\n" << rotated_result);                     \
            }

        auto left_root_no_y = build_tree({
          {   2   },
          { 1,  0 }
        });
        // The result of this rotation should be right_root_no_y   

        auto right_root_no_y = build_tree({
          {   1   },
          { 0,  2 }            
        });
        // The result of rotating this should be left_root_no_y

        CHECK_ROTATE(left_root_no_y, 1, right_root_no_y);  
        CHECK_ROTATE(right_root_no_y, 2, left_root_no_y);

        auto left_root_y = build_tree({
            {      3       },
            { 1,       0   },
          { 0,   2,  0,  0 }
        });

        auto right_root_y = build_tree({
          {       1        },
          {   0,      3    },
          { 0,  0,   2,  0 }            
        }); 

        CHECK_ROTATE(left_root_y, 1, right_root_y);
        CHECK_ROTATE(right_root_y, 3, left_root_y);       

        // This tree is used for testing both left/right rotates where p != root.
        // We either rotate(1,3) or rotate(10,9)
        auto non_root_no_y = build_tree({
            {        7        },
            {    3,      9    },
            {  1,   0, 0,  10 }
        });

        // If we rotate 1 with 3, this is the result
        auto left_non_root_no_y_result = build_tree({
            {        7        },
            {    1,      9    },
            {  0,   3, 0,  10 }
        });

        // If we rotate 10 with 9 this is the result
        auto right_non_root_no_y_result = build_tree({
            {        7        },
            {    3,      10   },
            {  1,   0, 9,  0  }
        });

        CHECK_ROTATE(non_root_no_y, 1, left_non_root_no_y_result);
        CHECK_ROTATE(non_root_no_y, 10, right_non_root_no_y_result);

        auto non_root_y = build_tree({
            {        7        },
            {    3,         9    },
            {  1,   0,   0,    11 },
            { 0,2, 0,0, 0, 0, 10, 0}  // y = 2 or 10
        });

        // If we rotate 1 with 3, this is the result
        auto left_non_root_y_result = build_tree({
            {        7        },
            {    1,        9    },
            {  0,   3,   0,   11 },
            { 0,0, 2,0, 0,0, 10, 0 }
        });

        // If we rotate 11 with 9 this is the result
        auto right_non_root_y_result = build_tree({
            {        7        },
            {    3,         11   },
            {  1,   0,   9,     0  },
            { 0,2, 0,0, 0,10, 0,  0 }
        });

        CHECK_ROTATE(non_root_y, 1, left_non_root_y_result);
        CHECK_ROTATE(non_root_y, 11, right_non_root_y_result);

        OK();
        return true;
    }

    bool test_insert(bool& insert_basic, bool& insert_full)
    {
        if(not has_insert_fn) {
            std::cout << "insert() not implemented, skipping." << std::endl;
            return false;
        }

        std::cout << "Testing insert() by inserting random values (no duplicates)...";

        std::vector<int> data = make_random_vector(20);
        for(int& x : data)
            x = (x % 109) - 9; // This gives values in the range of -9 to 99.

        {
            avl_tree t;
            for(int x : data) {
                t.insert(x);

                if(safe_find(t, x) == nullptr) {
                    insert_basic = false;
                    CHECK(safe_find(t,x) == nullptr,
                        "After inserting " << x << ", it does not exist in the "
                        "tree. Tree: " << t
                    );
                    return false;
                }
            }

            insert_basic = true;
        }        

        avl_tree t;
        int expected_size = 0;
        for(int x : data) {
            t.insert(x);
            ++expected_size;

            // TODO: Currently, this does not check to make sure that the student
            // code actual does the correct rotations, only that the tree is
            // AVL-balanced after each insert. 

            CHECK(not check_tree(t),
                "Tree is not valid after inserting " << x << "\n" <<
                t << '\n'
            );

            CHECK(safe_find(t, x) == nullptr,
                "After inserting " << x << ", value does not exist in the tree:\n" <<
                t << "\n"
            );

            if(has_height_fn) {
                int h = safe_height(t);
                CHECK(t.height() != h,
                    "After inserting " << x << ", tree.height() is incorrect (" <<
                    "actual height = " << h << ", tree.height() = " << t.height()
                    << ")\n"
                );
            }

            if(has_size_fn)
                CHECK(t.size() != count_nodes(t.rt),
                    "After inserting " << x << ", tree.size() does not match the number " <<
                    "of nodes in the tree (tree.size() = " << t.size() << ", but " <<
                    "actual number of nodes is " << count_nodes(t.rt) << ")\n"
                );

            if(has_size_fn)
                CHECK(t.size() != expected_size,
                    "After inserting " << x << ", tree.size() is incorrect (" <<
                    "expected " << expected_size << ", but got " << t.size() << ")\n"
                );

            if(has_empty_fn)
                CHECK(t.empty(),
                    "After inserting " << x << ", tree.empty() is still true\n"
                );

            if(has_find_fn)
                CHECK(t.find(x) == nullptr,
                    "After inserting " << x << ", tree.find(x) still returns nullptr\n"
                );
        }
        OK();

        // Now we insert duplicates of all the existing values; these should not
        // change the tree at all.
        int expected_height = safe_height(t);

        std::cout << "Testing insert by inserting duplicates of all existing keys...";

        for(int x : data) {
            t.insert(x);

            CHECK(not check_tree(t),
                "Tree is not valid after inserting a duplicate " << x << "\n" <<
                t << '\n'
            );

            int h = safe_height(t);
            CHECK(h != expected_height,
                "After inserting a duplicate of an existing value, the actual height of " <<
                "the tree changed (expected height = " << expected_height << ", " <<
                "actual height = " << h << ")\n"
            );

            if(has_height_fn)
                CHECK(t.height() != expected_height,
                    "After inserting a duplicate of an existing value, tree.height() changed " <<
                    "(inserting duplicates should not change the tree at all). " <<
                    "Expected height: " << expected_height << " but tree.height() = " << 
                    t.height() << ".\n"
                );

            if(has_size_fn)
                CHECK(t.size() != expected_size,
                    "After inserting a duplicate of an existing value, size changed " <<
                    "(inserting a duplicate should not change the tree at all). Expected " <<
                    "size = " << expected_size << ", but tree.size() = " << t.size() << ".\n"
                );
        }

        OK();  
        insert_full = true;      
        return true;
    }

    bool test_find()
    {
        if(not has_find_fn) {
            std::cout << "find() not implemented, skipping." << std::endl;
            return false;
        }

        if(not has_insert_fn) {
            std::cout << "Cannot test find() without insert(), skipping." << std::endl;
            return false;
        }        

        std::vector<int> data = make_random_vector(20);
        for(int& x : data)
            x = (x % 109) - 9; // This gives values in the range of -9 to 99.

        // Build random tree
        avl_tree t;
        for(int x : data)
            t.insert(x);

        // Test finding the values we just inserted.
        std::cout << "Testing find() by insert()-ing random values, then finding them...";
        for(int x : data) {
            avl_tree::node* j = t.find(x);

            CHECK(j == nullptr,
                "After inserting " << x << ", tree.find(" << x << ") returned nullptr.\n"
            );

            if(j != nullptr)
                CHECK(j->key != x,
                    "Find(" << x << ") returned a pointer to a node that doesn't contain " <<
                    x << " (actual node->key = " << j->key << ")\n"
                );
        }
        OK();

        // Then, test finding values that are NOT there.
        std::cout << "Testing find() by insert()-ing random values, then finding values NOT in the tree...";
        for(int i = 0; i < 100; ++i) {
            // First, find a random value not already in the tree
            int x = 0;
            do {
                x = rand();
                if(std::find(data.begin(), data.end(), x) == data.end())
                    break;
            } while(true);

            avl_tree::node* n = t.find(x);

            CHECK(n != nullptr,
                "Find(" << x << "), which DOES NOT exist in the tree, returned "
                "a non-nullptr value (returned a pointer to " << n->key << ")\n"
            );
        }
        OK();

        return true;
    }

    bool test_size()
    {
        if(not has_size_fn) {
            std::cout << "size() not implemented, skipping." << std::endl;
            return false;
        }

        if(not has_insert_fn) {
            std::cout << "Cannot test size() without insert(), skipping." << std::endl;
            return false;
        }

        std::vector<int> data = make_random_vector(20);
        for(int& x : data)
            x = (x % 109) - 9; // This gives values in the range of -9 to 99.

        avl_tree t;
        int expected_size = 0;

        CHECK(t.size() != 0,
            "Size of the empty tree is not 0 (tree.size() = " << t.size() << ")\n"
        );

        std::cout << "Testing size() by inserting random values...";
        for(int x : data) {
            t.insert(x);
            ++expected_size;

            CHECK(t.size() != expected_size,
                "After inserting " << expected_size << " elements, tree.size() is " <<
                "incorrect (expected: " << expected_size << ", but tree.size() = " <<
                t.size() << ")\n"
            );
        }
        OK();

        // Insert some duplicates, ensure that size does not change.
        std::cout << "Testing size() by inserting duplicates (size should not change)...";
        for(int x : data) {
            t.insert(x);

            CHECK(t.size() != expected_size,
                "After inserting a duplicate of an existing value, size changed " <<
                "(inserting a duplicate should not change the tree at all). Expected " <<
                "size = " << expected_size << ", but tree.size() = " << t.size() << ".\n"
            );
        }
        OK();

        return true;
    }

    bool test_height()
    {
        if(not has_height_fn) {
            std::cout << "height() not implemented, skipping." << std::endl;
            return false;
        } 

        if(not has_insert_fn) {
            std::cout << "Cannot test height() without insert(), skipping." << std::endl;
            return false;
        }

        // Tests for height in other places ensure that the height returned
        // matches the actual height of the tree, and that it doesn't change
        // when inserting duplicates. So here we focus on ensuring that the 
        // height is what we expect it to be, after inserting *specific* sequences
        // (i.e., I've traced through these inserts and looked at the heights
        // of the tree after each set).

        // So this is really more a test of insert() than of height().

        { 
            // This triggers outside imbalances
            std::vector<int> data = { 1, 2, 3, 4, 5, 6, 7, 8 };
            std::vector<int> hght = { 1, 2, 2, 3, 3, 3, 3, 4 }; // Expected heights

            std::cout << "Checking height() by inserting 1, 2, 3, ..., 7, 8 (outside imbalances): ";

            avl_tree t;

            for(size_t i = 0; i < data.size(); ++i) {
                t.insert(data.at(i));

                int expected_height = hght.at(i);

                CHECK(t.height() != expected_height,
                    "After inserting " << data.at(i) << ", tree.height() is " <<
                    "wrong (expected height = " << expected_height << ", but " <<
                    "tree.height() = " << t.height() << ")\n"

                );
            }

            OK();
        }

        { 
            // This triggers outside imbalances
            std::vector<int> data = { 8, 7, 6, 5, 4, 3, 2, 1 };
            std::vector<int> hght = { 1, 2, 2, 3, 3, 3, 3, 4 }; // Expected heights

            std::cout << "Checking height() by inserting 8, 7, 6, ..., 2, 1 (outside imbalances): ";

            avl_tree t;

            for(size_t i = 0; i < data.size(); ++i) {
                t.insert(data.at(i));

                int expected_height = hght.at(i);

                CHECK(t.height() != expected_height,
                    "After inserting " << data.at(i) << ", tree.height() is " <<
                    "wrong (expected height = " << expected_height << ", but " <<
                    "tree.height() = " << t.height() << ")\n"

                );
            }

            OK();
        }        

        {
            std::vector<int> data = { 10, 5, 15, 1, 7, 11, 20 };
            std::vector<int> hght = { 1,  2,  2, 3, 3, 3,  3  };

            std::cout << "Checking height() by inserting 10, 5, 15, 1, 7, 11, 20 (no imbalances): ";

            avl_tree t;

            for(size_t i = 0; i < data.size(); ++i) {
                t.insert(data.at(i));
                int expected_height = hght.at(i);
                CHECK(t.height() != expected_height,
                    "After inserting " << data.at(i) << ", tree.height() is " <<
                    "wrong (expected height = " << expected_height << ", but " <<
                    "tree.height() = " << t.height() << ")\n"

                );
            }

            OK();
        }

        {
            std::vector<int> data = { 7, 10, 11, 20, 15, 2, 5 };
            std::vector<int> hght = { 1,  2,  2,  3,  3, 3, 3 };

            std::cout << "Checking height() by inserting 7, 10, 11, 20, 15, 2, 5 (inside imbalances): ";

            avl_tree t;

            for(size_t i = 0; i < data.size(); ++i) {
                t.insert(data.at(i));
                int expected_height = hght.at(i);
                CHECK(t.height() != expected_height,
                    "After inserting " << data.at(i) << ", tree.height() is " <<
                    "wrong (expected height = " << expected_height << ", but " <<
                    "tree.height() = " << t.height() << ")\n"

                );
            }

            OK();
        }

        return true;   
    }

    bool test_empty()
    {
        if(not has_empty_fn) {
            std::cout << "empty() not implemented, skipping." << std::endl;
            return false;
        } 

        if(not has_insert_fn) {
            std::cout << "Cannot test empty() without insert(), skipping." << std::endl;
            return false;
        }

        std::cout << "Testing empty()...";

        avl_tree t;
        CHECK(not t.empty(),
            "The empty (default-constructed) tree.empty() did not return true."
        );

        t.insert(12);
        CHECK(t.empty(),
            "tree.empty() returned true after inserting 12."
        );

        OK();
        return true;   
    }

    bool test_copy()
    {
        if(not has_copy_fn) {
            std::cout << "copy() not implemented, skipping." << std::endl;
            return false;
        } 

        if(not has_insert_fn) {
            std::cout << "Cannot test empty() without insert(), skipping." << std::endl;
            return false;
        }

        std::cout << "Testing copy()...";

        avl_tree original;
        std::vector<int> data = make_random_vector(20);
        for(int& x : data) {
            x = (x % 109) - 9; // This gives values in the range of -9 to 99.
            original.insert(x);
        }

        avl_tree copy = original;
        
        CHECK(not check_tree(copy),
            "After making a copy, the copy is not a valid tree. Copy:\n" <<
            copy << "\n"
        );

        CHECK(not trees_equal(original, copy),
            "After copying a tree, the copy is different from the original:\n" <<
            "Original tree: \n" << original << "\nCopy: \n" << copy << "\n"
        );

        OK();
        return true;   
    }

    bool test_merge_with()
    {
        if(not has_merge_with_fn) {
            std::cout << "merge_with() (extra credit) not implemented, skipping." << std::endl;
            return false;
        } 

        if(not has_insert_fn) {
            std::cout << "Cannot test merge_with() without insert(), skipping." << std::endl;
            return false;
        }

        std::cout << "Testing merge_with()...";

        // Make two random trees

        avl_tree tree_a;
        std::vector<int> data_a = make_random_vector(20, 1726532);
        for(int& x : data_a) {
            // x = (x % 109) - 9; // This gives values in the range of -9 to 99.
            tree_a.insert(x);
        }

        // NOTE: There may be duplicates between the two trees; merge with must
        // handle these correctly. This also means that the size of the merged
        // tree may be < a.size() + b.size().

        avl_tree tree_b;
        std::vector<int> data_b = make_random_vector(20, 5672351);
        for(int& x : data_b) {
            // x = (x % 109) - 9; // This gives values in the range of -9 to 99.
            tree_b.insert(x);
        }

        int b_size = count_nodes(tree_b.rt);
        int b_height = safe_height(tree_b);

        // Merge b into a
        tree_a.merge_with(tree_b);

        std::vector<int> merged = data_a;
        for(int x : data_b) {
            if(std::find(data_a.begin(), data_a.end(), x) == data_a.end())
                merged.push_back(x);
        }

        // Make sure that a is still a valid tree.
        CHECK(not check_tree(tree_a),
            "After a.merge_with(b), a is no longer a valid tree. Tree a: \n" <<
            tree_a << "\n"
        );

        // Check to make sure that tree_b was not modified.
        CHECK(count_nodes(tree_b.rt) != b_size,
            "After a.merge_with(b), the size of b changed (original size = " <<
            b_size << ", new size = " << count_nodes(tree_b.rt) << ")\n"
        );

        CHECK(safe_height(tree_b) != b_height,
            "After a.merge_with(b), the height of b changed (original height = " <<
            b_height << ", new height = " << safe_height(tree_b) << ")\n"
        );

        // Make sure that a now contains all the values from a and b
        for(int x : data_a) {
            CHECK(safe_find(tree_a.rt, x) == nullptr,
                "After a.merge_with(b), value " << x << ", originally in `a`, does " <<
                "not exist. Tree a = \n" << tree_a << "\n"
            );
        }

        // A student reported that merge_with passes, even with an empty 
        // implementation?

        CHECK(size_t(tree_a.size()) != merged.size(),
            "Size of merged tree is not correct; expected " << merged.size() 
            << " but the size of the tree is " << tree_a.size());

        for(int x : data_b) {
            CHECK(safe_find(tree_a.rt, x) == nullptr,
                "After a.merge_with(b), value " << x << ", originally in `b`, does " <<
                "not exist. Tree a = \n" << tree_a << "\n"
            );
        } 

        // TODO: make sure node*s aren't being shared. (valgrind will detect this
        // or the test will crash from a double-free).       

        OK();
        return true;   
    }


    // -----------------------------------------------------------------------


    /* build_tree(...)
       Build an AVL tree from a level-order-ish representation of its values.
       This uses a nested vector to describe the tree structure. E.g., 

                        { 10 }
                    { 5,       20 }
                  { 1, NIL,  15, NIL }
                           ⋮

       This constructs the tree

                            10
                           /  \
                          5    20
                         /    /
                        1    15      

       Each "layer" must have twice the size of the one above it. 0 values mean
       do not insert a node here. (I.e., the resulting tree cannot contain any
       nodes with key = 0.) The resulting tree will have the correct heights,
       but no checks are done to make sure the values given follow the search
       order property, and the tree will not be rebalanced during construction.
    */
    static const int NIL = 0;

    static avl_tree build_tree(
        const std::vector< std::vector<int> >& data
    )
    {
        std::vector< avl_tree::node* > prev_level_nodes;
        std::vector< avl_tree::node* > level_nodes;
        avl_tree result;

        if(data.empty())
            return result;

        assert(data[0].size() == 1);

        // Process level 0
        result.rt = new avl_tree::node{ 
            data[0][0], 
            nullptr, 
            nullptr, 
            nullptr, 
            int(data.size()) 
        };
        prev_level_nodes.push_back(result.rt);

        // Process remaining levels
        for(size_t i = 1; i < data.size(); ++i) {
            // Must be 2x the size of the above level
            assert(data[i].size() == 2*data[i-1].size());

            for(size_t j = 0; j < data[i].size(); ++j) {
                int key = data[i][j];

                if(key == NIL) {
                    level_nodes.push_back(nullptr);
                    continue; // No node here
                }

                // Parent of the new node
                avl_tree::node* p = prev_level_nodes[j/2];

                // Cannot create children of a NIL node.
                assert(p != nullptr);

                avl_tree::node* n = new avl_tree::node{ 
                    key, 
                    nullptr, 
                    nullptr, 
                    p,
                    1
                };

                // Attach to parent
                (j % 2 == 0 ? p->left : p->right) = n;
                level_nodes.push_back(n);
            }

            prev_level_nodes = level_nodes;
            level_nodes.clear();
        }        

        // Now we just need to fix the heights
        fix_heights(result.rt);

        return result;
    }

    /* fix_heights(root)
       Fix all the heights in the tree rooted at root.
    */
    static void fix_heights(avl_tree::node* root)
    {
        if(root == nullptr)
            return;

        fix_heights(root->left);
        fix_heights(root->right);

        root->height = 1 + std::max({
            root->left == nullptr  ? 0 : root->left->height,
            root->right == nullptr ? 0 : root->right->height
        });
    }

    /* rotated(t, child)
       Rotate `child` and return the resulting tree. The original tree is not
       modified.
    */
    avl_tree rotated(const avl_tree& t, int child)
    {
        // Make a copy, find child in the copy
        avl_tree copy = safe_copy(t);
        avl_tree::node* n = safe_find(copy, child);
        assert(n != nullptr);

        copy.rotate(n);
        return copy;
    }

    /* safe_copy(t)
       Make a copy of the given tree, without using the copy-constructor or
       copy-assignment operator.

       NOTE: If the tree has any data members other than rt, this will fail. 
       There's a note to this effect in the avl_tree.hpp header file.
    */
    avl_tree safe_copy(const avl_tree& original)
    {
        avl_tree t;
        t.rt = safe_copy(original.rt);
        return t;
    }

    avl_tree::node* safe_copy(avl_tree::node* n, avl_tree::node* parent = nullptr)
    {
        if(n == nullptr)
            return nullptr;
        else {
            avl_tree::node* result = new avl_tree::node;
            result->key = n->key;
            result->height = n->height;
            result->parent = parent;

            result->left = safe_copy(n->left, result);
            result->right = safe_copy(n->right, result);

            return result;
        }
    }

    /* count_nodes(n)
       Returns the number of nodes in the subtree rooted at n.
    */
    static int count_nodes(avl_tree::node* root) {
        if(root == nullptr)
            return 0;
        else
            return 1 + count_nodes(root->left) + count_nodes(root->right);
    }

    /* tree_height(root)
       Returns the height of the tree rooted at `root`. This calculates the 
       height, rather than using the height stored in the nodes, but also checks
       for cycles in the process
    */
    static int safe_height(const avl_tree& t)
    {
        return safe_height(t.rt);
    }

    static int safe_height(avl_tree::node* root)
    {
        std::set<avl_tree::node*> seen_nodes;
        return safe_height(root, seen_nodes);
    }

    static int safe_height(avl_tree::node* root, std::set<avl_tree::node*>& seen_nodes) 
    {
        if(root == nullptr)
            return 0;
        else if(seen_nodes.count(root) != 0) {
            // Cycle!
            return std::numeric_limits<int>::min();
        }
        else {
            seen_nodes.insert(root);

            return 1 + std::max(
                safe_height(root->left, seen_nodes), 
                safe_height(root->right, seen_nodes)
            );
        }
    }

    /* safe_empty(t)
       Returns true if `t` is empty, by checking the tree directly.
    */
    static bool safe_empty(avl_tree& t)
    {
        return t.rt == nullptr;
    }

    /* trees_equal(a,b)
       Returns true if the trees rooted at a and b are equal, by comparing their
       keys.
    */
    static bool trees_equal(const avl_tree::node* a, const avl_tree::node* b)
    {
        if(a == nullptr and b == nullptr)
            return true;
        else if(a == nullptr or b == nullptr)
            return false; // One empty, the other not
        else if(a->key == b->key) {
            return trees_equal(a->left, b->left) and
                   trees_equal(a->right, b->right);
        }
        else
            return false;
    }

    static bool trees_equal(const avl_tree& a, const avl_tree& b)
    {
        return trees_equal(a.rt, b.rt);
    }

    /* find_true_root(n,nodes)
       Tries to find the true root of the tree, starting at node n.

       If the tree contains a cycle along the path to the root, then n itself will
       be returned, otherwise, the root of the tree (the node along the ->parent
       path with ->parent == nullptr) will be returned.
    */
    static avl_tree::node* find_true_root(
        avl_tree::node* n, 
        std::set<avl_tree::node*>& nodes
    )
    {
        if(nodes.count(n) != 0)
            return n; // Cycle!
        else if(n->parent == nullptr)
            return n;
        else {
            nodes.insert(n);
            return find_true_root(n->parent, nodes);
        }
    }

    /* find_true_root(t)
       Tries to find the true root of the tree t. If there is a cycle along the path
       to the root, n itself will be returned.
    */
    static avl_tree::node* find_true_root(avl_tree::node* n)
    {
        std::set<avl_tree::node*> nodes;
        return find_true_root(n, nodes);
    }

    /* safe_find(t, target)
       Find target in the tree t, even if t is not a valid BST.
    */
    avl_tree::node* safe_find(const avl_tree& t, int target)
    {
        return safe_find(t.rt, target);
    }

    avl_tree::node* safe_find(avl_tree::node* n, int target)
    {
        if(n == nullptr)
            return nullptr;
        else if(n->key == target)
            return n;

        avl_tree::node* l = safe_find(n->left, target);
        avl_tree::node* r = safe_find(n->right, target);

        return l != nullptr ? l : r ;
    }

    /* get_root(t)
       Get a pointer to the root of the given tree. This is used to allow non-
       friend code to manually traverse trees.
    */
    static avl_tree::node* get_root(const avl_tree& t)
    {
        return t.rt;
    }

    /* Tree printing
       -------------

       This uses a level-order traversal to print the tree. Nodes are printed in
       the form

                                       -.
             VALUE                    VALUE
           [p=PARENT]               [p=PARENT]
            .--^--.                  <Cycle>

       The note <Cycle> is added to any node that has already been printed 
       previously.

       We save a copy of each node in the traversal structure.

       TODO: Finish this function!
    */

    static void simple_print(const avl_tree& t)
    {
        std::queue<avl_tree::node*> q;
        int level_elems = 1; // Number of nodes in the current level.
        int current_level = 0;

        // this vector is used to draw the connections between
        // the levels. Each element corresponds to a node in the
        // above level, as one of the following values:
        //   0 = no children, 1 = right child, 2 = left child, 3 = both
        // -1 is used for null nodes
        std::vector<int> connectors;

        // We assume a terminal width of 132 cols.
        int level_spacing = 132/2;
        const int node_width = 6;

        q.push(t.rt);
        int i = 0;
        while(not q.empty()) {
            avl_tree::node* n = q.front();
            q.pop();

            int padding = level_spacing/2 - node_width/2;
            if(padding < 1)
                padding = 1;
            std::cout << std::string(padding, ' ');
            if(n == nullptr) {
                std::cout << "      ";
                /*
                std::cout << ANSI_W_BKG();
                std::cout << "  .  " << " ";
                std::cout << ANSI_NORMAL();
                */

                // In order to keep the spacing of the rows consistent, we need
                // to pad the next level with the appropriate number of null 
                // nodes to cover any "holes" in the tree. However,
                // this causes problems when we reach the bottom of the
                // tree.
                q.push(nullptr);
                q.push(nullptr);

                connectors.push_back(-1);
            }
            else {
                std::cout << ANSI_W_BKG();
                std::cout.width(5);
                std::cout << n->key << " ";
                std::cout << ANSI_NORMAL();

                q.push(n->left);
                q.push(n->right);

                connectors.push_back(
                    (n->right != nullptr) + 
                    2 * (n->left != nullptr)
                );
            }            

            // Have we reached the end of this level?
            ++i;
            if(i == level_elems) {
                // End of this level
                std::cout << std::endl;

                // Print the connectors between the previous level and the next
                for(int c : connectors) {

                    std::string symbol = " ";
                    switch(c) {
                        case -1: symbol = " "; break;
                        case 0:  symbol = " "; break;
                        case 1:  symbol = "└"; break;
                        case 2:  symbol = "┘"; break;
                        case 3:  symbol = "┴"; break;
                    }

                    std::cout << std::string(padding, ' ');
                    std::cout << "  " << symbol << "  " << " ";
                    std::cout << std::string(padding, ' ');
                }
                std::cout << std::endl;

                // Start next level
                connectors.clear();
                i = 0;
                level_elems *= 2;
                level_spacing /= 2;
                ++current_level;
            }
            else 
                std::cout << std::string(padding, ' ');

            // Quit after the bottom level
            if(current_level >= t.height())
                break;
        }
        std::cout << std::endl;
    }

    /* check_values(root, f)
       Helper function for printing trees, checks the *keys* in the tree for
       BST-ness (search order property) and calls `f(n)` on any node with an
       incorrect key.
    */
    template<typename Fn>
    static void check_keys(
        avl_tree::node* n, 
        Fn&& f,
        std::set<avl_tree::node*>& seen_nodes,
        int low = std::numeric_limits<int>::min(),
        int high = std::numeric_limits<int>::max()
    )
    {
        // Always ignore nullptr nodes
        if(n == nullptr)
            return; 

        // Ignore cycle nodes
        if(seen_nodes.count(n) > 0)
            return;

        seen_nodes.insert(n);

        if(n->key <= low or n->key >= high)
            f(n);

        check_keys(
            n->left,
            std::forward<Fn>(f),
            seen_nodes,
            low, n->key
        );

        check_keys(
            n->right,
            std::forward<Fn>(f),
            seen_nodes,
            n->key, high
        );
    }

    template<typename Fn>
    static void check_keys(
        const avl_tree& t,
        Fn&& f
    )
    {
        std::set<avl_tree::node*> seen_nodes;
        check_keys(
            t.rt,
            std::forward<Fn>(f),
            seen_nodes
        );
    }

    
    /* Problem structs
       ---------------

       These structs are embedded into a std::variant and used to record
       "problems" found while printing a tree. Once tree printing is
       complete, we print a list of problems, if any.
    */
    struct prob_wrong_height
    {
        avl_tree::node* n;
        int found_height, correct_height;
    };

    struct prob_wrong_key
    {
        avl_tree::node* n;
    };

    struct prob_wrong_left
    {
        avl_tree::node* n;
    };

    struct prob_wrong_right
    {
        avl_tree::node* n;
    };

    struct prob_wrong_parent
    {
        avl_tree::node* n;
        avl_tree::node* correct_parent;
    };

    struct prob_avl_imbalance
    {
        avl_tree::node* n; // The node at which the imbalance was detected.
        int imbalance;     // The (signed) imbalance at this node
    };

    using problem = variant<
        prob_wrong_height,
        prob_wrong_key,
        prob_wrong_left,
        prob_wrong_right,
        prob_wrong_parent,
        prob_avl_imbalance
    >;



    /* print_levelorder(t)
       Prints a tree. This goes to a lot of work to print a tree "nicely", in
       a vertical format. It also labels nodes with incorrect heights, is safe
       to call on trees that may contain cycles (and will label left/right 
       pointers that would result in a cycle), and can label incorrect parent
       pointers.

       This essentially works by laying out the subtrees of each node, 
       recursively. For a single node n there are four possible layouts, 
       depending on children. The Y-coordinate of any node can be determined 
       automatically from its level; only the X-coordinates need to be computed.
            
                                     No children

                                 (x,y)
                                   ┌──────•──────┐
                                   │      n      │   Height = 1
                                   └─────────────┘
                                     Width = nw

                                     Left child:

                                 (n.x,n.y)
                             ┄┄┄┄┄┄┌──────•──────┐
                            ┆      │      n      │    1  
                            ┆      └──────┬──────┘ 
                            ┆        ┌────┘      ┆    1
                            ┌┈┈┈┈┈┈┈┈┴┈┈┈┈┈┈┈┈┐  ┆ 
                            ┊  Width: l.w     ┊  ┆
                            ┊  Height: l.h    ┊  ┆
                            ┊                 ┊  ┆
                            ┊                 ┊  ┆
                            ┊                 ┊  ┆
                            └┈┈┈┈┈┈┈┈┄┄┄┄┄┄┄┄┄┘┄┄
                                               pad

                            Total width: l.w + pad
                            Total height: 2 + l.h

                                Right child:

                                 (n.x,n.y)
                                   ┌──────•──────┐┄┄┄┄┄┄
                                   │      n      │      ┆ 
                                   └──────┬──────┘      ┆
                                   ┆      └────┐        ┆
                                   ┆  ┌┈┈┈┈┈┈┈┈┴┈┈┈┈┈┈┈┈┐
                                   ┆  ┊  Width: r.w     ┊
                                   ┆  ┊  Height: r.h    ┊
                                   ┆  ┊                 ┊
                                   ┆  ┊                 ┊
                                   ┆  ┊                 ┊
                                    ┄┄└┈┈┈┈┈┈┈┈┄┄┄┄┄┄┄┄┄┘

                                     Both children:

                                 (n.x,n.y)
                       ┄┄┄┄┄┄┄┄┄┄┄┄┌──────•──────┐┄┄┄┄┄┄┄┄┄┄┄┄
                      ┆            │      n      │            ┆
                      ┆            └──────┬──────┘            ┆
                      ┆        ┌──────────┴──────────┐        ┆
                      ┌┈┈┈┈┈┈┈┈┴┈┈┈┈┈┈┈┈┐   ┌┈┈┈┈┈┈┈┈┴┈┈┈┈┈┈┈┈┐
                      ┊  Width: l.w     ┊   ┊  Width: r.w     ┊
                      ┊  Height: l.h    ┊   ┊  Height: r.h    ┊
                      ┊                 ┊   ┊                 ┊
                      ┊                 ┊   ┊                 ┊
                      ┊                 ┊   ┊                 ┊
                      └┈┈┈┈┈┈┈┈┄┄┄┄┄┄┄┄┄┘┄┄┄└┈┈┈┈┈┈┈┈┄┄┄┄┄┄┄┄┄┘ 

       TODO: Fix alignment of nodes with children.

       OK, so the reason why left-trees have extra padding is because the 
       width includes the width of the child, but we assume that the width
       *starts* at the X of root. So we push the width to the right. We'll need
       to store the "alignment" (X-offset within width) of each node, and use
       it when drawing them. It's fine for now, though.

    */
    static void print_levelorder(
        const avl_tree& t,
        bool show_heights = false
    )
    {
        // Special case for printing the empty tree.
        if(t.rt == nullptr) {
            std::cout << text_grid{"<empty>"} << std::endl;
            return;
        }

        // The width of a single node by itself
        const int node_width = 7;
        const int pad = 3; // Padding to the left/right of single-child nodes
        const int gap = 3; // Gap between children of 2-child nodes.

        struct render_node
        {   
            render_node()
            {
                n = nullptr;
            }

            render_node(avl_tree::node* n) : n(n) { }

            avl_tree::node* n;            // The node we are printing
            int w = 100, h = -1;          // Size of this node and its children
            int x = -1, y = -1;           // Computed position

            // Flags for things that can be wrong
            bool wrong_left = false,
                 wrong_right = false;

            bool wrong_parent = false;

            bool wrong_height = false;
            int true_height = -1;
            bool wrong_value = false;

            bool imbalanced = false;
        };

        // List of all the problems we found in the tree.
        std::vector<problem> problems;

        // This map collects all the nodes, and all the render_nodes, mapping 
        // between them so that we can update a render_node from its node 
        // easily
        std::map<avl_tree::node*, render_node> rns;

        // First, we populate the list of nodes. At this point, we don't know
        // anything about the sizes or positions of the nodes
        auto add_to_rns = 
            [&rns,&problems](avl_tree::node* n, avl_tree::node* p, bool wl, bool wr, int level) -> void
            {
                render_node rn{n};

                // Find the correct parent for this node
                if(n->parent != p) {
                    rn.wrong_parent = false;
                    problems.push_back(
                        prob_wrong_parent{n, p}
                    );
                }

                // Find the correct height for this node
                int correct_height = safe_height(n);
                if(n->height != correct_height) {
                    rn.wrong_height = true;                    
                    problems.push_back(
                        prob_wrong_height{n, n->height, correct_height}
                    );
                }
                rn.true_height = correct_height;

                rn.wrong_left = wl;
                rn.wrong_right = wr;

                if(wl)
                    problems.push_back(prob_wrong_left{n});
                if(wr)
                    problems.push_back(prob_wrong_right{n});

                rn.h = 1; // Node's height by itself.

                // Y is determined from level and does not change.
                rn.y = level * 2;

                rns.insert({n, rn});
            };

        traverse<traversal_order::PREORDER>(t,add_to_rns);

        // Mark nodes with non-BST keys
        // this does not affect rendering and could be done anywhere.
        check_keys(
            t, 
            [&rns,&problems](avl_tree::node* n) -> void
            {
                rns[n].wrong_value = true;
                problems.push_back(prob_wrong_key{n});
            }
        );

        // Mark imbalanced nodes
        // this does not affect rendering and could be done anywhere.
        traverse<traversal_order::POSTORDER>(
            t,
            [&rns,&problems](avl_tree::node* n, avl_tree::node*, bool, bool, int) -> void
            {   
                // An imbalance lower in the tree will tend to generate 
                // pseudo-imbalances above it, along the path to the root. To 
                // try to only capture the "lowest" imbalance, we do this traversal
                // as postorder (bottom-up), and we check if either of our 
                // children are marked as imbalanced. If they are, we do not 
                // mark this node as imbalanced.

                bool has_child_imbalance = false;
                if(n->left != nullptr and rns[n->left].imbalanced)
                    has_child_imbalance = true;
                if(n->right != nullptr and rns[n->right].imbalanced)
                    has_child_imbalance = true;

                int left_height = n->left == nullptr ? 0 : rns[n->left].true_height;
                int right_height = n->right == nullptr ? 0 : rns[n->right].true_height;

                if(std::abs(left_height - right_height) >= 2) {
                    // Note that if we have a child which is imbalanced, we
                    // avoid adding n to the list of problems, but we still
                    // mark n as imbalanced. This prevents the issue where, 
                    // along the path from the imbalanced node to the root, 
                    // every *other* level is marked as being a problem.

                    if(not has_child_imbalance)
                        problems.push_back(
                            prob_avl_imbalance{ n, left_height - right_height} 
                        );                     

                    rns[n].imbalanced = true;
                }
            }
        );

        // Now we want to get the sizes of the render_nodes. This needs to be
        // a bottom-up (postorder) traversal
        auto compute_size = 
            [&rns](avl_tree::node* n, avl_tree::node*, bool, bool, int) -> void
            {
                render_node& rn = rns[n];

                bool has_left = n->left != nullptr and not rn.wrong_left;
                bool has_right = n->right != nullptr and not rn.wrong_right;

                if(not has_left and not has_right)
                    rn.w = node_width; // Leaf node
                else if(not has_left or not has_right) {
                    // One child

                    avl_tree::node* maybe_left = rn.wrong_left ? nullptr : n->left;
                    avl_tree::node* maybe_right = rn.wrong_right ? nullptr : n->right ;
                    avl_tree::node* child = maybe_left == nullptr ? maybe_right : maybe_left;

                    // Our width is the width of our child, plus the padding.
                    rn.w = rns[child].w + pad;

                    // Height is 1 + the row between (1) + child height
                    rn.h = 2 + rns[child].h;
                }
                else {
                    // Two children; sum their widths and include the 
                    // padding between.
                    // Here it should be safe to access this children directly.

                    rn.w = rns[n->left].w + rns[n->right].w + gap;

                    rn.h = 2 + std::max(rns[n->left].h, rns[n->right].h);
                }
            };

        traverse<traversal_order::POSTORDER>(t, compute_size);

        // Once the sizes have been computed, we can compute positions.

        // Y-position has to be computed top-down, i.e., preorder.
        traverse<traversal_order::PREORDER>(t, 
            [&rns](avl_tree::node* n, avl_tree::node* p, bool, bool, int) -> void
            {
                render_node& rn = rns[n];

                if(p == nullptr) 
                    rn.y = 0; // Root
                else 
                    rn.y = rns[p].y + 2;
            }
        );

        rns[t.rt].x = 0;

        // We do a preorder so that we can push X positions down into children
        // before they are traversed.
        traverse<traversal_order::PREORDER>(
            t,
            [&rns, node_width, gap, pad](avl_tree::node* n, avl_tree::node*, bool, bool, int) -> void
            {
                render_node& rn = rns[n];
                bool has_left = n->left != nullptr and not rn.wrong_left;
                bool has_right = n->right != nullptr and not rn.wrong_right;

                if(not has_left and not has_right) {
                    // No children: no nodes to push X into                    
                }
                else if(has_left and not has_right) {
                    // One left child.
                    rns[n->left].x = rns[n].x - pad;
                }
                else if(not has_left and has_right) {
                    // One right child, push X into it.
                    rns[n->right].x = rns[n].x + pad;
                }
                else {
                    // Two children.
                    rns[n->left].x = rns[n].x - (rns[n->left].w + rns[n->right].w + gap - node_width) / 2;
                    rns[n->right].x = rns[n].x + (rns[n->left].w + rns[n->right].w + gap - node_width) / 2;
                }
            }
        );

        // We also need to figure out the max size of the arrangement.
        int min_x = 1000000000, max_x = -1000000000;
        int min_y = 1000000000, max_y = -1000000000;

        traverse(
            t,
            [&min_x,&min_y,&max_x,&max_y,&rns]
            (avl_tree::node* n, avl_tree::node*, bool, bool, int) -> void
            {
                int x0 = rns[n].x, y0 = rns[n].y;
                int x1 = x0 + rns[n].w - 1, 
                    y1 = y0 + rns[n].h - 1;

                if(x0 < min_x)
                    min_x = x0;
                if(y0 < min_y)
                    min_y = y0;
                if(x1 > max_x)
                    max_x = x1;
                if(y1 > max_y)
                    max_y = y1;
            }
        );

        int w = max_x - min_x + 1, h = max_y - min_y + 1;

        text_grid grid(w, h);

        // Finally, everything has an X and a Y so we can draw all of it.
        for(const auto& p : rns) {
            const render_node& rn = p.second;

            // Nodes are drawn centered, so we have to offset their x so that
            // they don't draw into negative coords.
            int x = rn.x - min_x + 3, 
                y = rn.y - min_y;
            grid.print(
                x,
                y,
                rn.n,
                rn.wrong_height or show_heights,    // Show height
                rn.wrong_value,                     // Mark value
                rn.wrong_height                     // Mark height
            );

            // Draw connectors
            bool has_left = not rn.wrong_left and rn.n->left != nullptr, 
                 has_right = not rn.wrong_right and rn.n->right != nullptr ;

            if(has_left) 
                grid.hconnect(x, y, rns[rn.n->left].x - min_x + 3, rns[rn.n->left].y - min_y);
            if(has_right)
                grid.hconnect(x, y, rns[rn.n->right].x - min_x + 3, rns[rn.n->right].y - min_y);
            if(has_left and has_right)
                grid.set(x, y + 1, "┴");

            // TODO: Indicate wrong children

            // TODO: Indicate wrong parent
        }

        // As a hack for nodes which have a width that includes extra space to
        // the right, we crop any unused space.
        grid.right_crop();

        // And then print the result!
        std::cout << grid << std::endl;

        // Print any problems we found
        if(not problems.empty()) {
            std::cout << ANSI_RED() << "PROBLEMS:" << ANSI_NORMAL() << std::endl;

            for(const problem& p : problems) {
                if(holds_alt<prob_wrong_height>(p)) {
                    const prob_wrong_height& wh = getv<prob_wrong_height>(p);
                    std::cout << " * Node " << wh.n->key << " has the wrong height "
                              << "(correct height = " << wh.correct_height 
                              << ", height in node = " << wh.found_height << ")"
                              << std::endl; 
                }
                else if(holds_alt<prob_wrong_key>(p)) {
                    const prob_wrong_key& wk = getv<prob_wrong_key>(p);
                    std::cout << " * Node " << wk.n->key << "'s key violates the "
                              << "search order property (wrong left/right position)"
                              << std::endl;
                }
                else if(holds_alt<prob_wrong_left>(p)) {
                    const prob_wrong_left& wl = getv<prob_wrong_left>(p);
                    std::cout << " * Node " << wl.n->key << " has incorrect left child "
                              << "(left child is " << wl.n->left->key << " but that node "
                              << " is a child of " << wl.n->left->parent->key << ")"
                              << std::endl;
                }
                else if(holds_alt<prob_wrong_right>(p)) {
                    const prob_wrong_right& wl = getv<prob_wrong_right>(p);
                    std::cout << " * Node " << wl.n->key << " has incorrect right child "
                              << "(right child is " << wl.n->left->key << " but that node "
                              << " is a child of " << wl.n->left->parent->key << ")"
                              << std::endl;
                }
                else if(holds_alt<prob_wrong_parent>(p)) {
                    const prob_wrong_parent& wp = getv<prob_wrong_parent>(p);

                    std::cout << " * Node " << wp.n->key << " has the wrong parent "
                              << "pointer (n->parent = " << wp.n->parent->key 
                              << " but it is a child of " << wp.correct_parent->key 
                              << ")" << std::endl;
                }
                else if(holds_alt<prob_avl_imbalance>(p)) {
                    const prob_avl_imbalance& wb = getv<prob_avl_imbalance>(p);

                    std::cout << " * Node " << wb.n->key << " violates AVL balance "
                              << "property (balance factor = " << wb.imbalance << ")"
                              << std::endl;
                }
            }
        }

    }

    /* check_for_cycles(n)
       Traverse the tree (preorder) starting at n, checking for cycles of nodes.
       Note that this does not check for parent-pointer cycles, only child-pointer
       cycles. Use `find_true_root` to find parent-pointer cycles.
    */
    static bool check_for_cycles(
        avl_tree::node* n, 
        std::set<avl_tree::node*>& nodes
    ) {
        if(n == nullptr)
            return true; // No cycles in an empty tree

        if(!cycle_check)
            return true; // No cycles

        if(nodes.count(n) > 0)
            return false;
        else {
            nodes.insert(n); // Mark n as seen

            // Explore left and right subtrees
            bool ret = true;
            if(n->left)
                ret = ret && check_for_cycles(n->left, nodes);
            if(n->right)
                ret = ret && check_for_cycles(n->right, nodes);

            return ret;
        }
    }

    static bool check_for_cycles(avl_tree::node* n) {
        if(n == nullptr)
            return true; // No cycles in an empty tree

        if(!cycle_check)
            return true;

        std::set<avl_tree::node*> nodes;

        if(!check_for_cycles(n, nodes)) {
            std::cout << "FAILED: tree structure contains a cycle.\n";
            return false;
        }
        else
            return true;
    }

    /* check_tree_pointers(root)
       Checks the parent/child pointers in the tree to ensure that they all line
       up correctly.
    */ 
    static bool check_tree_pointers(avl_tree::node* root, bool is_root = true) {
        if(!root)
            return true;
        else {
            if(is_root && root->parent != nullptr) {
                std::cout << "FAILED: root->parent should always be null.\n";
                return false;            
            }

            // Child child nodes (if they exist) to make sure their parents
            // point back to root.
            if(root->left) {
                if(root->left->parent != root) {
                    std::cout << "FAILED: found node " << root->left->key 
                         << " with incorrect parent pointer.\n";
                    return false;
                }
                if(root->left->key >= root->key) {
                    std::cout << "FAILED: found node " << root->left->key
                         << " which is on the wrong side of parent.\n";
                    return false;
                }
            }

            if(root->right) {
                if(root->right->parent != root) {
                    std::cout << "FAILED: found node " << root->right->key 
                         << " with incorrect parent pointer.\n";
                    return false;
                }
                if(root->right->key <= root->key) {
                    std::cout << "FAILED: found node " << root->right->key
                         << " which is on the wrong side of parent.\n";
                    return false;
                }            
            }
            
            if(root->right && root->left) {
                // Both children, if they exist, have valid parent pointers.
                // So now we check both subtrees recursively.
                return check_tree_pointers(root->left,  false) && 
                       check_tree_pointers(root->right, false);
            }

            return true;
        }
    }

    /* check_tree_values(root)
       Check the values (keys) in a tree to ensure that the tree is a valid BST.
    */
    static bool check_tree_values(
        avl_tree::node* root, 
        int low = std::numeric_limits<int>::min(),
        int high = std::numeric_limits<int>::max()) 
    {
        if(!root)
            return true;
        else if(root->key <= low) {
            std::cout << "FAILED: found node " << root->key << " improperly placed.\n";
            return false;
        }
        else if(root->key >= high) {
            std::cout << "FAILED: found node " << root->key << " improperly placed.\n";
            return false;   
        }
        else { // root->key is in the correct range
            return check_tree_values(root->left, low, root->key) &&
                   check_tree_values(root->right, root->key, high);
        }

    }

    /* check_tree_heights(root)
       Check the stored height values to ensure that they are correct.
    */
    static bool check_tree_heights(avl_tree::node* root)
    {
        auto height = [](avl_tree::node* n)
        {
            return n == nullptr ? 0 : n->height;
        };

        if(root == nullptr)
            return true;
        else if(root->height != std::max(height(root->left), height(root->right)) + 1)
            return false;
        else
            return check_tree_heights(root->left) and
                   check_tree_heights(root->right) ;
    }

    static bool check_tree_balance(avl_tree::node* root)
    {
        if(root == nullptr)
            return true;

        auto height = [](avl_tree::node* n)
        {
            return n == nullptr ? 0 : n->height;
        };

        if(std::abs(height(root->left) - height(root->right)) > 1)
            return false;
        else
            return check_tree_balance(root->left) and
                   check_tree_balance(root->right) ;
    }

    static bool check_tree(avl_tree::node* root, bool check_balance = true) 
    {
        if(root and root->parent != nullptr) {
            std::cout << "FAILED: Root of tree must have null parent pointer";
            std::cout << " (root->parent->key = " << root->parent->key << ")\n";
            return false;
        }
        
        bool cycle_result = check_for_cycles(root),
             ptrs_result = check_tree_pointers(root),
             values_result = check_tree_values(root),
             heights_result = check_tree_heights(root),
             bal_result = (not check_balance) or check_tree_balance(root) ;

        return cycle_result and 
               ptrs_result and
               values_result and
               heights_result and 
               bal_result ;
    }

    /* check_tree(t)
       Perform all checks against the given tree.
    */
    static bool check_tree(const avl_tree& t, bool check_balance = true)
    {
        return check_tree(t.rt, check_balance);
    }
};

void check_tree_printing();

int main()
{
    init_tests();

    // check_tree_printing();

    assign4_test_runner tr;

    bool result = tr.start();

    if(not result) {
        std::cout << "Some tests failed" << std::endl;
        return 1;
    }

    std::cout << "\u001b[32;1m*** All tests passed! ***" << ANSI_NORMAL() << std::endl;   
    return 0;
}

void print(const avl_tree& t)
{
    text_grid grid(80, 25);

    auto print_node = 
        [&grid](avl_tree::node* c, avl_tree::node*, bool, bool, int level) -> void
        {
            grid.print(80/2, level*2, c);    
        };


    assign4_test_runner::traverse_levelorder(
        t, 
        print_node
    );

    std::cout << grid << std::endl;
}

bool operator==(const avl_tree& a, const avl_tree& b)
{
    return assign4_test_runner::trees_equal(a, b);
}

std::ostream& operator<< (std::ostream& out, const avl_tree& t)
{
    //assign4_test_runner::simple_print(t);
    //print(t);
    assign4_test_runner::print_levelorder(t);
    return out;
}


// This function just exists to test the tree-printer; it constructs 
// various good/messed up trees and then prints them.
void check_tree_printing()
{
    auto t1 = assign4_test_runner::build_tree({
        {        7        },
        {    3,         9    },
        {  1,   0,   0,    11 },
        { 0,2, 0,0, 0, 0, 10, 0}  // y = 2 or 10
    });

    // If we rotate 1 with 3, this is the result
    auto t2 = assign4_test_runner::build_tree({
        {        7        },
        {    1,        9    },
        {  0,   3,   0,   11 },
        { 0,0, 2,0, 0,0, 10, 0 }
    });

    // If we rotate 11 with 9 this is the result
    auto t3 = assign4_test_runner::build_tree({
        {        7        },
        {    3,         11   },
        {  1,   0,   9,     0  },
        { 0,2, 0,0, 0,10, 0,  0 }
    }); 

    auto empty_tree = avl_tree{};

    auto single_node = assign4_test_runner::build_tree({{12}});

    auto all_left = assign4_test_runner::build_tree({
        {      7      },
        {   6,    0   },
        { 5,  0, 0, 0 },
        {4,0,0,0,0,0,0,0}
    });

    auto all_right = assign4_test_runner::build_tree({
        {      2       },
        {  0,        3   },
        { 0,  0,    0,  4  },
        {0,0,0,0,  0,0, 0,5 }
    });

    auto complete = assign4_test_runner::build_tree({
        {        8        },
        {    4,         12   },
        {  1,   6,   10,     14  },
        { -1,2, 5,7, 9,11, 13,  15 }
    }); 

    std::cout << "Empty: \n" << empty_tree << std::endl;

    std::cout << "Single node: \n" << single_node << std::endl;

    std::cout << "All left: \n" << all_left << std::endl;

    std::cout << "All right: \n" << all_right << std::endl;

    std::cout << "Complete:\n" << complete << std::endl;
    /*
    std::cout << t1 << std::endl;
    std::cout << t2 << std::endl;
    std::cout << t3 << std::endl;      
    */

    std::cout << "=== Incorrect trees ===" << std::endl;

    std::cout << "Wrong value/side:" << std::endl;
    auto wrong_side = assign4_test_runner::build_tree({
        {        7        },
        {    11,         12   },
        {  1,   0,   3,     0  },
        { 0,2, 0,0, 0, 9, 0,  0 }
    });  

    std::cout << wrong_side << std::endl;

    std::cout << "Wrong height" << std::endl;
    auto wrong_height = assign4_test_runner::build_tree({
        {        7        },
        {    3,         11   },
        {  1,   0,   9,     0  },
        { 0,2, 0,0, 0,10, 0,  0 }
    });  

    assign4_test_runner::get_root(wrong_height)->right->left->height = 3; // Height of 9
    assign4_test_runner::get_root(wrong_height)->left->left->height = 1;

    std::cout << wrong_height << std::endl;

    std::cout << "Wrong child:" << std::endl;
    auto wrong_child = assign4_test_runner::build_tree({
        {        7        },
        {    3,         11   },
        {  1,   0,   9,     0  },
        { 0,2, 0,0, 0,10, 0,  0 }
    });  

    // We set 11's right child to be 3, which has already been seen in the tree
    assign4_test_runner::get_root(wrong_child)->right->right = 
        assign4_test_runner::get_root(wrong_child)->left ;

    std::cout << wrong_child << std::endl;

    // We have to put the tree back correctly before it's destroyed
    assign4_test_runner::get_root(wrong_child)->right->right = nullptr;

    auto wrong_child2 = assign4_test_runner::build_tree({
        {        7        },
        {    3,         11   },
        {  1,   0,   9,     0  },
        { 0,2, 0,0, 0,10, 0,  0 }
    });  

    // Here we set the right child of 3 to be 7, the root (creating a cycle).
    assign4_test_runner::get_root(wrong_child2)->left->right = 
        assign4_test_runner::get_root(wrong_child2) ;

    std::cout << "Wrong child with cycle: " << std::endl;
    std::cout << wrong_child2 << std::endl;

    assign4_test_runner::get_root(wrong_child2)->left->right = nullptr;

    std::cout << "Wrong parent:" << std::endl;

    auto wrong_parent = assign4_test_runner::build_tree({
        {        7        },
        {    3,         11   },
        {  1,   0,   9,     0  },
        { 0,2, 0,0, 0,10, 0,  0 }
    });  

    // We set the parent of 3 to be 11
    assign4_test_runner::get_root(wrong_parent)->left->parent = 
           assign4_test_runner::get_root(wrong_parent)->right ;

    std::cout << wrong_parent << std::endl;
}