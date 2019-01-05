#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Array {
    void **data_;
    size_t len_;
    size_t capacity_;
} Array;

typedef enum ArrayError {
    LARR_OK,
    LARR_NO_MEMORY,
    LARR_OUT_OF_RANGE
} ArrayError;

/**
 *  Initializes an Array with length and capacity 0.
 *
 *  @param self Must not be NULL.
 */
void Array_new(Array *self);

/**
 *  Initializes an Array length 0 and at least enough space to hold
 *	capacity elements.
 *
 *  @param self Must not be NULL.
 *	@returns LARR_NO_MEMORY If malloc() returns NULL, otherwise
 *			 LARR_OK.
 */
ArrayError Array_with_capacity(Array *self, size_t capacity);

/**
 *  Deallocates any memory owned by this Array and sets its length
 *  and capacity to 0.
 *
 *  @param self Must not be NULL.
 */
void Array_delete(Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Array can contain.
 */
size_t Array_capacity(const Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Array contains.
 */
size_t Array_len(const Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns Nonzero if this Array is empty, zero otherwise.
 */
int Array_is_empty(const Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the first element of this Array
 *           if this Array is non-empty, NULL otherwise.
 */
const void** Array_first(const Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the first element of this Array if
 *           this Array is non-empty, NULL otherwise.
 */
void** Array_first_mut(Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the last element of this Array
 *           if this Array is non-empty, NULL otherwise.
 */
const void** Array_last(const Array *self);

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the last element of this Array if
 *           this Array is non-empty, NULL otherwise.
 */
void** Array_last_mut(Array *self);

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns An immutable reference to the index-th element of this
 *           Array if index is in [0, len), NULL otherwise.
 */
const void** Array_get(const Array *self, size_t index);

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns A mutable reference to the index-th element of this Array
 *           if index is in [0, len), NULL otherwise.
 */
void** Array_get_mut(Array *self, size_t index);

/**
 *  If this Array is full, will reallocate memory with realloc() and
 *  invalidate any previously created references to elements contained
 *  in the Array.
 *
 *  @param self Must not be NULL.
 *  @param element Will be pushed onto the end of the Array.
 *  @returns NULL If realloc() returns NULL, a mutable reference to the
 *           pushed element otherwise.
 */
void** Array_push(Array *self, void *element);

/**
 *  @param self Must not be NULL.
 *  @returns NULL If this Array is empty, a copy of the last element
 *           otherwise.
 */
void* Array_pop(Array *self);

/**
 *  Inserts an element into an Array at index. All elements at and
 *  after index are shifted right, invalidating references to them. If
 *  this Array is full, reallocates memory using realloc() and
 *  invalidates any references to elements contained in the Array.
 *
 *  @param self Must not be NULL.
 *  @param index Should be <= len.
 *  @param element The element to insert.
 *  @returns LARR_OK If element was inserted, LARR_OUT_OF_RANGE if
 *           index > len, or LARR_NO_MEMORY if realloc() returns NULL.
 */
int Array_insert(Array *self, size_t index, void *element);

/**
 *  Removes the element at index. All elements after index are shifted
 *  left, invaliding references to them.
 *
 *  @param self Must not be NULL.
 *  @param index Should be < len.
 *  @returns NULL If index >= len, otherwise the removed element.
 */
void* Array_remove(Array *self, size_t index);

/**
 *  Resets the length of the Array to zero without deallocating any
 *  memory.
 */
void Array_clear(Array *self);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
