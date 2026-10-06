/*====================================================================
  AVL tree                                   
  A BST node carries a cached height, and every node keeps its two
  subtree heights within one of each other.  That one restriction forces
  height O(log n) for any insertion order, so the tree never degenerates.
====================================================================*/

struct AvlNode
{
    ElementType Element;
    struct AvlNode *Left;
    struct AvlNode *Right;
    int Height;
};

typedef struct AvlNode *AvlTree;



/*
 * The empty subtree is defined as height -1, not 0, so a leaf has height 0
 */
static int
Height( AvlTree T )
{
    return T == NULL ? -1 : T->Height;
}

/* Return the larger of two integers.*/
static int
Max( int A, int B )
{
    return A > B ? A : B;
}

/*
 * Recompute the height of one node after its children changed.
 */
static void
UpdateHeight( AvlTree T )
{
    T->Height = Max( Height( T->Left ), Height( T->Right ) ) + 1;
}

/*
 * Rotate a left-heavy node with its left child and return the new root.
 */
static AvlTree
SingleRotateWithLeft( AvlTree K2 )
{
    AvlTree K1 = K2->Left;

    K2->Left = K1->Right;
    K1->Right = K2;
    UpdateHeight( K2 ); //Remember to update the height of the nodes
    UpdateHeight( K1 );
    return K1;                             
}

/* The mirror of SingleRotateWithLeft,
 * Rotate a right-heavy node with its right child and return the new root.
 */
static AvlTree
SingleRotateWithRight( AvlTree K1 )
{
    AvlTree K2 = K1->Right;

    K1->Right = K2->Left;
    K2->Left = K1;
    UpdateHeight( K1 );
    UpdateHeight( K2 );
    return K2;                            
}

/*
 * Rebalance the AVL Tree when a node is inserted or deleted
 * To keep the balancing condition of AVL Tree after executing insert or delete
 */
static AvlTree
AvlBalance( AvlTree T )
{
    if( T == NULL )
        return T;

    if( Height( T->Left ) - Height( T->Right ) > 1 )
    {
        if( Height( T->Left->Left ) < Height( T->Left->Right ) )
            T->Left = SingleRotateWithRight( T->Left );  //correspond to LR
        T = SingleRotateWithLeft( T ); // correspond to LL
    }
    else if( Height( T->Right ) - Height( T->Left ) > 1 )
    {
        if( Height( T->Right->Right ) < Height( T->Right->Left ) )
            T->Right = SingleRotateWithLeft( T->Right ); //correspond to RL
        T = SingleRotateWithRight( T ); //correspond to RR
    }
    UpdateHeight( T );
    return T;
}

/*
 * Insert a key and rebalance the ancestors of the new leaf.
 */
static AvlTree
AvlInsert( ElementType X, AvlTree T )
{
    if( T == NULL )
    {
        T = NewNode( sizeof( *T ) ); // call for memory
        T->Element = X; // Insert the new node
    }
    else if( X < T->Element )
        T->Left = AvlInsert( X, T->Left );
    else if( X > T->Element )
        T->Right = AvlInsert( X, T->Right );

    return AvlBalance( T );               
}

/*
 * Delete a key and repair every ancestor the removal disturbed.
 */
static AvlTree
AvlDelete( ElementType X, AvlTree T )
{
    AvlTree P;

    if( T == NULL )
        return NULL;
    if( X < T->Element )
        T->Left = AvlDelete( X, T->Left );
    else if( X > T->Element )
        T->Right = AvlDelete( X, T->Right );
    else if( T->Left != NULL && T->Right != NULL ) // X == T->Element but T still have two children
    {
        P = T->Right;
        while( P->Left != NULL )
            P = P->Left;
        T->Element = P->Element;
        T->Right = AvlDelete( P->Element, T->Right ); // Give the problem to the right subtree
    }
    else
    {
        P = T->Left != NULL ? T->Left : T->Right;
        free( T );
        return P;
    }
    return AvlBalance( T );
}

/*
 * Find a node with key X
 */
static int
AvlFind( ElementType X, AvlTree T )
{
    while( T != NULL && T->Element != X )
        T = X < T->Element ? T->Left : T->Right;
    return T != NULL;
}

/*
 * Release an AVL tree in postorder and return NULL. 
 */
static AvlTree
AvlMakeEmpty( AvlTree T )
{
    if( T != NULL )
    {
        AvlMakeEmpty( T->Left );
        AvlMakeEmpty( T->Right );
        free( T );
    }
    return NULL;
}


