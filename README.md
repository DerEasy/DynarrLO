# DynarrLO
Dynamic Array implementation focused on low overhead in C. Compliant with C11 and C17 (at least). For C99 compliance see below.

## Main features
- Low overhead
- Small struct size (48 Bytes on my machine)
- Struct definition in header file
- No heap allocation required
- Supports generic types via void pointers and "primitive data types" via size_t
- Each DynarrLO object contains its own error flag
- (De-)Allocation function freely choosable
- Written in pure C, no extensions, very few standard library functions used
- Safety measures to prevent illegal memory accesses (where preventable by library)
- No automatic shrinkage of internal array

## Primitive support
DynarrLO can either store a `void *` or a `size_t` type object in its internal array. The latter is useful if you want a dynamic array that uses its array as the storage for the actual objects instead of just pointers to some other location. It may safe you a lot of memory and prevent memory fragmentation if your objects are small, but it also means that the size of these primitives are limited to the `sizeof(size_t)`.

DynarrLO uses an unnamed union of `void **` and `size_t *` for its internal array to make this possible. DynarrLO automatically compiles with primitive support disabled when the `sizeof(size_t)` does not match the `sizeof(void *)`, in which case this unnamed union is removed from the struct definition and instead replaced by just a `void **`. All primitive functions will also be removed from compilation.

You may manually and forcibly enable/disable primitive support by defining the macro `DAL_PRIMITIVE_SUPPORT` as 1 or 0 to enable or disable primitive support respectively before including the header. Alternatively you may define the macro during compilation as a compile option or just edit the macro definition inside of the header file `dynarrlo.h`.

## Installation
I'm not experienced with other OS's / Distros and their differences. If you know how to install a simple library on your system, then just do that. This is what I do to compile DynarrLO as a static library on **Ubuntu Linux** with gcc:

1. Open a terminal
2. Download the repository: `git clone https://github.com/DerEasy/DynarrLO.git`
3. Navigate into cloned directory `DynarrLO`
4. Run `cmake .`
5. Run `make`
6. Copy the compiled static library file `libdynarrlo.a` to your library path. I copy to `/usr/local/lib`. 
7. Copy the header file `dynarrlo.h` to your include path. I copy to `/usr/local/include`.

That should be it. To use the library in your project, just `#include "dynarrlo.h"` and compile with `-ldynarrlo`.

### Restore C99 compliance
DynarrLO uses an unnamed union in its struct definition. This breaks strict C99 compliance. One trivial way to circumvent the issue is to compile without primitive support. This removes the union and replaces it with just the `void **` for the internal array. This will also remove all primitive functions.

Before performing step 4, open the header file `dynarrlo.h`. Somewhere at the very top you will find the macro definition `DAL_PRIMITIVE_SUPPORT`. Replace its definition with just `0`. Done.

## Implementation details
What does "low overhead" mean? Strictly speaking, this "low overhead" will partly depend on your compiler and platform. Here my results on x86_64 GNU/Linux using gcc 11.3.0:

- Many important key functions are completely branchless and constant time operations. These include:
  - `dal_setLength()`
  - `dal_write()`
  - `dal_get()`
  - `dal_pop()`
  - `dal_removeLast()`
  - `dal_removeLastMany()`
  - …and all of their primitive variants
- All above functions implement error checking and correction and they still just cost roughly one or two dozen (fast) instructions.
- The array will not automatically shrink. It will only grow automatically. You can set the capacity to a lower value yourself if needed.
- DynarrLO will never perform any memory allocation other than array growth or the programmer explicitly allocating an object with its built-in functions.
- The struct size is small enough to fit in a cache line.
- The implementation is quite small. The source file contains around 400 lines. The header contains roughly 450, though most of it is just documentation.

Convince yourself of the assembler output by running `objdump -d libdynarrlo.a -M intel > dynarrlo.s` on the library file.

### What you should know
A DynarrLO object requires both a `realloc()` and a `free()` function to do its allocations. It doesn't matter what implementation you use, you may even define your own functions. The only requirement is that they comply to the C standard. You can entirely avoid heap allocation this way, if that is what you need. The growth factor of the internal array is 1.5x.

The struct definition of DynarrLO is in its header. This means you have access to the innards of it.
- You may read any of the fields.
  - This is even necessary to get the length and capacity of the the array, as I have decided against declaring functions for them.
  - Keep in mind that you will have no error protection if you read from the arrays in this way. Use the provided functions. They have next to no overhead (the reason for the existence of this library).
- You may clear the error flag by directly writing to it (beware of common issues in multithreaded programs).
- But: ***DO NOT modify/write to any other fields under any circumstance!***
- To initialise or discard a DynarrLO, use `dal_createDynarrLO()` and `dal_destroyDynarrLO` respectively. You may reinitialise a DynarrLO object after having destroyed it.

DynarrLO has a minimum capacity specified by the macro `DAL_MIN_CAPACITY` (currently 2, unlikely to change). It is not possible to configure less capacity than this and any value below it will be 'rounded' to this value.

Important side note: DynarrLO will always allocate `capacity + 1` elements for its array. That extra padding element at the end is always initialised to NULL and you have no access to it. This padding element is important for error handling and manually modifying its value may cause all sorts of nasal demons and undesired behaviour. It is not possible to accidentally modify the padding element if you use the library functions and refrain from directly accessing the internal array.

Some functions have primitive variants. These are prefixed with an additional 'p' before their actual name. Funnily, in the case of `dal_pop()`, this leads to `dal_ppop()`. Don't get confused.

## Documentation
The header file contains Doxygen documentation for every struct, field, function and macro. Everything relating to DynarrLO is prefixed with `dal_` or `DAL_`.

You may also refer to the `dynarrlo_example.c` file in this repository for some examples on how this library is used. There are a total of four examples available currently.
