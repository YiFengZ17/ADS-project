/* Test driver for five search trees. */
#define _POSIX_C_SOURCE 200809L             // clock_gettime, mkdir
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

typedef int ElementType;

/* Allocate zero-filled memory. */
static void *
NewNode( size_t Size )
{
    void *P = calloc( 1, Size );

    if( P == NULL )
    {
        fprintf( stderr, "Out of space!\n" );
        exit( EXIT_FAILURE );
    }
    return P;
}

/* Tree implementations. */
#include "bst.c"
#include "avl.c"
#include "splay.c"
#include "redblack.c"
#include "bplus.c"




enum { BST, AVL, SPLAY, REDBLACK, BPLUS, TREES };

static const char *Names[] =
{ "BST", "AVL", "Splay", "RedBlack", "BPlus32" };

static const char *Scenarios[] =
{ "increasing_same", "increasing_reverse", "random_random" };

static const int Sizes[] =
{ 1000, 2000, 5000, 10000, 20000, 50000, 100000 };


static uint32_t State;

/* Insert a key and return the root. */
static void *
Insert( int Which, ElementType X, void *T )
{
    switch( Which )
    {
        case BST:      return BstInsert( X, T );
        case AVL:      return AvlInsert( X, T );
        case SPLAY:    return SplayInsert( X, T );
        case REDBLACK: return RedBlackInsert( X, T );
        default:       return BPlusInsert( X, T );
    }
}

/* Delete a key and return the root. */
static void *
Delete( int Which, ElementType X, void *T )
{
    switch( Which )
    {
        case BST:      return BstDelete( X, T );
        case AVL:      return AvlDelete( X, T );
        case SPLAY:    return SplayDelete( X, T );
        case REDBLACK: return RedBlackDelete( X, T );
        default:       return BPlusDelete( X, T );
    }
}

/* Check whether a key exists. */
static int
Find( int Which, ElementType X, void *T )
{
    switch( Which )
    {
        case BST:      return BstFind( X, T );
        case AVL:      return AvlFind( X, T );
        case SPLAY:    return SplayFind( X, T );
        case REDBLACK: return RedBlackFind( X, T );
        default:       return BPlusFind( X, T );
    }
}

/* Free the tree and return NULL. */
static void *
MakeEmpty( int Which, void *T )
{
    switch( Which )
    {
        case BST:      return BstMakeEmpty( T );
        case AVL:      return AvlMakeEmpty( T );
        case SPLAY:    return SplayMakeEmpty( T );
        case REDBLACK: return RedBlackMakeEmpty( T );
        default:       return BPlusMakeEmpty( T );
    }
}

/* Stop if a check fails. */
static void
Require( int Condition, const char *Message, const char *Tree )
{
    if( !Condition )
    {
        fprintf( stderr, "%s: %s\n", Tree, Message );
        exit( EXIT_FAILURE );
    }
}

/* Generate the next xorshift32 value. */
static uint32_t
Random( void )
{
    State ^= State << 13;
    State ^= State >> 17;
    State ^= State << 5;
    return State;
}

/* Generate a random value below Bound. */
static uint32_t
BoundedRandom( uint32_t Bound )
{
    uint32_t R, Threshold = (uint32_t)( -Bound ) % Bound;

    do
        R = Random();
    while( R < Threshold );                // Rejection sampling
    return R % Bound;
}

/* Shuffle the array with Fisher-Yates. */
static void
Shuffle( int *A, int N )
{
    int I, J, Temporary;

    for( I = N - 1; I > 0; --I )
    {
        J = (int)BoundedRandom( (uint32_t)I + 1 );   // Uniform in 0..I
        Temporary = A[ I ];
        A[ I ] = A[ J ];
        A[ J ] = Temporary;
    }
}

/* Read the monotonic clock in seconds. */
static double
Seconds( void )
{
    struct timespec T;

    Require( clock_gettime( CLOCK_MONOTONIC, &T ) == 0, "clock_gettime failed", "driver" );
    return (double)T.tv_sec + (double)T.tv_nsec * 1e-9;
}

/* Check insertion, lookup and deletion. */
static void
Check( void )
{
    enum { N = 1000 };
    int Which, I;
    void *T;

    for( Which = 0; Which < TREES; ++Which )
    {
        T = NULL;
        T = Delete( Which, 0, T );              // Deleting from empty is legal
        T = Insert( Which, INT_MIN, T );
        T = Insert( Which, INT_MAX, T );

        /* Test increasing keys. */
        for( I = 0; I < N; ++I )
            T = Insert( Which, I * 2, T );

        Require( Find( Which, INT_MIN, T ) && Find( Which, INT_MAX, T ),
                 "boundary key was not stored", Names[ Which ] );
        for( I = 0; I < N; I += 97 )
            Require( Find( Which, I * 2, T ), "inserted key not found", Names[ Which ] );
        Require( !Find( Which, 1, T ), "absent key reported as present", Names[ Which ] );

        for( I = N - 1; I >= 0; --I )      // Reverse deletion
            T = Delete( Which, I * 2, T );
        T = Delete( Which, INT_MIN, T );
        T = Delete( Which, INT_MAX, T );
        Require( T == NULL, "tree not empty after deleting every key", Names[ Which ] );

        printf( "%s: PASS\n", Names[ Which ] );
    }
}

