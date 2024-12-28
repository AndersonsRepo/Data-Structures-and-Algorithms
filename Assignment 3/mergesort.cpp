/*
 * mergesort.cpp
 * Mergesort on singly-linked lists.
 */

/* node
   The node type for our linked list.
*/
struct node 
{
    int value;
    node* next;
};

// Declarations for the functions below.
node* mergesort(node* input);
node* mergesort(node* input, int length);
node* merge(node* left, node* right);

/* mergesort(input)
   Mergesort the list whose head is `input`, returning a new sorted list. This
   function should compute the length of the list, and then pass `input` and
   the length to the two-parameter recursive `mergesort` overload, below.

   Must run in O(n log n) time (but that's because `mergesort(node*,int)`, below
   runs in O(n log n)).

   Must use O(n) space (i.e., the returned list is created new).
*/
node* mergesort(node* input)
{
    int length = 0;

    for(node* i = input; i != nullptr; i = i->next)
    {
        length++;
    }

    return mergesort(input, length);
};

/* mergesort(input, length)
   Recursively Mergesort the `input` list (whose length is given by `length`),
   returning a new sorted list. 

   Must run in O(n log n) time, where n = `length`.

   Must use O(n) space (returned list is created new). 

   NOTE: The `input` list must not be modified in any way.
*/
node* mergesort(node* input, int length)
{   
    if(length <= 1)
    {
        if(input == nullptr)
        {
            return nullptr;
        }
        node* n_node = new node();
        n_node->value = input->value;
        n_node->next = nullptr;
        return n_node;
    }
    
    //establish head of left side and length
    node* left = input;
    int l_size = length / 2;

    node* head_left = new node();
    node* n_left = head_left;

    n_left->value = input->value;

    //iterate through original list copying over values and iterating place holder "left"
    for(int i = 0; i < l_size; ++i)
    {
        n_left->next = new node();
        n_left->next->value = left->next->value;
        n_left->next->next = nullptr;
        n_left = n_left->next;
        left = left->next;
    }

    //Establishes placeholder for right side of list
    node* right = left;
    //I don't think this code is needed since left is being incremented in the previous loop
    /*for(int i = 0; i < l_size; ++i)
    {
        right = right->next;
    }
    */

    //Creates new list and length of new list
    node* head_right = new node();
    node* n_right = head_right;

    n_right->value = right->value;

    int r_size = length - length / 2;

    //Iterates through copying each value from original list to new right side of list
    for(int i = 0; i < r_size; ++i)
    {
        n_right->next = new node();
        n_right->next->value = right->next->value;
        n_right->next->next = nullptr;
        n_right = n_right->next;
        right = right->next; 
    }

    mergesort(n_left, l_size); //Use n_left variable for the input for the node 
    mergesort(n_right, r_size); //Use n_right variable for the input for the node 
    
    return merge(n_left, n_right);
}

/* merge(left, right)
   Merge the lists given by `left` and `right`, returning the head of the merged
   list (the returned node should either be the head of `left` or the head of
   `right`). The two input lists will always be sorted in ascending order.

   Must run in O(m+n) time where `m` is the length of `left` and `n` is the 
   length of `right`.

   Must use O(1) space (i.e., this is an in-place operation); no new nodes 
   may be created.

   NOTE: This function should modify the `next` pointers in the nodes, but NOT
   modify the `value`s. That is, it merges the lists by updating their list
   structure, not by moving values from one list to another.
*/              
node* merge(node* left, node* right)
{
    node* hd = nullptr;
    node* n_left = left;
    node* n_right = right;

    if(n_left == nullptr && n_right == nullptr)
    {
        return nullptr;
    }
    else if(n_left != nullptr && n_right == nullptr)
    {
        return left;
    }
    else if(n_right != nullptr && n_left == nullptr)
    {
        return right;
    }
    else
    {
        if(left->value < right->value)
        {
            n_left->next = n_right;
            hd = n_left;
        }
        else if(right->value < left->value)
        {
            n_right->next = n_left;
            hd = n_right;
        }
    }

    merge(left, right);

    return hd;
}
