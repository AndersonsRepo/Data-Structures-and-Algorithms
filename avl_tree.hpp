#pragma once
/* 
 * avl_tree.hpp
 * Class definition for AVL trees.
 */
#include <cassert>
#include <algorithm>
/* not_implemented
   Thrown by functions you haven't written yet, to signal to the test-runner
   to not test them.
*/
struct not_implemented {};

/* avl_tree
   AVL (height-balanced) tree class.
*/
class avl_tree {
  public:
    struct node
    {
        int key;
        node* left;
        node* right;
        node* parent;
        int height;
    };

    /* root()
       Helper function, returns the root of the tree.
    */
    node* root() { return rt; }

    /* height(n)
       Returns the height of n. This helper function exists to get the 
       height of a (possibly nullptr) node. If you just do n->height, then your
       program will crash with a segfault if n is ever nullptr, but there are
       lots of places where we want to get the height without knowing whether
       a node exists or not.
    */
    static int height(node* n) { return n == nullptr ? 0 : n->height; }

    /* default constructor
       The default avl_tree is empty, so this constructor does nothing (it must
       be defined, however, because the presence of the copy constructor below 
       causes the normal compiler-generated constructor to be deleted).
    */
    avl_tree() { }

    /* destructor
       Destroy all the nodes in the tree. 

       See below for the `destroy()` method which you must implement.
    */
    ~avl_tree()
    {
        destroy(rt);
        rt = nullptr;
    }

    /* copy constructor
       Construct a copy of the `original` tree.

       See below for the `copy()` method which you must implement.
    */
    avl_tree(const avl_tree& original)
    {
        rt = copy(original.rt);
    }

    /* copy assignment
       Copy `original` into the current tree.
    */
    avl_tree& operator= (const avl_tree& original)
    {
        if(rt == original.rt)
            return *this;

        destroy(rt);
        rt = copy(rt);

        return *this;
    }

    /* size()
       Returns the size (number of nodes) in the tree.

       See below for the version of this function that you need to implement.
    */
    int size() const
    {
        return size(rt);
    }

    /* ---------------- Complete all the following functions ---------------- */
    // Note: There is one function (`rotate()`) in the private section which you
    // must also complete.

    /* height()
       Returns the height of the tree (empty tree has height of 0).

       Must run in O(1) time.
    */
    int height() const
    {
        return height(rt);
    }

    /* empty()
       Returns true if the tree is empty.

       Must run in O(1) time.
    */
    bool empty() const
    {
        if(rt == nullptr)
        {
        	return true;
        }
        else
        	return false;
    }

    /* find(x)
       Returns a pointer to the node containing `x`, or nullptr if `x` is not
       found in the tree.

       Must run in O(log n) time.

       NOTE: If you want to write `find` recursively, you'll need to add a 
       private helper function

            node* find(node* n, int x);

       to do the actual recursion.
    */
    node* find(int x)
    {
		return find(rt, x);		
    }

    /* insert(x)
       Insert `x` into the tree. If `x` already exists in the tree, then the
       tree should be left unchanged. Otherwise, if `x` is added to the tree,
       then the tree should be rebalanced as necessary, so that the AVL height
       property still holds.

       Must run in O(log n) time.
    */
    void insert(int x)
    {   
        //Condition Checks
        if(rt == nullptr)
        {
           rt = new node{x, nullptr, nullptr, nullptr, 1};
           return;
        }
        
        node* current = rt;
        node* parent = nullptr;

        //Node traversal to leaf node
        while(current != nullptr)
        {
          if(current->key == x)
          {
             return;
          }

          parent = current;
          if(current->key > x)
            {
               current = current->left;
            }
          else
          {
            current = current->right;
          }
               
        }

         //Node creation at end of tree and parent pointer adjustment
        node* ins_node = new node{x, nullptr, nullptr, parent, 1};
        if(parent->key > x)
        {
           parent->left = ins_node;
        }
        else
           parent->right = ins_node;
      
        //Balancing factor portion
        node* bal_parent = parent;
        while(bal_parent != nullptr)
        {
           bal_parent->height = 1 + std::max(height(bal_parent->left), height(bal_parent->right));
           int bal_fact = height(bal_parent->left) - height(bal_parent->right);
         
           //left heavy
           if(bal_fact > 1)
           {
              //outside imbalance
              if(x < bal_parent->left->key)
              { 
                 rotate(bal_parent->left);
              }
              //inside imbalance
              else
              {
                 rotate(bal_parent->left->right);
                 rotate(bal_parent->left);
              }
           }
           //right heavy
           else if(bal_fact < -1)
           {
              //outside imbalance
              if(x > bal_parent->right->key) 
              {
                 rotate(bal_parent->right);
              }
              //inside imbalance
              else if(x < bal_parent->right->key)
              {
                 rotate(bal_parent->right->left);
                 rotate(bal_parent->right);
              }
           }

           bal_parent = bal_parent->parent;
        }

     }