/* Save the input orders and seed. */
static void
WriteInputs( const char *Directory, int N, int Scenario, int Repeat, uint32_t Seed,
             const int *InsertOrder, const int *DeleteOrder )
{
    char Path[ 512 ];
    FILE *F;
    int I;

    Require( snprintf( Path, sizeof( Path ), "%s/%s_%d_r%d.txt", Directory,
                      Scenarios[ Scenario ], N, Repeat ) < (int)sizeof( Path ),
             "input path too long", "driver" );
    F = fopen( Path, "w" );
    Require( F != NULL, "cannot write input file", "driver" );
    fprintf( F, "# N=%d scenario=%s repeat=%d seed=%u\n", N, Scenarios[ Scenario ], Repeat, Seed );
    fprintf( F, "%d\n", N );
    for( I = 0; I < N; ++I )
        fprintf( F, "%d%c", InsertOrder[ I ], I + 1 == N ? '\n' : ' ' );   // Insertion order
    for( I = 0; I < N; ++I )
        fprintf( F, "%d%c", DeleteOrder[ I ], I + 1 == N ? '\n' : ' ' );   // Deletion order
    Require( fclose( F ) == 0, "input close failed", "driver" );
}

/* Time insertion and deletion; save CSV results. */
static void
Benchmark( const char *CsvPath, const char *InputDirectory, int Repeats )
{
    FILE *Csv = fopen( CsvPath, "w" );
    size_t S;
    int Scenario, Repeat, N, I, Offset, K, Which;
    int *Insertion, *Deletion;
    uint32_t Seed;
    void *T;
    double Start, InsertTime, DeleteTime;

    Require( Csv != NULL, "cannot open CSV", "driver" );
    Require( mkdir( InputDirectory, 0755 ) == 0 || errno == EEXIST,
             "cannot create input directory", "driver" );
    fprintf( Csv, "scenario,n,repeat,seed,tree,insert_seconds,delete_seconds,total_seconds\n" );

    /* Warm up before timing. */
    for( Which = 0; Which < TREES; ++Which )
    {
        T = NULL;
        for( I = 0; I < 256; ++I )
            T = Insert( Which, I, T );
        for( I = 0; I < 256; ++I )
            T = Delete( Which, I, T );
        Require( T == NULL, "warmup failure", Names[ Which ] );
    }

    for( S = 0; S < sizeof( Sizes ) / sizeof( Sizes[ 0 ] ); ++S )
    {
        N = Sizes[ S ];
        Insertion = NewNode( (size_t)N * sizeof( *Insertion ) );   // Shared input arrays
        Deletion = NewNode( (size_t)N * sizeof( *Deletion ) );
        for( Scenario = 0; Scenario < 3; ++Scenario )
        for( Repeat = 0; Repeat < Repeats; ++Repeat )
        {
            Seed = 0x9e3779b9u ^ (uint32_t)N ^ (uint32_t)( Repeat + 1 ) * 0x85ebca6bu;
            State = Seed;
            for( I = 0; I < N; ++I )
            {
                Insertion[ I ] = I;        // Scenario 0: both increasing
                Deletion[ I ] = I;
            }
            if( Scenario == 1 )
                for( I = 0; I < N; ++I )
                    Deletion[ I ] = N - 1 - I;   // Scenario 1: reverse deletion
            if( Scenario == 2 )
            {
                Shuffle( Insertion, N );   // Scenario 2: independent shuffles
                Shuffle( Deletion, N );
            }
            WriteInputs( InputDirectory, N, Scenario, Repeat, Seed, Insertion, Deletion );

            Offset = ( Repeat + Scenario + (int)S ) % TREES;
            for( K = 0; K < TREES; ++K )
            {
                Which = ( K + Offset ) % TREES;   // Rotate tree order
                T = NULL;
                Start = Seconds();
                for( I = 0; I < N; ++I )
                    T = Insert( Which, Insertion[ I ], T );
                InsertTime = Seconds() - Start;

                /* Check sampled keys outside timing. */
                for( I = 0; I < 256; ++I )
                    Require( Find( Which, Insertion[ (size_t)I * N / 256 ], T ),
                             "benchmark key missing", Names[ Which ] );

                Start = Seconds();
                for( I = 0; I < N; ++I )
                    T = Delete( Which, Deletion[ I ], T );
                DeleteTime = Seconds() - Start;

                Require( T == NULL, "tree not empty after deletion phase", Names[ Which ] );
                fprintf( Csv, "%s,%d,%d,%u,%s,%.9f,%.9f,%.9f\n", Scenarios[ Scenario ], N,
                         Repeat, Seed, Names[ Which ], InsertTime, DeleteTime, InsertTime + DeleteTime );
                fflush( Csv );             // Keep a partial run readable
                T = MakeEmpty( Which, T );
            }
            fprintf( stderr, "Completed N=%d %s repetition=%d\n", N, Scenarios[ Scenario ], Repeat );
        }
        free( Insertion );
        free( Deletion );
    }
    Require( fclose( Csv ) == 0, "CSV close failed", "driver" );
}

/* Run the check or benchmark. */
int
main( int Argc, char **Argv )
{
    if( Argc == 2 && strcmp( Argv[ 1 ], "--check" ) == 0 )
        Check();
    else if( Argc == 4 && strcmp( Argv[ 1 ], "--bench" ) == 0 )
        Benchmark( Argv[ 2 ], Argv[ 3 ], 3 );   // Three repetitions per trial
    else
    {
        fprintf( stderr, "Usage: %s --check | --bench results.csv inputs\n", Argv[ 0 ] );
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
