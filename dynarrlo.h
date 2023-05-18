//
// Created by easy on 14.05.23.
//

#ifndef EASY_DYNARRLO_H
#define EASY_DYNARRLO_H

#include <stddef.h>

///
/// Every DynarrLO object assumes at an absolute minimum this capacity.
#define DAL_MIN_CAPACITY 2

#ifndef DAL_PRIMITIVE_SUPPORT
///
/// Evaluates true if this primitive data type usage is supported.
#define DAL_PRIMITIVE_SUPPORT (__SIZEOF_SIZE_T__ == __SIZEOF_POINTER__)
#endif

/**
 * DAL_ERROR contains every error code that the error flag of a DynarrLO object
 * may assume. The codes are listed by increasing severity. Every function that
 * may change the error flag will automatically set it to DAL_OK if the function
 * succeeded. The value of DAL_OK is equal to 0, indicating success. Any
 * non-zero value indicates an error.
 */
typedef enum DAL_ERROR {
    DAL_OK,
    DAL_OUTOFRANGE,
    DAL_NULLARG,
    DAL_ALLOCFAIL
} DAL_ERROR;


/**
 * DynarrLO is a dynamic array implementation written in C, conforming to at
 * least the C11 and C17 standards. The only non-C99 feature (that I know of) is
 * the usage of an unnamed union inside the struct definition of DynarrLO.
 * No effort has been made to support C++.\n\n
 *
 * The LO stands for low overhead. DynarrLO does the bare minimum to function as
 * a convenient dynamic array without sacrificing safety.\n
 * It provides a great amount of flexibility with its functions that are
 * focussed on key functionality of a dynamic array and on the most performant
 * operations its implementation can perform.\n\n
 *
 * The implementation tries to do many things in a mostly branchless manner.
 * Conditional jumps are used sparingly and conditional moves are preferred to
 * lower the risk of pipeline stalls. Some important key functions that are
 * entirely branchless when compiling on amd64 Linux gcc 13.1 with optimisation
 * enabled are \p dal_write() , \p dal_get() , \p dal_setLength() and
 * \p dal_pop() . I have been mindful to pay attention to cache efficiency
 * as well.\n\n
 *
 * DynarrLO does not allocate heap memory on a whim and only uses it very
 * sparingly i.e. when it is needed for array growth or when the programmer
 * explicitly tells it to.\n
 * On top of that, memory (de-)allocation is done exclusively by the functions
 * that the programmer provides the DynarrLO.\n
 * It merely needs a \p realloc() and a \p free() function that conform to the C
 * standard to do its job.\n\n
 *
 * DynarrLO uses a growth factor of 1.5 and does \a not automatically shrink the
 * memory back down if there is slack space at the end of the array. This is
 * done for performance reasons and the fact that in most use cases, that unused
 * space at the end will be used again in the near future, in which case freeing
 * it would fragment memory and cause unnecessary memory operations. If memory
 * should be freed regardless, you may use \p dal_setCapacity() to set the
 * capacity to a value you are comfortable with.\n\n
 *
 * DynarrLO supports the usage of primitive data types as array elements instead
 * of generic void pointers. This is useful to avoid memory wastage. The type
 * used for primitive operations and storage is size_t.\n
 * DynarrLO is automatically compiled with primitive support if
 * \p sizeof(size_t) equals \p sizeof(void *) on the current compiler.\n
 * To check for primitive support, you may use the \p DAL_PRIMITIVE_SUPPORT
 * macro which evaluates to true if support is enabled. You may manually disable
 * primitive support by defining \p DAL_PRIMITIVE_SUPPORT as 0/false before
 * including this header. This would also (trivially) resolve the issue with
 * the unnamed union inside the DynarrLO struct, which breaks C99 conformance.
 * \n\n
 *
 * DynarrLO only works with a few very basic functions of the C standard library
 * and with nothing else. The source file includes stdbool.h and string.h.
 * Additionally, the header includes stddef.h.\n\n
 *
 * It is recommended not to modify the contents of the array or the array itself
 * or the length and capacity fields manually and instead use the functions
 * provided by the library. Modifying the error flag is fine.\n\n
 *
 * \b DO \b NOT reassign different functions to the internal \p realloc() and
 * \p free() functions after executing \p dal_createDynarrLO() to create a
 * DynarrLO object. Doing so \b will \b definitely cause all kinds of erratic
 * behaviour and possibly crash your system.
 */
