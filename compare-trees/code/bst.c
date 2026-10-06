/*====================================================================
  Unbalanced binary search tree
  One key and two child links per node, every left subtree's key are smaller
  than the node's key, while every right subtree's key are bigger than the
  node's key. 
====================================================================*/

struct BstNode
{
    ElementType Element;
    struct BstNode *Left;
    struct BstNode *Right;
};

typedef struct BstNode *BstTree;

/*
 * Insert a key and return the root.  
 */
static BstTree
BstInsert( ElementType X, BstTree T )
{
    BstTree *Link = &T;                    // The link that will own the new node

    while( *Link != NULL )
    {
        if( X == ( *Link )->Element )
            return T;                      // Duplicate: the set is unchanged
        Link = X < ( *Link )->Element ? &( *Link )->Left : &( *Link )->Right;
    }
    *Link = NewNode( sizeof( **Link ) );   // Splice the new leaf into that link
    ( *Link )->Element = X;
    return T;
}

/*
 * Delete a key and return the root.
 */
static BstTree
BstDelete( ElementType X, BstTree T )
{
    BstTree *Link = &T, *Succ;             // Link points at the link owning the node
    BstTree Old;

    while( *Link != NULL && ( *Link )->Element != X )
        Link = X < ( *Link )->Element ? &( *Link )->Left : &( *Link )->Right;
    if( *Link == NULL )
        return T;                          // X is not in the tree

    Old = *Link;
    if( Old->Left != NULL && Old->Right != NULL )   // Two children: use the successor
    {
        Succ = &Old->Right;
        while( ( *Succ )->Left != NULL )
            Succ = &( *Succ )->Left;       // Leftmost node of the right subtree
        Old->Element = ( *Succ )->Element; // Copy the successor key up into Old
        Link = Succ;                       // and unlink the successor instead
        Old = *Link;
    }
    *Link = Old->Left != NULL ? Old->Left : Old->Right;   // Bypass the single child
    free( Old );                           // Old is out of the tree: release it
    return T;
}

/*
 * Report whether a key is present, without modifying the tree.
 */
static int
BstFind( ElementType X, BstTree T )
{
    while( T != NULL && T->Element != X )
        T = X < T->Element ? T->Left : T->Right;
    return T != NULL;
}

/*
 * Release every node and return NULL.  
 */
static BstTree
BstMakeEmpty( BstTree T )
{
    BstTree P;

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


