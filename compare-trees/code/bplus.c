/*====================================================================
  B+ tree, capacity 32
  Interior nodes hold only copies of separator keys and up to 32 children,
  Records live only in the leaves, which are chained in sorted order by Next
  Every non-root node hold the number of children from (Capacity / 2) to Capacity 
====================================================================*/

#define CAPACITY 32
#define MINIMUM ( CAPACITY / 2 )

struct BPlusNode
{
    int Leaf;                                  //whether the node is a leaf
    int Count;                                 // the data count in a leaf or the index count in a interior node
    ElementType Minimum;                      // to cache the minimum key in subtree
    ElementType Key[ CAPACITY + 1 ];         // data in leaves or separator key in interior nodes
    struct BPlusNode *Child[ CAPACITY + 1 ];// children pointer
    struct BPlusNode *Next;                 //pointer to next leaf
};

typedef struct BPlusNode *BPlusTree;

/*
 * Update the minimum of a leave or Update the separate index of an interior node
 */
static void
Refresh( BPlusTree T )
{
    int I;

    if( T->Leaf )
    {
        if( T->Count != 0 )                // An empty leaf has no minimum to cache
            T->Minimum = T->Key[ 0 ];
    }
    else
    {
        T->Minimum = T->Child[ 0 ]->Minimum;
        for( I = 1; I < T->Count; ++I )
            T->Key[ I - 1 ] = T->Child[ I ]->Minimum;   // Separator i = min of child i+1
    }
}

/*
 * Find the child of an interior node that owns a key. 
 */
static int
ChildIndex( ElementType X, BPlusTree T )
{
    int Low = 0, High = T->Count - 1, Mid;

    while( Low < High )
    {
        Mid = ( Low + High ) / 2;
        if( X < T->Key[ Mid ] )
            High = Mid;
        else
            Low = Mid + 1;                 // X == separator also goes right
    }
    return Low;
}

/*
 * Report whether a key is stored in the tree.
 */
static int
BPlusFind( ElementType X, BPlusTree T )
{
    int Low = 0, High, Mid;

    if( T == NULL )
        return 0;
    while( !T->Leaf )
        T = T->Child[ ChildIndex( X, T ) ];   // Routing only: records are in the leaves
    High = T->Count;
    while( Low < High )
    {
        Mid = ( Low + High ) / 2;
        if( T->Key[ Mid ] < X )
            Low = Mid + 1;
        else
            High = Mid;
    }
    return Low < T->Count && T->Key[ Low ] == X;
}

/*
 * Insert into one node recursively and report whether it had to split.
 */
static BPlusTree
BPlusInsertNode( ElementType X, BPlusTree T )
{
    BPlusTree Right;
    int I, J;

    if( T->Leaf )
    {
        for( I = 0; I < T->Count && T->Key[ I ] < X; ++I )
            ;                              // I is the gap the record belongs in
        if( I < T->Count && T->Key[ I ] == X )
            return NULL;                   // Already present
        for( J = T->Count; J > I; --J )
            T->Key[ J ] = T->Key[ J - 1 ]; // Open the gap
        T->Key[ I ] = X;
        ++T->Count;
    }
    else
    {
        I = ChildIndex( X, T );
        Right = BPlusInsertNode( X, T->Child[ I ] );
        if( Right != NULL )
        {
            for( J = T->Count; J > I + 1; --J )
                T->Child[ J ] = T->Child[ J - 1 ];
            T->Child[ I + 1 ] = Right;     // The new sibling sits next to child I
            ++T->Count;
        }
    }
    Refresh( T );
    if( T->Count <= CAPACITY )
        return NULL;                       // Fits: no split

    Right = NewNode( sizeof( *Right ) );   // Overflow: split into two nodes
    Right->Leaf = T->Leaf;
    Right->Count = T->Count - MINIMUM;
    for( I = 0; I < Right->Count; ++I )
        if( T->Leaf )
            Right->Key[ I ] = T->Key[ I + MINIMUM ];
        else
            Right->Child[ I ] = T->Child[ I + MINIMUM ];
    T->Count = MINIMUM;                    // The left node keeps the first half
    if( T->Leaf )
    {
        Right->Next = T->Next;             // Splice the new leaf into the chain
        T->Next = Right;
    }
    Refresh( T );
    Refresh( Right );
    return Right;
}

/*
 * Insert a key, growing the tree by one level when the root splits. 
 */
static BPlusTree
BPlusInsert( ElementType X, BPlusTree T )
{
    BPlusTree Right, P;

    if( T == NULL )
    {
        T = NewNode( sizeof( *T ) );
        T->Leaf = 1;
    }
    Right = BPlusInsertNode( X, T );
    if( Right == NULL )
        return T;

    P = NewNode( sizeof( *P ) );           // New root over the two halves
    P->Count = 2;
    P->Child[ 0 ] = T;
    P->Child[ 1 ] = Right;
    Refresh( P );
    return P;
}

