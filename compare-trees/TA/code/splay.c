/*====================================================================
  Splay tree
  No stored height and no colour: updates splay around the key,
  moving the accessed key to the root, which gives amortized O(log n)
  over any sequence of updates while leaving the tree free to become a
  chain in between. 
====================================================================*/

struct SplayNode
{
    ElementType Element;
    struct SplayNode *Left;
    struct SplayNode *Right;
};

typedef struct SplayNode *SplayTree;

/*
 * Bring a key to the root, or the last node reached if it is absent.
 *
 * This is top-down splaying: the access path is split as it is traversed
 * into two partial trees hung from a header node, LeftMax collecting the
 * keys below X and RightMin those above.  Collapsing a zig-zig before the
 * node joins a partial tree keeps repeated-direction paths short.
 */
static SplayTree
Splay( ElementType X, SplayTree T )
{
    struct SplayNode Header = { 0, NULL, NULL };   // Dummy parent of both partial trees
    SplayTree LeftMax = &Header, RightMin = &Header, P;

    if( T == NULL )
        return NULL;
    for( ;; )
    {
        if( X < T->Element )
        {
            if( T->Left == NULL )
                break;                     // No smaller child: T is the ceiling of X
            if( X < T->Left->Element )
            {
                P = T->Left;               // Zig-zig: collapse both left links
                T->Left = P->Right;
                P->Right = T;
                T = P;
                if( T->Left == NULL )
                    break;
            }
            RightMin->Left = T;            // T is above X: hang it on RightMin
            RightMin = T;
            T = T->Left;
        }
        else if( X > T->Element )
        {
            if( T->Right == NULL )
                break;                     // No larger child: T is the floor of X
            if( X > T->Right->Element )
            {
                P = T->Right;              // Zig-zig: collapse both right links
                T->Right = P->Left;
                P->Left = T;
                T = P;
                if( T->Right == NULL )
                    break;
            }
            LeftMax->Right = T;            // T is below X: hang it on LeftMax
            LeftMax = T;
            T = T->Right;
        }
        else
            break;                         // Found X
    }
    LeftMax->Right = T->Left;              // T's own subtrees straddle X
    RightMin->Left = T->Right;
    T->Left = Header.Right;                // Reassemble: below ... T ... above
    T->Right = Header.Left;
    return T;
}

/*
 * Insert a key by splaying around it and making it the new root. 
 */
static SplayTree
SplayInsert( ElementType X, SplayTree T )
{
    SplayTree P;

    T = Splay( X, T );
    if( T != NULL && T->Element == X )
        return T;                          // Already present
    P = NewNode( sizeof( *P ) );
    P->Element = X;
    if( T != NULL )
    {
        if( X < T->Element )               // X precedes the splayed root; smaller keys may remain
        {
            P->Left = T->Left;
            P->Right = T;
            T->Left = NULL;
        }
        else                               // X follows the splayed root; larger keys may remain
        {
            P->Right = T->Right;
            P->Left = T;
            T->Right = NULL;
        }
    }
    return P;
}

/*
 * Delete a key after splaying it to the root.  
 */
static SplayTree
SplayDelete( ElementType X, SplayTree T )
{
    SplayTree P;

    T = Splay( X, T );
    if( T == NULL || T->Element != X )
        return T;                          // Absent: only the splay happened
    if( T->Left == NULL )
        P = T->Right;                      // No left subtree: the right one stands in
    else
    {
        P = Splay( X, T->Left );           // New root has no right child left
        P->Right = T->Right;
    }
    free( T );
    return P;
}

/*
 * Report whether a key is present, without modifying the tree.  
 */
static int
SplayFind( ElementType X, SplayTree T )
{
    while( T != NULL && T->Element != X )
        T = X < T->Element ? T->Left : T->Right;
    return T != NULL;
}

/*
 * Release a splay tree iteratively and return NULL. 
 */
static SplayTree
SplayMakeEmpty( SplayTree T )
{
    SplayTree P;

    while( T != NULL )
    {
        if( T->Left != NULL )              // Rotate the left child up over T
        {
            P = T->Left;
            T->Left = P->Right;
            P->Right = T;
            T = P;
        }
        else
        {
            P = T->Right;                  // Nothing left below: T is freeable
            free( T );
            T = P;
        }
    }
    return NULL;
}


