//
// Created by easy on 14.05.23.
//

#include "dynarrlo.h"
#include <stdbool.h>
#include <string.h>


// Returns the lesser of the two arguments.
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
// Returns the greater of the two arguments.
#define MAX(a, b) ((a) >= (b) ? (a) : (b))


static size_t itemsToBytes(size_t n) {
    return n * sizeof(void *);
}


static size_t grownCapacity(size_t n) {
    return n + n / 2;
}


static bool growthRequired(const DynarrLO *d) {
    return d->length >= d->capacity;
}


static DAL_ERROR setCapacity(DynarrLO *d, size_t capacity) {
    capacity = MAX(capacity, DAL_MIN_CAPACITY);

    if (capacity == d->capacity)
        return DAL_OK;

    // Allocate 1 padding element
    void **memory = d->realloc(d->array, itemsToBytes(capacity + 1));
    if (!memory)
        return DAL_ALLOCFAIL;

    // Initialise padding element to zero
    memory[capacity] = NULL;
    d->length = MIN(d->length, capacity);
    d->capacity = capacity;
    d->array = memory;
    return DAL_OK;
}


/**
 * Will grow the array if it is necessary to do so. Does nothing on failure.
 * @return True iff memory allocation failed.
 */
static bool growArray(DynarrLO *d) {
    return growthRequired(d) &&
    (d->error = setCapacity(d, grownCapacity(d->capacity)));
}


/**
 * Will grow the array if necessary so it can hold at least num elements.
 * Does nothing on failure.
 * @return True iff memory allocation failed.
 */
static bool growArrayArbitrary(DynarrLO *d, size_t num) {
    size_t size = MAX(grownCapacity(d->capacity), num);

    return num > d->capacity && (d->error = setCapacity(d, size));
}



DAL_ERROR dal_createDynarrLO(DynarrLO *d,
                             size_t capacity,
                             void *(*realloc) (void *, size_t),
                             void (*free) (void *)) {

    if (!d || !realloc || !free)
        return DAL_NULLARG;

    capacity = MAX(capacity, DAL_MIN_CAPACITY);

    // Allocate 1 padding element
    void **memory = realloc(NULL, itemsToBytes(capacity + 1));
    if (!memory)
        return DAL_ALLOCFAIL;

    // Initialise padding element to zero
    memory[capacity] = NULL;
    d->realloc = realloc;
    d->free = free;
    d->capacity = capacity;
    d->length = 0;
    d->error = DAL_OK;
    d->array = memory;

    return DAL_OK;
}


void dal_destroyDynarrLO(DynarrLO *d) {
    d->free(d->array);
    *d = (DynarrLO) {0};
}


void dal_zeroOut(DynarrLO *d,
                 size_t iStart,
                 size_t iEnd) {

    d->error = (iStart >= d->capacity) | (iEnd > d->capacity);
    iEnd = MIN(iEnd, d->capacity);
    iStart = MIN(iStart, iEnd);
    memset(d->array + iStart, 0, itemsToBytes(iEnd - iStart));
}


void dal_setCapacity(DynarrLO *d, size_t capacity) {
    d->error = setCapacity(d, capacity);
}


void dal_setLength(DynarrLO *d, size_t length) {
    d->error = length > d->capacity;
    d->length = MIN(length, d->capacity);
}


void dal_write(DynarrLO *d,
               size_t index,
               void *obj) {

    d->error = index >= d->length;
    index = MIN(index, d->capacity);
    d->array[index] = obj;
    d->array[d->capacity] = NULL;
}


void *dal_writeInst(DynarrLO *d,
                    size_t index,
                    size_t size) {

    d->error = index >= d->length;

    if (index >= d->capacity)
        return NULL;

    void *obj = d->realloc(NULL, size);

    if (!obj) {
        d->error = DAL_ALLOCFAIL;
        return NULL;
    }

    return d->array[index] = obj;
}


void dal_append(DynarrLO *d, void *obj) {
    if (growArray(d))
        return;

    d->error = DAL_OK;
    d->array[d->length++] = obj;
}


void *dal_appendInst(DynarrLO *d, size_t size) {
    d->error = DAL_OK;
    void *obj = d->realloc(NULL, size);

    if (!obj || growArray(d)) {
        d->error = DAL_ALLOCFAIL;
        d->free(obj);
        return NULL;
    }

    return d->array[d->length++] = obj;
}


void dal_insert(DynarrLO *d,
                size_t index,
                void *obj) {

    bool error = index > d->length;
    d->error = error;

    if (error || growArray(d))
        return;

    memmove(d->array + index + 1,
            d->array + index,
            itemsToBytes(d->length - index));

    d->array[index] = obj;
    ++d->length;
}


