/*====================================================================
  Left-leaning red-black tree
  Red links encode a 2-3 tree, and requiring every red link to lean left
  makes the encoding canonical.  The root is black, no red node has a red
  child, and every path down to a missing link crosses the same number of
  black links.
====================================================================*/

struct RedBlackNode
{
    ElementType Element;
    struct RedBlackNode *Left;
    struct RedBlackNode *Right;
    int Red; //whether a node is Red or Black
};

typedef struct RedBlackNode *RedBlackTree;

/*
 * Test whether the link into a subtree is red.
 */
static int
IsRed( RedBlackTree T )
{
    return T != NULL && T->Red;
}

/*
 * Rotate a red right link into a red left link and return the new root.
 */
static RedBlackTree
RotateLeft( RedBlackTree T )
{
    RedBlackTree P = T->Right;

    T->Right = P->Left;                    // Middle subtree changes parent
    P->Left = T;
    P->Red = T->Red;                       // The two colours swap over
    T->Red = 1;
    return P;
}

/* The mirror of RotateLeft, with the same colour transfer and the same
   preservation of black height. */
static RedBlackTree
RotateRight( RedBlackTree T )
{
    RedBlackTree P = T->Left;

    T->Left = P->Right;                    // Middle subtree changes parent
    P->Right = T;
    P->Red = T->Red;                       // The two colours swap over
    T->Red = 1;
    return P;
}

/*
 * Toggle the colour of a node and of both links to its children. 
 */
static void
FlipColors( RedBlackTree T )
{
    T->Red = !T->Red;                      // Split a 4-node, or join a 2-node
    T->Left->Red = !T->Left->Red;
    T->Right->Red = !T->Right->Red;
}

/*
 * Restore the canonical left-leaning shape after an update.  
 */
static RedBlackTree
RedBlackBalance( RedBlackTree T )
{
    if( IsRed( T->Right ) )                // Rule 1: red links lean left
        T = RotateLeft( T );
    if( IsRed( T->Left ) && IsRed( T->Left->Left ) )
        T = RotateRight( T );              // Rule 2: no two reds in a row
    if( IsRed( T->Left ) && IsRed( T->Right ) )
        FlipColors( T );                   // Rule 3: split the 4-node
    return T;
}

/*
 * Recursive insertion of one key.  
 */
static RedBlackTree
RedBlackInsertNode( ElementType X, RedBlackTree T )
{
    if( T == NULL )
    {
        T = NewNode( sizeof( *T ) );
        T->Element = X;
        T->Red = 1;                        // A new key always arrives red
        return T;
    }
    if( X < T->Element )
        T->Left = RedBlackInsertNode( X, T->Left );
    else if( X > T->Element )
        T->Right = RedBlackInsertNode( X, T->Right );

    return RedBlackBalance( T );           // Repair on the way back up
}

/*
 * Insert a key and return a tree whose root satisfies the public rule.
 */
static RedBlackTree
RedBlackInsert( ElementType X, RedBlackTree T )
{
    T = RedBlackInsertNode( X, T );
    T->Red = 0;                            // The public root is always black
    return T;
}

/*
 * Report whether a key is present, without modifying the tree.  
 */
static int
RedBlackFind( ElementType X, RedBlackTree T )
{
    while( T != NULL && T->Element != X )
        T = X < T->Element ? T->Left : T->Right;
    return T != NULL;
}

/*
 * Prepare the left branch before a deletion descends into it.  
 */
static RedBlackTree
MoveRedLeft( RedBlackTree T )
{
    FlipColors( T );                       // T, and both children, change colour
    if( IsRed( T->Right->Left ) )          // Right child is a 3-node: borrow its key
    {
        T->Right = RotateRight( T->Right );
        T = RotateLeft( T );
        FlipColors( T );
    }
    return T;
}

/* The mirror of MoveRedLeft, borrowing from the left sibling when it is
   not a bare 2-node. */
static RedBlackTree
MoveRedRight( RedBlackTree T )
{
    FlipColors( T );                       // T, and both children, change colour
    if( IsRed( T->Left->Left ) )           // Left child is a 3-node: borrow its key
    {
        T = RotateRight( T );
        FlipColors( T );
    }
    return T;
}

/*
 * Remove the smallest key of a subtree.  
 */
static RedBlackTree
DeleteMin( RedBlackTree T )
{
    if( T->Left == NULL )
    {
        free( T );                         // This is the minimum itself
        return NULL;
    }
    if( !IsRed( T->Left ) && !IsRed( T->Left->Left ) )
        T = MoveRedLeft( T );              // Make the left branch able to lose a key
    T->Left = DeleteMin( T->Left );
    return RedBlackBalance( T );
}

/*
 * Delete a key that is known to be present.  
 */
static RedBlackTree
RedBlackDeleteNode( ElementType X, RedBlackTree T )
{
    RedBlackTree P;

    if( X < T->Element )
    {
        if( !IsRed( T->Left ) && !IsRed( T->Left->Left ) )
            T = MoveRedLeft( T );
        T->Left = RedBlackDeleteNode( X, T->Left );
    }
    else
    {
        if( IsRed( T->Left ) )             // Lean the other way before descending right
            T = RotateRight( T );
        if( X == T->Element && T->Right == NULL )
        {
            free( T );                     // Found it, and it is a leaf here
            return NULL;
        }
        if( !IsRed( T->Right ) && !IsRed( T->Right->Left ) )
            T = MoveRedRight( T );
        if( X == T->Element )
        {
            P = T->Right;                  // Two children: use the successor
            while( P->Left != NULL )
                P = P->Left;               // Leftmost node of the right subtree
            T->Element = P->Element;       // Copy the successor key up into T
            T->Right = DeleteMin( T->Right );   // and remove the successor instead
        }
        else
            T->Right = RedBlackDeleteNode( X, T->Right );
    }
    return RedBlackBalance( T );
}

/*
 * Delete a key, or leave the tree unchanged.  
 */
static RedBlackTree
RedBlackDelete( ElementType X, RedBlackTree T )
{
    if( !RedBlackFind( X, T ) )
        return T;                          // Absent: the tree is untouched
    if( !IsRed( T->Left ) && !IsRed( T->Right ) )
        T->Red = 1;                        // Root must be able to lose a key
    T = RedBlackDeleteNode( X, T );
    if( T != NULL )
        T->Red = 0;                        // Restore the black root
    return T;
}

/*
 * Release a red-black tree in postorder and return NULL. 
 */
static RedBlackTree
RedBlackMakeEmpty( RedBlackTree T )
{
    if( T != NULL )
    {
        RedBlackMakeEmpty( T->Left );
        RedBlackMakeEmpty( T->Right );
        free( T );
    }
    return NULL;
}


