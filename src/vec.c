#include "vec.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/**
 *  Initializes an Vec with length and capacity 0.
 *
 *  @param self Must not be NULL.
 */
void Vec_new(Vec *self, size_t element_size) {
    assert(self);

    self->data = NULL;
    self->element_size = element_size;
    self->len = 0;
    self->capacity = 0;
}

/**
 *  Initializes an Vec length 0 and at least enough space to hold
 *  capacity elements.
 *
 *  @param self Must not be NULL.
 *  @returns LARR_NO_MEMORY If malloc() returns NULL, otherwise
 *           LARR_OK.
 */
int Vec_with_capacity(Vec *self, size_t element_size, size_t capacity) {
    assert(self);

    Vec_new(self, element_size);

    if (Vec_reserve(self, capacity) != LARR_OK) {
        return LARR_NO_MEMORY;
    }

    return LARR_OK;
}

/**
 *  Deallocates any memory owned by this Vec and sets its length
 *  and capacity to 0.
 *
 *  @param self Must not be NULL.
 */
void Vec_delete(Vec *self) {
    assert(self);

    free(self->data);
    self->data = NULL;
    self->len = 0;
    self->capacity = 0;
}

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Vec can contain.
 */
size_t Vec_capacity(const Vec *self) {
    assert(self);

    return self->capacity;
}

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Vec contains.
 */
size_t Vec_len(const Vec *self) {
    assert(self);

    return self->len;
}

/**
 *  @param self Must not be NULL.
 *  @returns Nonzero if this Vec is empty, zero otherwise.
 */
int Vec_is_empty(const Vec *self) {
    assert(self);

    return (self->len == 0);
}

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the first element of this Vec
 *           if this Vec is non-empty, NULL otherwise.
 */
const void* Vec_first(const Vec *self) {
    assert(self);

    return Vec_get(self, 0);
}

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the first element of this Vec if
 *           this Vec is non-empty, NULL otherwise.
 */
void* Vec_first_mut(Vec *self) {
    assert(self);

    return Vec_get_mut(self, 0);
}

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the last element of this Vec
 *           if this Vec is non-empty, NULL otherwise.
 */
const void* Vec_last(const Vec *self) {
    assert(self);

    return Vec_get(self, self->len - 1);
}

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the last element of this Vec if
 *           this Vec is non-empty, NULL otherwise.
 */
void* Vec_last_mut(Vec *self) {
    assert(self);

    return Vec_get_mut(self, self->len - 1);
}

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns An immutable reference to the index-th element of this
 *           Vec if index is in [0, len), NULL otherwise.
 */
const void* Vec_get(const Vec *self, size_t index) {
    assert(self);

    if (index >= self->len) {
        return NULL;
    }

    return (const char*) self->data + index * self->element_size;
}

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns A mutable reference to the index-th element of this Vec
 *           if index is in [0, len), NULL otherwise.
 */
void* Vec_get_mut(Vec *self, size_t index) {
    assert(self);

    if (index >= self->len) {
        return NULL;
    }

    return (char*) self->data + index * self->element_size;
}

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
int Vec_push(Vec *self, const void *element) {
    assert(self);

    if (Vec_reserve(self, 1) != LARR_OK) {
        return LARR_NO_MEMORY;
    }

    memcpy((char*) self->data + self->len * self->element_size, element, self->element_size);
    ++self->len;

    return LARR_OK;
}

/**
 *  Removes the last element from this Vec.
 *
 *  @param self Must not be NULL.
 *  @returns LARR_OUT_OF_RANGE if this Vec is empty, otherwise
 *           LARR_OK.
 */
int Vec_pop(Vec *self) {
    assert(self);

    if (self->len == 0) {
        return LARR_OUT_OF_RANGE;
    }

    --self->len;

    return LARR_OK;
}

/**
 *  Move all elements in arr one index right, overwriting the last
 *  element and leaving the first element unmodified.
 */
static void shift_right(void *arr, size_t element_size, size_t length);

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
int Vec_insert(Vec *self, size_t index, const void *element) {
    assert(self);

    if (index > self->len) {
        return LARR_OUT_OF_RANGE;
    } else if (Vec_reserve(self, 1) != LARR_OK) {
        return LARR_NO_MEMORY;
    }

    shift_right((char*) self->data + self->element_size * index,
                self->element_size, self->len - index);
    memcpy((char*) self->data + self->element_size * index, element, self->element_size);
    ++self->len;

    return LARR_OK;
}

