#ifndef LCN_UTIL_H
#define LCN_UTIL_H


#include "lucene.h"

BEGIN_C_DECLS

/**
 * @defgroup lcn_util Util
 * @ingroup lucene
 * @{
 */

typedef struct lcn_ptr_array_t
{
    void** arr;
    unsigned int length;

} lcn_ptr_array_t;

typedef struct lcn_byte_array_t
{
    lcn_byte_t* arr;
    unsigned int length;

} lcn_byte_array_t;

typedef struct lcn_int_array_t
{
    int* arr;
    unsigned int length;

} lcn_int_array_t;

typedef struct lcn_float_array_t
{
    float* arr;
    unsigned int length;

} lcn_float_array_t;

typedef struct lcn_size_array_t
{
    unsigned int* arr;
    unsigned int length;

} lcn_size_array_t;


/**
 * @defgroup lcn_list List
 * @ingroup lcn_util
 * @{
 */

//typedef struct lcn_list_t lcn_list_t;

apr_status_t
lcn_list_sort_cstrings( lcn_list_t *list );

void*
lcn_list_get( const lcn_list_t *list,
              unsigned int nth );

apr_status_t
lcn_list_uniquify( lcn_list_t **new_list,
                   lcn_list_t *list,
                   apr_pool_t* pool );

void
lcn_list_set( lcn_list_t *list,  unsigned int n, void *element );

apr_status_t
lcn_list_add( lcn_list_t *list, void *element );

apr_status_t
lcn_list_insert( lcn_list_t *list, unsigned int pos, void* element );

void
lcn_list_remove( lcn_list_t *list, unsigned int n );

apr_status_t
lcn_list_create( lcn_list_t **new_list,
                 unsigned int size,
                 apr_pool_t *pool );

apr_status_t
lcn_list_swap( lcn_list_t *list,
               unsigned int a,
               unsigned int b );

apr_status_t
lcn_list_append( lcn_list_t *list, lcn_list_t *list_to_add );


unsigned int
lcn_list_size( const lcn_list_t *list );

/**
 * @brief Deletes all elements of the list, setting the size of the
 *        list to 0. The list can be then reused.
 *
 * @param list  underlying list
 */
void
lcn_list_clear( lcn_list_t *list );

apr_pool_t *
lcn_list_pool( lcn_list_t *list );

apr_status_t
lcn_list_sort( lcn_list_t* list,
               int(*sort_func)(const void* a, const void* b ) );

/**
 * @brief Creates a clone of a given list
 *
 * @param list the list to clone
 * @param clone Pointer to the result
 * @param element_clone_func If a function pointer is provided,
 *        it is used to perform deep copy on each list element. Can be NULL
 * @param pool Memory Context for the cloned list
 * @result LCN-Status-Value
 */

apr_status_t
lcn_list_clone( lcn_list_t* list,
                lcn_list_t** clone,
                void* (*element_clone_func)( const void*, apr_pool_t* ),
                apr_pool_t* pool );

/**
 * @brief Returns last element of the list
 *
 * @oaram list list-object
 *
 * @return last element of the list
 */
void*
lcn_list_last( lcn_list_t* list );

/** @} */

/**
 * @defgroup lcn_linked_list Linked List
 * @ingroup lcn_util
 * @{
 */

typedef struct lcn_linked_list_t lcn_linked_list_t;
typedef struct lcn_linked_list_el_t lcn_linked_list_el_t;


apr_status_t
lcn_linked_list_create( lcn_linked_list_t** linked_list, apr_pool_t* pool );

void
lcn_linked_list_clear( lcn_linked_list_t* list );

unsigned int
lcn_linked_list_size( lcn_linked_list_t* l );

apr_status_t
lcn_linked_list_add_last( lcn_linked_list_t* linked_list,
                          void* obj );

apr_status_t
lcn_linked_list_add_first( lcn_linked_list_t* linked_list,
                           void* obj );

/**
 * @brief Remove first element of the linked list
 *
 * @param linked_list  Linked list
 */
const lcn_linked_list_el_t*
lcn_linked_list_remove_first( lcn_linked_list_t* linked_list );