/*
 * Restore minimum occupancy after one child loses an entry.  A child still
 * at MINIMUM is fine, so the common case returns at once.  Otherwise one
 * entry is borrowed from an adjacent sibling that has more than MINIMUM
 * entries, shifting the recipient to keep it sorted; when both siblings
 * are minimal, two adjacent children merge.
 */
static void
RepairChild( BPlusTree T, int I )
{
    BPlusTree P = T->Child[ I ], L, R;
    int J;

    if( P->Count >= MINIMUM )
        return;                            // Still legal: nothing to do
    if( I > 0 && T->Child[ I - 1 ]->Count > MINIMUM )
    {
        L = T->Child[ I - 1 ];             // Borrow from the left
        for( J = P->Count; J > 0; --J )
            if( P->Leaf )
                P->Key[ J ] = P->Key[ J - 1 ];   // Open a gap at the front
            else
                P->Child[ J ] = P->Child[ J - 1 ];
        if( P->Leaf )
            P->Key[ 0 ] = L->Key[ L->Count - 1 ];   // Left sibling's last record
        else
            P->Child[ 0 ] = L->Child[ L->Count - 1 ];
        --L->Count;
        ++P->Count;
        Refresh( L );
        Refresh( P );
    }
    else if( I + 1 < T->Count && T->Child[ I + 1 ]->Count > MINIMUM )
    {
        R = T->Child[ I + 1 ];             // Borrow from the right
        if( P->Leaf )
            P->Key[ P->Count ] = R->Key[ 0 ];   // Right sibling's first record
        else
            P->Child[ P->Count ] = R->Child[ 0 ];
        ++P->Count;
        --R->Count;
        for( J = 0; J < R->Count; ++J )
            if( P->Leaf )
                R->Key[ J ] = R->Key[ J + 1 ];   // Close the gap at the front
            else
                R->Child[ J ] = R->Child[ J + 1 ];
        Refresh( P );
        Refresh( R );
    }
    else
    {
        if( I > 0 )                        // Merge into the left sibling
            --I;
        L = T->Child[ I ];
        R = T->Child[ I + 1 ];
        for( J = 0; J < R->Count; ++J )
            if( L->Leaf )
                L->Key[ L->Count + J ] = R->Key[ J ];   // Append the right half
            else
                L->Child[ L->Count + J ] = R->Child[ J ];
        L->Count += R->Count;
        if( L->Leaf )
            L->Next = R->Next;             // Keep the leaf chain intact
        free( R );
        for( J = I + 1; J < T->Count - 1; ++J )
            T->Child[ J ] = T->Child[ J + 1 ];   // Drop the freed child link
        --T->Count;
        Refresh( L );
    }
    Refresh( T );
}

/*
 * Remove one record and repair any underflow on the way back up.
 */
static void
BPlusDeleteNode( ElementType X, BPlusTree T )
{
    int I, J;

    if( T->Leaf )
    {
        for( I = 0; I < T->Count && T->Key[ I ] < X; ++I )
            ;
        if( I == T->Count || T->Key[ I ] != X )
            return;                        // X is not in the tree
        for( J = I; J < T->Count - 1; ++J )
            T->Key[ J ] = T->Key[ J + 1 ]; // Close the gap
        --T->Count;
    }
    else
    {
        I = ChildIndex( X, T );
        BPlusDeleteNode( X, T->Child[ I ] );
        RepairChild( T, I );               // The child may now be underfull
    }
    Refresh( T );
}

/*
 * Delete a key, handling the exceptions that apply only to the root.  
 */
static BPlusTree
BPlusDelete( ElementType X, BPlusTree T )
{
    BPlusTree P;

    if( T == NULL )
        return NULL;
    BPlusDeleteNode( X, T );
    if( !T->Leaf && T->Count == 1 )
    {
        P = T->Child[ 0 ];                 // Root has one child: drop a level
        free( T );
        T = P;
    }
    if( T->Leaf && T->Count == 0 )
    {
        free( T );                         // Last record gone: the tree is empty
        T = NULL;
    }
    return T;
}

/*
 * Release a B+ tree and return NULL.  
 */
static BPlusTree
BPlusMakeEmpty( BPlusTree T )
{
    int I;

    if( T != NULL )
    {
        if( !T->Leaf )
            for( I = 0; I < T->Count; ++I )
                BPlusMakeEmpty( T->Child[ I ] );   // Next is not followed
        free( T );
    }
    return NULL;
}