typedef struct DynarrLO {
#if DAL_PRIMITIVE_SUPPORT
    union {
        void **array;   ///< Generic void pointer array.
        size_t *arrayp; ///< Primitive data type size_t array.
    };
#else
    void **array;       ///< Generic void pointer array.
#endif

    size_t length;      ///< Current amount of elements in this DynarrLO.
    size_t capacity;    ///< Allocated memory for DynarrLO array in elements.
    DAL_ERROR error;    ///< Error flag.

    ///
    /// A \p realloc() function conforming to the C standard.
    void *(*realloc) (void *ptr, size_t size);
    ///
    /// A \p free() function conforming to the C standard.
    void  (*free)    (void *ptr);
} DynarrLO;


/**
 * Tries to create and allocate a dynamic array.
 * @param d Pointer to DynarrLO object that shall be initialised.
 * @param capacity Desired starting capacity.
 * @param realloc realloc function conforming to the C standard.
 * @param free free function conforming to the C standard.
 * @return Error code indicating either success, a null argument error or
 * failure to allocate the requested amount of memory.
 */
DAL_ERROR dal_createDynarrLO(DynarrLO *d,
                             size_t capacity,
                             void *(*realloc) (void *, size_t),
                             void (*free) (void *));

/**
 * Frees its array and sets all struct fields of this DynarrLO to 0. Elements
 * residing in the array are not automatically freed. This needs to be done
 * manually beforehand.
 */
void dal_destroyDynarrLO(DynarrLO *d);

/**
 * Zeroes out the memory in the given range of the array.
 * Sets error flag to DAL_OUTOFRANGE if any index is out of range, else DAL_OK.
 * @param iStart Index at which zeroing begins (inclusive). Automatically set to
 * the final value of \a iEnd if it exceeds the final value of \a iEnd.
 * @param iEnd Index at which zeroing ends (exclusive). Automatically set to
 * capacity if it exceeds the value of capacity.
 */
void dal_zeroOut(DynarrLO *d,
                 size_t iStart,
                 size_t iEnd);

/**
 * Unconditionally sets the capacity to \a capacity, cutting off excess
 * elements if \a capacity subceeds the current capacity. Does not change
 * the contents of the array barring this exception.
 *
 * Sets error flag to \b DAL_ALLOCFAIL if memory couldn't be allocated.
 * @param capacity Desired capacity.
 */
void dal_setCapacity(DynarrLO *d, size_t capacity);

/**
 * Length is set to \a length or to the capacity of the array if \a length
 * exceeds the bounds of the array, in which case the error flag is set to
 * \b DAL_OUTOFRANGE.
 * @param length Desired length.
 */
void dal_setLength(DynarrLO *d, size_t length);

/**
 * Writes an object into the array. Does nothing if \a index >= capacity.
 * Sets error flag to \b DAL_OUTOFRANGE if \a index >= length.
 * @param index Index to overwrite.
 * @param obj Object that shall be written at the \a index.
 */
void dal_write(DynarrLO *d,
               size_t index,
               void *obj);

/**
 * Allocates an object and writes it into the array. Does nothing if \a index
 * >= capacity. Sets error flag to \b DAL_OUTOFRANGE \a index >= length or to
 * \b DAL_ALLOCFAIL if memory couldn't be allocated.
 * @param index Index to overwrite.
 * @param size Size of the object to allocate.
 * @return Pointer to object if successful. NULL if either \b DAL_ALLOCFAIL
 * occurred or \a index >= capacity.
 */
void *dal_writeInst(DynarrLO *d,
                    size_t index,
                    size_t size);

/**
 * Appends an object to the back of the array. Grows the array if needed. Error
 * flag is set to \b DAL_ALLOCFAIL if memory couldn't be allocated.
 * @param obj Object to append.
 */
void dal_append(DynarrLO *d, void *obj);

/**
 * Allocates and appends an object to the back of the array. Grows the array
 * if needed. Error flag is set to \b DAL_ALLOCFAIL if memory couldn't be
 * allocated.
 * @param size Size of the object to allocate.
 * @return Pointer to object if successful. NULL on failure.
 */
void *dal_appendInst(DynarrLO *d, size_t size);

/**
 * Inserts an element at \a index shifting all elements starting at \a index
 * one to the right before doing so. Grows array if needed. If \a index ==
 * length, behaviour is identical to \p dal_append().
 *
 * Error flag is set to \b DAL_OUTOFRANGE if \a index > length or to
 * \b DAL_ALLOCFAIL if memory couldn't be allocated.
 * @param index Index object should assume.
 * @param obj Object to insert.
 */