/**
 * @brief Removes last element of the linked list
 *
 * @param linked_list  Linked list
 */
const lcn_linked_list_el_t*
lcn_linked_list_remove_last( lcn_linked_list_t* linked_list );

/**
 * @brief Removes given element of the list
 *
 * @param l   Linked List
 * @param el  Element to remove
 */
const lcn_linked_list_el_t*
lcn_linked_list_remove_element( lcn_linked_list_t* l,
                                const lcn_linked_list_el_t *el );

/**
 * @brief First element of the linked list
 *
 * @param linked_list  Linked list
 */
const lcn_linked_list_el_t*
lcn_linked_list_first( lcn_linked_list_t* linked_list );

/**
 * @brief Last element of the linked list
 *
 * @param linked_list  Linked List
 */
const lcn_linked_list_el_t*
lcn_linked_list_last( lcn_linked_list_t* linked_list );

/**
 * @brief Next element of the linked list
 *
 * @param linked_list_element  Current element
 */
const lcn_linked_list_el_t*
lcn_linked_list_next( const lcn_linked_list_el_t* linked_list_element );

/**
 * @brief Content of the linked list element
 *
 * @param list_element  List element
 */
void*
lcn_linked_list_content( const lcn_linked_list_el_t* list_element );


/**
 * @brief Copies the contents of the linked list to a
 *        new pointer array
 *
 * @param l      Linked list
 * @param array  new array
 * @param pool   APR pool
 */
apr_status_t
lcn_linked_list_to_array( lcn_linked_list_t* l,
                          lcn_ptr_array_t** array,
                          apr_pool_t* pool );

lcn_linked_list_el_t*
lcn_linked_list_get_element(lcn_linked_list_t* l,unsigned int nth);


/** @} */

/**
 * @defgroup lcn_ptr_array Array
 * @ingroup lcn_util
 * @{
 */

/**
 * @brief Create a lcn_array to store pointers (void*)
 *
 * @param array   Stores the new array
 * @param length  Size of the new array
 * @param pool    APR pool
 */
apr_status_t
lcn_ptr_array_create( lcn_ptr_array_t** array,
                      unsigned int length,
                      apr_pool_t* pool );

/**
 * @brief Create a lcn_array to store lcn_byte_t
 *
 * @param array   Stores the new array
 * @param length  Size of the new array
 * @param pool    APR pool
 */
apr_status_t
lcn_byte_array_create( lcn_byte_array_t** array,
                       unsigned int length,
                       apr_pool_t* pool );

/**
 * @brief Create an int array of fixed size.
 *
 * The contents of the array are not nulled.
 *
 * @param array   Stores the new array
 * @param length  Size of the new array
 * @param pool    APR pool
 */
apr_status_t
lcn_int_array_create( lcn_int_array_t** array,
                      unsigned int length,
                      apr_pool_t* pool );

/**
 * @brief Create a zeroed int array of fixed size.
 *
 * The contents of the array are filled with zero.
 *
 * @param array   Stores the new array
 * @param length  Size of the new array
 * @param pool    APR pool
 */
apr_status_t
lcn_zero_int_array_create( lcn_int_array_t** array,
                           unsigned int length,
                           apr_pool_t* pool );

/**
 * @brief Create a float array of fixed size
 *
 * @param array   Stores resulting array
 * @param length  Length of array
 * @param pool    APR pool
 */
apr_status_t
lcn_float_array_create( lcn_float_array_t** array,
                        unsigned int length,
                        apr_pool_t* pool );
/**
 * @brief Creates lcn_array for storing unsigned int
 *
 * @param array   Stores resulting array
 * @param length  Length of array
 * @param pool    APR-Pool
 */
apr_status_t
lcn_size_array_create( lcn_size_array_t** array,
                       unsigned int length,
                       apr_pool_t* pool );

/** @} */

/**
 * @defgroup lcn_string_buffer String-Buffer
 * @ingroup lcn_util
 * @{
 */

typedef struct lcn_string_buffer_t lcn_string_buffer_t;

/**
 * @brief Creates new string buffer
 *
 * @param string_buffer  Stores the newly created buffer
 * @param pool           APR-Pool
 */