/**
 *  Move all elements in arr one index left, overwriting the first
 *  element and leaving the last element unmodified.
 */
static void shift_left(void *arr, size_t element_size, size_t length);

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
int Vec_remove(Vec *self, size_t index) {
    assert(self);

    if (index >= self->len) {
        return LARR_OUT_OF_RANGE;
    }

    shift_left((char*) self->data + self->element_size + index,
               self->element_size, self->len - index);
    --self->len;

    return LARR_OK;
}

/**
 *  Resets the length of the Vec to zero without deallocating any
 *  memory.
 */
void Vec_clear(Vec *self) {
    assert(self);

    self->len = 0;
}

static size_t round_up_to_next_highest_power_of_2(size_t x);

/**
 *  Preallocates space for at least len + additional elements. If there
 *  isn't enough space as is, memory will be reallocated and all
 *  existing references will be invalidated.
 *
 *  @param self Must not be NULL.
 *  @param additional The minimum number of extra elements that this
 *                    Vec should be able to push without reallocating.
 *  @returns LARR_NO_MEMORY if realloc() returns NULL, LARR_OK
 *           otherwise.
 */
int Vec_reserve(Vec *self, size_t additional) {
    size_t requested_capacity;

    assert(self);

    requested_capacity = self->len + additional;

    if (self->capacity >= requested_capacity) {
        return LARR_OK;
    } else {
        const size_t new_capacity = round_up_to_next_highest_power_of_2(requested_capacity);
        void *const new_data = (void*) realloc(self->data, self->element_size * new_capacity);

        if (!new_data) {
            return LARR_NO_MEMORY;
        }

        self->data = new_data;
        self->capacity = new_capacity;

        return LARR_OK;
    }
}

/** @returns An immutable pointer to this Vec's memory buffer. */
const void* Vec_as_ptr(const Vec *self) {
    assert(self);

    return self->data;
}

/** @returns A mutable pointer to this Vec's memory buffer. */
void* Vec_as_mut_ptr(Vec *self) {
    assert(self);

    return self->data;
}

/**
 *  Copies an array of elements to the end of this Vec.
 *
 *  If there isn't enough capacity to hold all additional elements,
 *  memory will be reallocated and all existing references will be
 *  invalidated.
 *
 *  @param self Must not be NULL.
 *  @param other Must not be NULL. Points to memory spanning
 *               len elements of element_size bytes each.
 *  @param len The number of elements to copy.
 *  @returns LARR_NO_MEMORY if realloc() returns NULL, LARR_OK
 *           otherwise.
 */
int Vec_append(Vec *self, const void *other, size_t len) {
    assert(self);
    assert(other);

    if (len == 0) {
        return LARR_OK;
    }

    if (Vec_reserve(self, len) != LARR_OK) {
        return LARR_NO_MEMORY;
    } else {
        memcpy((char*) self->data + self->len * self->element_size, other,
               len * self->element_size);
        self->len += len;

        return LARR_OK;
    }
}

/**
 *  Move all elements in arr one index right, overwriting the last
 *  element and leaving the first element unmodified.
 */
static void shift_right(void *arr, size_t element_size, size_t length) {
    if (length == 0 || element_size == 0) {
        return;
    }

    memmove((char*) arr + element_size, arr, (length - 1) * element_size);
}

/**
 *  Move all elements in arr one index left, overwriting the first
 *  element and leaving the last element unmodified.
 */
static void shift_left(void *arr, size_t element_size, size_t length) {
    if (length == 0 || element_size == 0) {
        return;
    }

    memmove(arr, (const char*) arr + element_size, (length - 1) * element_size);
}

/* Stanford bit twiddling hack */
static size_t round_up_to_next_highest_power_of_2(size_t x) {
    assert(sizeof(size_t) * CHAR_BIT == 32 || sizeof(size_t) * CHAR_BIT == 64);

    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    if (sizeof(size_t) * CHAR_BIT == 64) {
        x |= x >> 32;
    }

    ++x;

    return x;
}