void dal_insert(DynarrLO *d,
                size_t index,
                void *obj);

/**
 * Same as \p dal_insert() , but inserts many objects at once, thereby shifting
 * elements starting at \a index by \a num to the right.
 * @param index Index first object should assume.
 * @param objs Array of objects to insert.
 * @param num Number of objects to insert.
 */
void dal_insertMany(DynarrLO *d,
                    size_t index,
                    void **objs,
                    size_t num);

/**
 * Gets the element at \a index. Error flag is set to \b DAL_OUTOFRANGE if
 * \a index >= length.
 * @param index Index to access.
 * @return Element at \a index or NULL if \a >= capacity.
 */
void *dal_get(DynarrLO *d, size_t index);

/**
 * Removes the hindmost element and returns it. Error flag is set to
 * \b DAL_OUTOFRANGE if array is empty.
 * @return Hindmost element or NULL if array is empty.
 */
void *dal_pop(DynarrLO *d);

/**
 * Calls \p free() on the object at \a index and then overwrites that element
 * with NULL. The element is not removed from the array. Does nothing if
 * \a index >= capacity. Error flag is set to \b DAL_OUTOFRANGE if \a index >=
 * length.
 * @param index Index of object to free.
 */
void dal_freeItem(DynarrLO *d, size_t index);

/**
 * Calls \p free() on many consecutive objects and overwrites each element with
 * NULL afterwards. The elements are not removed from the array. Does nothing if
 * index range is out of array bounds. Error flag is set to \b DAL_OUTOFRANGE if
 * \a iStart >= length or iEnd > length.
 * @param iStart Index at which freeing begins (inclusive).
 * @param iEnd Index at which freeing ends (exclusive). Automatically set to
 * capacity if it exceeds the value of capacity.
 */
void dal_freeItems(DynarrLO *d,
                   size_t iStart,
                   size_t iEnd);

/**
 * Calls \p free() on the last object, assigns NULL to the element and removes
 * it. Error flag is set to DAL_OUTOFRANGE if array is empty.
 */
void dal_fremoveLast(DynarrLO *d);

/**
 * Removes the last element.
 * Error flag is set to DAL_OUTOFRANGE if array is empty.
 */
void dal_removeLast(DynarrLO *d);

/**
 * Removes multiple elements from the back of the array.
 * Error flag is set to DAL_OUTOFRANGE if \a amount > length.
 * @param amount Number of elements to remove from back.
 */
void dal_removeLastMany(DynarrLO *d, size_t amount);

/**
 * Removes the element at \a index, thereby shifting all elements after it
 * by one place to the left. Does nothing if \a index >= length, in which case
 * error flag is set to DAL_OUTOFRANGE.
 * @param index Index of element to remove.
 */
void dal_remove(DynarrLO *d, size_t index);

/**
 * Removes multiple elements starting at \a iStart, thereby shifting all
 * elements starting at \a iEnd to the left, so that the element at \a iEnd will
 * then be located at index \a iStart. Error flag is set to \b DAL_OUTOFRANGE if
 * \a iStart >= length or \a iEnd > length.
 * @param iStart Index at which removal begins (inclusive). Automatically set to
 * the final value of \a iEnd if it exceeds the final value of \a iEnd.
 * @param iEnd Index at which removal ends (exclusive). Automatically set to
 * length if it exceeds the value of length.
 */
void dal_removeMany(DynarrLO *d,
                    size_t iStart,
                    size_t iEnd);


#if DAL_PRIMITIVE_SUPPORT

/**
 * Primitive version of \p dal_write().
 */
void dal_pwrite(DynarrLO *d,
                size_t index,
                size_t val);

/**
 * Primitive version of \p dal_append().
 */
void dal_pappend(DynarrLO *d, size_t val);

/**
 * Primitive version of \p dal_insert().
 */
void dal_pinsert(DynarrLO *d,
                 size_t index,
                 size_t val);

/**
 * Primitive version of \p dal_insertMany().
 */
void dal_pinsertMany(DynarrLO *d,
                    size_t index,
                    size_t *vals,
                    size_t num);

/**
 * Primitive version of \p dal_get().
 */
size_t dal_pget(DynarrLO *d, size_t index);

/**
 * Primitive version of \p dal_pop().
 */
size_t dal_ppop(DynarrLO *d);

#endif // DAL_PRIMITIVE_SUPPORT

#endif // EASY_DYNARRLO_H
