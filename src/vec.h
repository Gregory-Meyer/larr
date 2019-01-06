#ifndef VEC_H
#define VEC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vec {
    void *data;
    size_t element_size;
    size_t len;
    size_t capacity;
} Vec;

typedef enum VecErr {
    LARR_OK,
    LARR_NO_MEMORY,
    LARR_OUT_OF_RANGE
} VecErr;

/**
 *  Initializes an Vec with length and capacity 0.
 *
 *  @param self Must not be NULL.
 *  @param element_size The stride, in bytes, between elements. Usually sizeof(T).
 */
void Vec_new(Vec *self, size_t element_size);

/**
 *  Initializes an Vec length 0 and at least enough space to hold
 *	capacity elements.
 *
 *  @param self Must not be NULL.
 *  @param capacity The minimum number of elements that this Vec will be able to hold without
 *                  reallocating memory.
 *  @param element_size The stride, in bytes, between elements. Usually sizeof(T).
 *	@returns LARR_NO_MEMORY if malloc() returns NULL, otherwise
 *			 LARR_OK.
 */
int Vec_with_capacity(Vec *self, size_t element_size, size_t capacity);

/**
 *  Deallocates any memory owned by this Vec and sets its length
 *  and capacity to 0.
 *
 *  @param self Must not be NULL.
 */
void Vec_delete(Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Vec can contain.
 */
size_t Vec_capacity(const Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Vec contains.
 */
size_t Vec_len(const Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns Nonzero if this Vec is empty, zero otherwise.
 */
int Vec_is_empty(const Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the first element of this Vec
 *           if this Vec is non-empty, NULL otherwise.
 */
const void* Vec_first(const Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the first element of this Vec if
 *           this Vec is non-empty, NULL otherwise.
 */
void* Vec_first_mut(Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the last element of this Vec
 *           if this Vec is non-empty, NULL otherwise.
 */
const void* Vec_last(const Vec *self);

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the last element of this Vec if
 *           this Vec is non-empty, NULL otherwise.
 */
void* Vec_last_mut(Vec *self);

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns An immutable reference to the index-th element of this
 *           Vec if index is in [0, len), NULL otherwise.
 */
const void* Vec_get(const Vec *self, size_t index);

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns A mutable reference to the index-th element of this Vec
 *           if index is in [0, len), NULL otherwise.
 */
void* Vec_get_mut(Vec *self, size_t index);

/**
 *  Appends an element to the end of this Vec.
 *
 *  If this Vec is full, will reallocate memory with realloc() and
 *  invalidate any previously created references to elements contained
 *  in the Vec.
 *
 *  @param self Must not be NULL.
 *  @param element Will be pushed onto the end of the Vec.
 *  @returns LARR_NO_MEMORY if realloc() returns NULL, LARR_OK
 *           otherwise.
 */
int Vec_push(Vec *self, const void *element);

/**
 *  Removes the last element from this Vec.
 *
 *  @param self Must not be NULL.
 *  @returns LARR_OUT_OF_RANGE if this Vec is empty, otherwise
 *           LARR_OK.
 */
int Vec_pop(Vec *self);

/**
 *  Inserts an element into an Vec at index.
 *
 *  All elements at and after index are shifted right, invalidating
 *  references to them. If this Vec is full, reallocates memory using
 *  realloc() and invalidates any references to elements contained in
 *  the Vec.
 *
 *  @param self Must not be NULL.
 *  @param index Should be <= len.
 *  @param element The element to insert.
 *  @returns LARR_OK if element was inserted, LARR_OUT_OF_RANGE if
 *           index > len, or LARR_NO_MEMORY if realloc() returns NULL.
 */
int Vec_insert(Vec *self, size_t index, const void *element);

/**
 *  Removes the element at index.
 *
 *  All elements after index are shifted left, invaliding references to
 *  them.
 *
 *  @param self Must not be NULL.
 *  @param index Should be < len.
 *  @returns LARR_OUT_OF_RANGE if index >= len, otherwise LARR_OK.
 */
int Vec_remove(Vec *self, size_t index);

/**
 *  Resets the length of the Vec to zero without deallocating any
 *  memory.
 */
void Vec_clear(Vec *self);

/**
 *	Preallocates space for at least len + additional elements. If there
 *	isn't enough space as is, memory will be reallocated and all
 *	existing references will be invalidated.
 *
 *	@param self Must not be NULL.
 *	@param additional The minimum number of extra elements that this
 *					  Vec should be able to push without reallocating.
 *	@returns LARR_NO_MEMORY if realloc() returns NULL, LARR_OK
 *			 otherwise.
 */
int Vec_reserve(Vec *self, size_t additional);

/** @returns An immutable pointer to this Vec's memory buffer. */
const void* Vec_as_ptr(const Vec *self);

/** @returns A mutable pointer to this Vec's memory buffer. */
void* Vec_as_mut_ptr(Vec *self);

/**
 *	Copies an array of elements to the end of this Vec.
 *
 *	If there isn't enough capacity to hold all additional elements,
 *	memory will be reallocated and all existing references will be
 *	invalidated.
 *
 *	@param self Must not be NULL.
 *	@param other Must not be NULL. Points to memory spanning
 *				 len elements of element_size bytes each.
 *	@param len The number of elements to copy.
 *	@returns LARR_NO_MEMORY if realloc() returns NULL, LARR_OK
 *			 otherwise.
 */
int Vec_append(Vec *self, const void *other, size_t len);

/**
 *	Shortens this Vec to len elements. If len > the current length,
 *	equivalent to Vec_clear. Performs no reallocation.
 *
 *	@param self Must not be NULL.
 *	@param len The number of elements to keep.
 */
void Vec_truncate(Vec *self, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
