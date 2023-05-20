//
// Created by easy on 19.05.23.
//

#include "dynarrlo.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



static size_t firstElement(DynarrLO *d) {
    return d->arrayp[0];
}


static size_t lastElement(DynarrLO *d) {
    return d->arrayp[d->length - !!d->length];
}


static size_t onepastElement(DynarrLO *d) {
    return d->arrayp[d->length];
}


static size_t paddingElement(DynarrLO *d) {
    return d->arrayp[d->capacity];
}


static void setpadding(DynarrLO *d, size_t val) {
    d->arrayp[d->capacity] = val;
}


static void printDynarrLO(DynarrLO *d, int b) {
    char error[18];

    if (d->error == DAL_OK)
        strcpy(error, "OK");
    else if (d->error == DAL_OUTOFRANGE)
        strcpy(error, "OUT OF RANGE");
    else if (d->error == DAL_NULLARG)
        strcpy(error, "NULL ARGUMENT");
    else if (d->error == DAL_ALLOCFAIL)
        strcpy(error, "ALLOCATION FAILED");

    printf("----------------------------------------\n"
           "DynarrLO   %p\n"
           "Length:    %zu\tCapacity: %zu\n"
           "First:     %zu\n"
           "Last:      %zu\n"
           "One past:  %zu\n"
           "Padding:   %zu\n"
           "Error:     %s\n\n",
           d,
           d->length, d->capacity,
           firstElement(d),
           lastElement(d),
           onepastElement(d),
           paddingElement(d),
           error
           );

    for (size_t i = 0; i < d->length; ++i) {
        printf(" [%zu]  %zu\n", i, dal_pget(d, i));
    }

    for (size_t i = d->length; b && i < d->capacity; ++i) {
        printf("-[%zu]  %zu\n", i, dal_pget(d, i));
    }

    printf("----------------------------------------\n\n");
}


static void dal_test_zeroOut(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 10, realloc, free);

    for (size_t i = 0; i < d->capacity; ++i) {
        dal_pappend(d, i * i);
    }

    setpadding(d, 69);
    dal_setLength(d, 3);
    printDynarrLO(d, true);
    dal_zeroOut(d, 3, 0);
    printDynarrLO(d, true);
}


static void dal_test_setCapacity(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 0, realloc, free);

    printf("%zu\n", d->capacity);
    dal_setCapacity(d, 10000);
    printf("%zu\n", d->capacity);
    dal_setCapacity(d, 0);
    printf("%zu\n", d->capacity);
    dal_setCapacity(d, 10);
    printf("%zu\n", d->capacity);
    dal_setCapacity(d, 1);
    printf("%zu\n", d->capacity);
    dal_setCapacity(d, -1);
    printf("%zu\n", d->capacity);
    printf("%i\n", d->error);
}


static void dal_test_write(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 0, realloc, free);
    dal_setLength(d, d->capacity);

    dal_write(d, 0, 5);
    dal_write(d, 1, 10);
    dal_write(d, 2, 15);
    dal_write(d, 3, 20);
}


static void dal_test_append(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 0, realloc, free);

    for (size_t i = 1; i <= 10; ++i)
        dal_append(d, 5 * i);
}


static void dal_test_insert(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 0, realloc, free);

    for (size_t i = 0; i < 10; ++i) {
        printDynarrLO(d, false);
        dal_insert(d, i, 5 * (i + 1));
    }

    printDynarrLO(d, false);
    dal_insert(d, 4, 999);
    dal_insert(d, 3, 888);
    dal_insert(d, 4, 777);
    printDynarrLO(d, false);
    dal_insert(d, 0, 1234);
    dal_insert(d, 0, 5678);
    printDynarrLO(d, false);
    dal_insert(d, 17, 55555);
    printDynarrLO(d, true);
}


static void dal_test_insertMany(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 0, realloc, free);

    void *a[] = {1,2,3,4,5,6,7,8};
    printDynarrLO(d, true);
    dal_insertMany(d, 0, a, 8);
    printDynarrLO(d, true);
    dal_insertMany(d, 4, a + 2, 4);
    printDynarrLO(d, true);
    dal_setLength(d, 0);
    dal_insertMany(d, 0, a, 8);
    printDynarrLO(d, true);
    dal_insertMany(d, 4, a + 5, 3);
    printDynarrLO(d, true);
}


static void dal_test_remove(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 8, realloc, free);

    for (size_t i = 0; i < 8; ++i)
        dal_append(d, 5 * (i + 1));

    printDynarrLO(d, true);
    dal_removeLast(d);
    printDynarrLO(d, true);
    dal_remove(d, 3);
    printDynarrLO(d, true);
    dal_remove(d, 0);
    printDynarrLO(d, true);
    dal_remove(d, -1);
    printDynarrLO(d, true);
}


static void dal_test_removeMany(void) {
    DynarrLO *d = &(DynarrLO) {0};
    dal_createDynarrLO(d, 8, realloc, free);

    for (size_t i = 0; i < 16; ++i)
        dal_append(d, 5 * (i + 1));

    printDynarrLO(d, true);
    dal_removeMany(d, 3, 6);
    printDynarrLO(d, true);
    dal_removeMany(d, 0, 1);
    printDynarrLO(d, true);
    dal_removeMany(d, 10, -1);
    printDynarrLO(d, true);
    dal_removeLastMany(d, 11);
    printDynarrLO(d, true);
}


int main(void) {
    // Test some functions here
}