apr_status_t
lcn_string_buffer_create( lcn_string_buffer_t** string_buffer,
                          apr_pool_t* pool );

/**
 * @brief Appends string to string buffer
 *
 * @param string_buffer  Underlying buffer
 * @param string         String to append
 */
apr_status_t
lcn_string_buffer_append( lcn_string_buffer_t* string_buffer,
                          const char* string );

/**
 * @brief Appends n leading characters of the string to
 *        string buffer
 *
 * If the length of the string is less than n, the the complete
 * string is appended and the function successfully returns.
 *
 * @param string_buffer  Underlying buffer
 * @param string         String to append
 * @param n              Number of characters to append
 */
apr_status_t
lcn_string_buffer_nappend( lcn_string_buffer_t* string_buffer,
                           const char* string,
                           unsigned int n );
/**
 * @brief Converts a float to a string and appends to the
 *        string buffer
 *
 * @param string_buffer  Underlying buffer
 * @param f              Float to append
 */
apr_status_t
lcn_string_buffer_append_float( lcn_string_buffer_t* string_buffer,
                                float f );

/**
 * @brief Converts an integer to a string and apppends to the
 *        string buffer
 *
 * @param string_buffer  Underlying buffer
 * @param i              Integer to append
 */
apr_status_t
lcn_string_buffer_append_int( lcn_string_buffer_t* string_buffer,
                              int i );

/**
 * @brief Converts an unsigned integer to a string and
 *        appends to the string_buffer
 *
 * @param string_buffer  Underlying buffer
 * @param i              Integer to append
 */
apr_status_t
lcn_string_buffer_append_uint( lcn_string_buffer_t* string_buffer,
                               unsigned int i );

/**
 * @brief Appends the contents of the buffer to_add to the
 *        string_buffer
 *
 * @param string_buffer  Underlying buffer
 * @param to_add         Buffer to append
 */
apr_status_t
lcn_string_buffer_append_buffer( lcn_string_buffer_t* string_buffer,
                                 lcn_string_buffer_t* to_add );

/**
 * @brief Append formatted string to the buffer
 *
 * @param string_buffer  Underlying buffer
 * @param fmt            Format string
 */
apr_status_t
lcn_string_buffer_append_format( lcn_string_buffer_t* string_buffer,
                                 const char* fmt, ... );

/**
 * @brief Creates new string with the contents of the buffer
 *
 * @param string_buffer Underlying buffer
 * @param string        Stores new string
 * @param pool          APR-Pool of the string
 */
apr_status_t
lcn_string_buffer_to_string( lcn_string_buffer_t* string_buffer,
                             char** string,
                             apr_pool_t* pool );

/**
 * @brief Length of the buffer
 *
 * @param string_buffer  Underlying buffer
 *
 * @return Length of the buffer
 */
unsigned int
lcn_string_buffer_length( const lcn_string_buffer_t* string_buffer );

/**
 * @brief Last character of the buffer
 *
 * @param sb  Underlying buffer
 *
 * @return    Last character of the buffer
 */
char
lcn_string_buffer_last_char( const lcn_string_buffer_t* sb );

/**
 * @brief Character at the position i of the buffer
 *
 * @param sb  Underlying buffer
 * @param i   Index of the character
 *
 * @return    Character at the position i of the buffer or 0 if
 *            the position is invalid.
 */
char
lcn_string_buffer_char_at( const lcn_string_buffer_t* sb, unsigned int i );

/**
 * @brief APR-Pool of the string_buffer
 *
 * @param buf  Underlying buffer
 * @return     APR-Pool
 */
apr_pool_t*
lcn_string_buffer_pool( lcn_string_buffer_t* buf );

/** @} */

/**
 * @defgroup lcn_priority_queue Priority-Queue
 * @ingroup lcn_util
 * @{
 */

typedef struct lcn_priority_queue_t lcn_priority_queue_t;