    /* size(n)
       Returns the size of the tree under the node `n`, using recursion. This
       is the (recursive) implementation of the `size()` method, above.

       Must run in O(n) time. Note that because you are not allowed to add 
       additional private data members, you cannot use the `int sz;` trick to
       make this O(1).
    */
    static int size(node* root)
    {
    	if(root == nullptr)
    	{
    		return 0;
    	}

    	int sl = size(root->left);
    	int sr = size(root->right);

    	return sl + sr + 1;
    }

    /* destroy(root)
       Destroy `root` and all its descendants. 

       Must run in O(n) time, where 

            n = size(root)

       This is called from the destructor to destroy the tree.
    */
    static void destroy(node* root)
    {
      if(root == nullptr)
      {
      	return;
      }

      destroy(root->left);
      destroy(root->right);

      delete root;
    }

    /* copy(root, parent)
       Return a copy of the tree under `root`, with root's parent
       being `parent`.

       Must run in O(n) time.
    */
    static node* copy(node* root, node* parent = nullptr)
    {
        if(root == nullptr)
		{
          return nullptr;
        }

		node* copy_tree = new node{root->key, nullptr, nullptr, parent, 0};
		
      copy_tree->left = copy(root->left, copy_tree);
      copy_tree->right = copy(root->right, copy_tree);

      copy_tree->height = 1 + std::max(height(copy_tree->left), height(copy_tree->right));

      return copy_tree;
    }

    /* --------------------------- Extra Credit --------------------------- */

    // If you complete the functions in this section, then this assignment
    // will count as two assignments, for the purposes of computing your grade.

    /* merge_with(other)
       Merge this tree with `other`. This is similar to `insert`-ing all the 
       values in `other`, although there are more efficient ways to go about 
       doing it.

       Must run in O(n) time. (Note that `insert`-ing all the values in `other`
       would run in O(n log n) time, so this requirement rules out that 
       approach.) n here is the total size of the final tree (including all the
       nodes in this tree, plus all the nodes in the other tree). Must run
       in O(n) space. There are special cases where the runtime can be reduced 
       to O(log n) time.

       You may need to write some helper functions to aid with this.
    */
    void merge_with(const avl_tree& other)
    {
        throw not_implemented{};
    }

  private:

    /* ------------------------- Not Extra Credit ------------------------- */

    // `rotate` is not an extra-credit function, it is a key part of the AVL 
    // tree implementation.

    /* rotate(child)
       Rotate `child` with its parent, updating the heights of both as 
       necessary.

       Must run in O(1) time.

       NOTE: `rotate` is *not* static, because it sometimes needs to update 
       the root `rt` of the tree!

       You can assume that `child != nullptr` and `child->parent != nullptr`.
    */
    void rotate(node* child)
    {
        assert(child != nullptr);
        assert(child->parent != nullptr);

        node* parent = child->parent;
        node* grandParent = child->parent->parent;

        if(child == parent->left)
          {
            node* c_greater = child->right;
            child->right = parent;
            parent->left = c_greater;

            if(c_greater != nullptr)
              {
               c_greater->parent = parent;
              }
          }
        else if(child == parent->right)
          {
            node* c_lesser = child->left;
            child->left = parent;
            parent->right = c_lesser;
            if(c_lesser != nullptr)
              {
                c_lesser->parent = parent;
              }
          }

        parent->parent = child;
        child->parent = grandParent;

         //Checks that grandparent exists and decides which side is the child
         //Based on key value
         //If Grandparent doesn't exist Child is the root
         if(grandParent != nullptr)
         {
           if(parent->key < grandParent->key)
              {
               grandParent->left = child;
              }
            else if(parent->key > grandParent->key)
              {
                  grandParent->right = child;
              }
         }
         else
           {
             rt = child;
             child->parent = nullptr;
           }

         //compares the sides of the tree to find the largest and adds one to it
         parent->height = 1 + std::max(
            height(parent->left),
            height(parent->right));
         
         child->height = 1 + std::max(
            height(child->right),
            height(child->left));
    }

    // Root of the tree (nullptr = empty tree).
    node* rt = nullptr;

    // Add any other private members/functions you want/need.
    // DO NOT add any additional data members; they will break the test code. :(
    node* find(node* n, int x)
    {
    	if(n == nullptr)
    	{
    		return nullptr;
    	}
    	
    	if(x == n->key)
    	{
    		return n;
    	}

    	else if(x < n->key)
    	{
    		return find(n->left, x);
    	}
    	else
    	{
    		return find(n->right, x);
    	}
    }
    friend class assign4_test_runner;
};