void dal_insertMany(DynarrLO *d,
                    size_t index,
                    void **objs,
                    size_t num) {

    bool error = index > d->length;
    d->error = error;

    if (error || growArrayArbitrary(d, d->length + num))
        return;

    memmove(d->array + index + num,
            d->array + index,
            itemsToBytes(d->length - index));

    memmove(d->array + index,
            objs,
            itemsToBytes(num));

    d->length += num;
}


void *dal_get(DynarrLO *d, size_t index) {
    d->error = index >= d->length;
    index = MIN(index, d->capacity);
    return d->array[index];
}


void *dal_pop(DynarrLO *d) {
    d->error = !d->length;
    size_t normlen = d->length - !!d->length;
    void *obj1 = NULL;
    void *obj2 = d->array[normlen];
    void *obj = d->length ? obj2 : obj1;
    d->length = normlen;
    return obj;
}


void dal_freeItem(DynarrLO *d, size_t index) {
    d->error = index >= d->length;
    index = MIN(index, d->capacity);
    d->free(d->array[index]);
    d->array[index] = NULL;
}


void dal_freeItems(DynarrLO *d,
                   size_t iStart,
                   size_t iEnd) {

    d->error = (iStart >= d->length) | (iEnd > d->length);
    iEnd = MIN(iEnd, d->capacity);

    for (size_t i = iStart; i < iEnd; ++i) {
        d->free(d->array[i]);
        d->array[i] = NULL;
    }
}


void dal_fremoveLast(DynarrLO *d) {
    d->error = !d->length;
    size_t normlen = d->length - !!d->length;
    d->free(d->array[normlen]);
    d->array[normlen] = NULL;
    d->length = normlen;
}


void dal_removeLast(DynarrLO *d) {
    d->error = !d->length;
    d->length -= !!d->length;
}


void dal_removeLastMany(DynarrLO *d, size_t amount) {
    d->error = amount > d->length;
    amount = MIN(amount, d->length);
    d->length -= amount;
}


void dal_remove(DynarrLO *d, size_t index) {
    if ((d->error = index >= d->length))
        return;

    memmove(d->array + index,
            d->array + index + 1,
            itemsToBytes(d->length - (index + 1)));

    --d->length;
}


void dal_removeMany(DynarrLO *d,
                    size_t iStart,
                    size_t iEnd) {

    d->error = (iStart >= d->length) | (iEnd > d->length);
    iEnd = MIN(iEnd, d->length);
    iStart = MIN(iStart, iEnd);

    memmove(d->array + iStart,
            d->array + iEnd,
            itemsToBytes(d->length - iEnd));

    d->length -= iEnd - iStart;
}



#if DAL_PRIMITIVE_SUPPORT

void dal_pwrite(DynarrLO *d,
                size_t index,
                size_t val) {

    d->error = index >= d->length;
    index = MIN(index, d->capacity);
    d->arrayp[index] = val;
    d->arrayp[d->capacity] = 0;
}


void dal_pappend(DynarrLO *d, size_t val) {
    if (growArray(d))
        return;

    d->arrayp[d->length++] = val;
}


void dal_pinsert(DynarrLO *d,
                 size_t index,
                 size_t val) {

    bool error = index > d->length;
    d->error = error;

    if (error || growArray(d))
        return;

    memmove(d->array + index + 1,
            d->array + index,
            itemsToBytes(d->length - index));

    d->arrayp[index] = val;
    ++d->length;
}


void dal_pinsertMany(DynarrLO *d,
                    size_t index,
                    size_t *vals,
                    size_t num) {

    bool error = index > d->length;
    d->error = error;

    if (error || growArrayArbitrary(d, d->length + num))
        return;

    memmove(d->array + index + num,
            d->array + index,
            itemsToBytes(d->length - index));

    memmove(d->array + index,
            vals,
            itemsToBytes(num));

    d->length += num;
}


size_t dal_pget(DynarrLO *d, size_t index) {
    d->error = index >= d->length;
    index = MIN(index, d->capacity);
    return d->arrayp[index];
}


size_t dal_ppop(DynarrLO *d) {
    d->error = !d->length;
    size_t normlen = d->length - !!d->length;
    size_t val1 = 0;
    size_t val2 = d->arrayp[normlen];
    size_t val = d->length ? val2 : val1;
    d->length = normlen;
    return val;
}

#endif // DAL_PRIMITIVE_SUPPORT
