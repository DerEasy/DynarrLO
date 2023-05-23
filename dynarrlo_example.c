#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "dynarrlo.h"


void stack_like_usage(DynarrLO *d) {
    // Print every digit of a 64-bit unsigned integer
    // set to its maximum value separately.

    dal_createDynarrLO(d, 20, realloc, free);
    uint64_t n = -1;
    puts("Largest unsigned 64-bit integer value:");

    while (n) {
        dal_pappend(d, n % 10);
        n /= 10;
    }

    while (d->length)
        putchar('0' + dal_ppop(d));
    
    printf("\n\n\n");
    dal_destroyDynarrLO(d);
}


DynarrLO *generate_primes(size_t amount) {
    // Generate some amount of prime numbers.
    // I will use the prime factorisation method.

    DynarrLO *primes = malloc(sizeof *primes);
    dal_createDynarrLO(primes, amount, realloc, free);

    for (size_t n = 2; primes->length < amount; ++n) {
        for (size_t i = 0; i < primes->length; ++i) {
            if (n % dal_pget(primes, i) == 0) {
                goto not_prime;
            }
        }

        dal_pappend(primes, n);

        not_prime:
        ;   // Just do nothing
        // This is also how you replicate a for ... else
        // construct from Python in C by the way.
    }

    return primes;
}


typedef struct IntSq {
    int n;
    int n2;
} IntSq;

void squares(DynarrLO *d, int rangeStart, int rangeEnd) {
    // Example of setting capacity below minimum allowed
    // capacity. This DynarrLO will have DAL_MIN_CAPACITY
    // capacity instead.

    dal_createDynarrLO(d, 0, realloc, free);

    for (int i = rangeStart; i < rangeEnd; ++i) {
        IntSq *isq = dal_appendInst(d, sizeof *isq);
        isq->n = i;
        isq->n2 = i * i;
    }

    // Shrink to fit.
    dal_setCapacity(d, d->length);
}


void errors(DynarrLO *d) {
    dal_createDynarrLO(d, 5, realloc, free);

    // Zero-initialise the entire DynarrLO
    dal_zeroOut(d, 0, d->capacity);

    // Manually setting the length to some value, so
    // we can access those elements without causing
    // an error.
    dal_setLength(d, 3);
    // It's as if we just added three elements to our array.


    // Don't care for their values
    int *a = malloc(sizeof *a * 5);

    for (size_t i = 0; i < d->capacity; ++i)
        dal_write(d, i, a + i);
    
    // Uh-oh! We wrote to index 3 and 4, but those indices
    // aren't in the length-3-bounds we set earlier!
    // The error flag is now set to DAL_OUTOFRANGE, but
    // the pointers have still been written into the DynarrLO,
    // as the out-of-range error was just a logical one, not
    // an actual illegal memory access.
    printf(
        "This demonstrates a logical out-of-range error: %i\n",
        d->error
    );

    // We can verify this by reading the value at that address
    // and comparing it to our original pointer. (This will also
    // cause an out-of-range error).
    printf(
        "Array element equals passed pointer: %s\n\n",
        dal_get(d, 4) == (a + 4) ? "Yes" : "No"
    );

    // Now if we actually did try an illegal memory access,
    // DynarrLO would simply discard the write operation:
    dal_pwrite(d, 10, 123456);

    // If we tried to read back the value, we would get 0 / NULL back:
    printf(
        "This demonstrates a real out-of-range error: %i\n"
        "Array element equals passed value: %s\n",
        d->error,
        dal_pget(d, 10) == 123456 ? "Yes" : "No"
    );
    
    
    // This is legal because the malloc function above is the same
    // one that we use for our DynarrLO object, albeit in the form
    // of realloc. If it wasn't, we'd need to instead call the
    // corresponding free() function manually on that object like
    // free(dal_get(d, 0));
    // in this specific case, we could also just write
    // free(a);
    dal_freeItem(d, 0);


    dal_destroyDynarrLO(d);
}


int main(void) {
    DynarrLO d;

    // Example 1
    stack_like_usage(&d);



    // Example 2
    DynarrLO *primes = generate_primes(30);
    puts("First 30 prime numbers:");

    for (size_t i = 0; i < primes->length; ++i)
        printf("%zu\n", dal_pget(primes, i));
    
    printf("\n\n");
    dal_destroyDynarrLO(primes);
    free(primes);
    


    // Example 3
    squares(&d, -5, 20);
    puts("Some squares:");

    for (size_t i = 0; i < d.length; ++i) {
        IntSq *isq = dal_get(&d, i);
        printf("(%i)² = %i\n", isq->n, isq->n2);
    }

    printf("\n\n");
    dal_destroyDynarrLO(&d);



    // Example 4
    errors(&d);
}