/**
 * A lcn_priority_queue_t maintains a partial ordering of its elements such that the
 * least element can always be found in constant time.  Put()'s and pop()'s
 * require log(size) time.
 *
 * An example of subclassing lcn_priority_queue_t:
 *
 * \code
 * lcn_priority_queue_t *
 * lcn_some_queue_create( int size )
 * {
 *     lcn_priority_queue_t *queue = create_lcn_priority_queue_t();
 *
 *     if ( queue )
 *     {
 *         queue->initialize( queue, size );
 *     }
 *
 *     // my_less_than_function depends on the objects
 *     // to be stored in the queue
 *     queue->less_than = my_less_than_function
 *
 *     return queue;
 * }
 * \endcode
 **/

struct lcn_priority_queue_t
{
    /**
     * Function to use while comparing elements of the queue.
     * Should be initialized by subclassing code.
     */
    lcn_bool_t (*less_than ) (struct lcn_priority_queue_t*, void *a, void *b);

    /**
     * Heap for storing the queued Objects
     */
    void **heap;

    /**
     * Current size of queue
     */
    size_t size;

    /**
     * Maximal possible size of queue
     */
    size_t max_size;

    /**
     * APR pools of the queue
     */
    apr_pool_t* pool;
    apr_pool_t* heap_pool;
};


/**
 * @brief Subclass constructors must call this.
 *
 * @param queue  Queue to initialize
 * @param max_size maximal size of queue heap
 */
apr_status_t
lcn_priority_queue_initialize( lcn_priority_queue_t *queue,
                               unsigned int max_size );
/**
 * @brief Current size of queue
 */
unsigned int
lcn_priority_queue_size( lcn_priority_queue_t* queue );

/**
 * @brief Maximal possible size of queue
 */
unsigned int
lcn_priority_queue_max_size( lcn_priority_queue_t* queue );

/**
 * @brief  Should be called when the Object at top changes values.
 *
 * Still log(n) worst case, but it's at least twice as fast to <pre>
 *  { pq.top().change(); pq.adjustTop(); }
 * </pre> instead of <pre>
 *  { o = pq.pop(); o.change(); pq.push(o); }
 * </pre>
 */
void
lcn_priority_queue_adjust_top( lcn_priority_queue_t* queue );

/**
 * @brief Adds an Object to a lcn_priority_queue_t in log(size) time.
 *
 * TODO: Must set errno and check it in client code
 */
apr_status_t
lcn_priority_queue_put( lcn_priority_queue_t *, void *element );

/**
 * Adds element to the lcn_priority_queue_t in log(size) time if either
 * the lcn_priority_queue_t is not full, or not lessThan(element, top()).
 *
 * @return Lucene-Statusvalue
 */
void *
lcn_priority_queue_insert( lcn_priority_queue_t *, void *);

/**
 * @brief Returns the least element of the #lcn_priority_queue_t
 *        in constant time and removes it from the queue
 *
 * @param  queue  The queue to be used
 */
void *
lcn_priority_queue_pop( lcn_priority_queue_t *queue );

/**
 * @brief Returns the least element of the #lcn_priority_queue_t
 *        in constant time.
 *
 * @param queue  The queue to be used
 */
void *
lcn_priority_queue_top( lcn_priority_queue_t *queue );

/**
 * @brief Returns the greatest (max) element of the #lcn_priority_queue_t
 *        in linear time
 *
 * @param queue  The queue to be used
 */
void *
lcn_priority_queue_max( lcn_priority_queue_t *queue );

/**
 * @brief Removes all entries from the lcn_priority_queue_t.
 *
 * @param queue  The queue to be cleared
 */
void
lcn_priority_queue_clear( lcn_priority_queue_t* queue);

/**
 * @brief Returns nth element of the internal heap. There is
 *        no error checking.
 *
 * @param queue  The queue to be used
 * @param n      Index of element to get
 */
void*
lcn_priority_queue_element_at( lcn_priority_queue_t* queue, unsigned int n );

/**
 * @brief Sets the nth element of the internal heap. There is
 *        no error checking. Use for copying only!
 *
 * @param queue    The queue to be used
 * @param element  element to set
 * @param n        Index of element to set
 */
void
lcn_priority_queue_set_element_at( lcn_priority_queue_t* queue,
                                   void *element,
                                   unsigned int n );

/**
 * @brief Sets new max size of the priority_queue. The current status of the
 *        queue is (size and contents) do not change
 *
 * @param queue    The queue to be used
 * @param new_size New max size of the queue
 */
apr_status_t
lcn_priority_queue_resize( lcn_priority_queue_t *priority_queue,
                           size_t new_size );


/** @} */

/**
 * Determines the ordering of objects in this priority queue.
 *  Subclasses must define this one method.
 *
 * a < b  => return TRUE
 * a > b  => return FALSE
 **/

bool lcn_priority_queue_less_than(void *a, void *b);

apr_status_t
lcn_priority_queue_create( lcn_priority_queue_t** priority_queue,
                           lcn_bool_t (*less_than)( lcn_priority_queue_t*, void*, void* ),
                           apr_pool_t* pool );


lcn_byte_t
lcn_smallfloat_float_to_byte315( float f );

float
lcn_smallfloat_byte315_to_float( lcn_byte_t b );



void
lcn_to_string_boost( char* buf, unsigned int buf_len, float boost );

#define LCN_IS_CHAR(c)  (                                \
                          ( c >= 'a' && c <= 'z' ) ||    \
                          ( c >= 'A' && c <= 'Z' ) ||    \
                          c == '\344' ||                 \
                          c == '\374' ||                 \
                          c == '\366' ||		 \
			  c == '\304' ||                 \
                          c == '\334' ||                 \
                          c == '\326' ||                 \
                          c == '\337' ||                 \
                          c == '\341' ||                 \
                          c == '\351' )

#define LCN_IS_UPPER(c) ( ( c >= 'A' && c <= 'Z' ) || c == 'Ä' || c == 'Ö' || c == 'Ü' )

#define LCN_IS_DIGIT(c) ( c >= '0' && c <= '9' )

extern unsigned char lcn_upper_to_lower_map[];

/**
 * Converts an integer to the numeric representation
 * to base 32. Up to 7 characters may be written to
 * buffer
 */
void
lcn_itoa32( unsigned int i, char *buf );

/**
 * Converts an integer to the numeric representation
 * to base 36. Up to 7 characters may be written to
 * buffer
 */
void
lcn_itoa36( apr_int64_t i, char *buf );


/**
 * Converts a numeric representation to base 32 to an
 * integer. At most first 7 digits are evaluated.
 */
unsigned int
lcn_atoi32( const char *buf );

/**
 * Converts a numeric representation to base 32 to an
 * integer. At most first 7 digits are evaluated.
 */
unsigned int
lcn_atoi36( const char *buf );

/**
 * Compares two strings, character by character, and returns the
 * first position where the two strings differ from one another.
 *
 * @param s1 The first string to compare
 * @param s2 The second string to compare
 * @return The first position where the two strings differ.
 */
int
lcn_string_difference( const char *s1, const char *s2 );

/**
 * @brief Checks, if <code>haystack</code> starts with <code>needle</code>
 *
 * @result LCN_TRUE if haystack starts with needle, LCN_FALSE otherwise
 */

lcn_bool_t
lcn_string_starts_with( const char* haystack, const char* needle );

/**
 * @brief Converts all letters in the string to lowercase
 *
 * @param string String to convert to lowercase
 * @result Pointer to the parameter for convenient use as a function
 */

char*
lcn_string_to_lower( char* string );

apr_status_t
lcn_string_unescape( char** result,
                     const char* input,
                     const char* chars_to_escape,
                     char escape_char,
                     apr_pool_t* pool );

apr_status_t
lcn_string_escape( char** result,
                   const char* input,
                   const char* chars_to_escape,
                   char escape_char,
                   apr_pool_t* pool );


const char*
lcn_string_next_unescaped( const char* input, char c, char escape_char );

char*
lcn_string_purge_whitespaces( char* str );

apr_status_t
lcn_string_from_file( char** result,
                      const char* path,
                      unsigned int* length,
                      apr_pool_t* pool );

lcn_bool_t
lcn_string_is_digit( char *str );

char
lcn_string_char_at( const char *str, unsigned int index );

/** @} **/
END_C_DECLS

#endif /* LCN_UTIL_H */


