#include "array.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/**
 *  Initializes an Array with length and capacity 0.
 *
 *  @param self Must not be NULL.
 */
void Array_new(Array *self) {
    assert(self);

    self->data_ = NULL;
    self->len_ = 0;
    self->capacity_ = 0;
}

static void** realloc_and_move(Array *self, size_t capacity);

/**
 *  Initializes an Array length 0 and at least enough space to hold
 *  capacity elements.
 *
 *  @param self Must not be NULL.
 *  @returns LARR_NO_MEMORY If malloc() returns NULL, otherwise
 *           LARR_OK.
 */
ArrayError Array_with_capacity(Array *self, size_t capacity) {
    assert(self);

    Array_new(self);

    if (!realloc_and_move(self, capacity)) {
        return LARR_NO_MEMORY;
    }

    return LARR_OK;
}

/**
 *  Deallocates any memory owned by this Array and sets its length
 *  and capacity to 0.
 *
 *  @param self Must not be NULL.
 */
void Array_delete(Array *self) {
    assert(self);

    free(self->data_);
    self->data_ = NULL;
    self->len_ = 0;
    self->capacity_ = 0;
}

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Array can contain.
 */
size_t Array_capacity(const Array *self) {
    assert(self);

    return self->capacity_;
}

/**
 *  @param self Must not be NULL.
 *  @returns The number of elements that this Array contains.
 */
size_t Array_len(const Array *self) {
    assert(self);

    return self->len_;
}

/**
 *  @param self Must not be NULL.
 *  @returns Nonzero if this Array is empty, zero otherwise.
 */
int Array_is_empty(const Array *self) {
    assert(self);

    return (self->len_ == 0);
}

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the first element of this Array
 *           if this Array is non-empty, NULL otherwise.
 */
const void** Array_first(const Array *self) {
    assert(self);

    return Array_get(self, 0);
}

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the first element of this Array if
 *           this Array is non-empty, NULL otherwise.
 */
void** Array_first_mut(Array *self) {
    assert(self);

    return Array_get_mut(self, 0);
}

/**
 *  @param self Must not be NULL.
 *  @returns An immutable reference to the last element of this Array
 *           if this Array is non-empty, NULL otherwise.
 */
const void** Array_last(const Array *self) {
    assert(self);

    return Array_get(self, self->len_ - 1);
}

/**
 *  @param self Must not be NULL.
 *  @returns A mutable reference to the last element of this Array if
 *           this Array is non-empty, NULL otherwise.
 */
void** Array_last_mut(Array *self) {
    assert(self);

    return Array_get_mut(self, self->len_ - 1);
}

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns An immutable reference to the index-th element of this
 *           Array if index is in [0, len), NULL otherwise.
 */
const void** Array_get(const Array *self, size_t index) {
    assert(self);

    if (index >= self->len_) {
        return NULL;
    }

    return self->data_ + index;
}

/**
 *  @param self Must not be NULL.
 *  @param index The index of the element to get.
 *  @returns A mutable reference to the index-th element of this Array
 *           if index is in [0, len), NULL otherwise.
 */
void** Array_get_mut(Array *self, size_t index) {
    assert(self);

    if (index >= self->len_) {
        return NULL;
    }

    return self->data_ + index;
}

/**
 *  If this Array is full, will reallocate memory and invalidate any
 *  previously created references to elements contained in the Array.
 *
 *  @param self Must not be NULL.
 *  @param value Will be pushed onto the end of the Array.
 *  @returns NULL If realloc() returns NULL, a mutable reference to the
 *           pushed element otherwise.
 */
void** Array_push(Array *self, void *value) {
    assert(self);

    if (!realloc_and_move(self, self->len_ + 1)) {
        return NULL;
    }

    self->data_[self->len_] = value;
    ++self->len_;

    return self->data_ + self->len_ - 1;
}

/**
 *  @param self Must not be NULL.
 *  @returns NULL If this Array is empty, a copy of the last element
 *           otherwise.
 */
void* Array_pop(Array *self) {
    assert(self);

    if (self->len_ == 0) {
        return 0;
    }

    --self->len_;

    return self->data_[self->len_];
}

/**
 *  Move all elements in arr one index right, overwriting the last
 *  element and leaving the first element unmodified.
 */
static void shift_right(void **arr, size_t length);

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
int Array_insert(Array *self, size_t index, void *element) {
    assert(self);

    if (index > self->len_) {
        return LARR_OUT_OF_RANGE;
    } else if (!realloc_and_move(self, self->len_ + 1)) {
        return LARR_NO_MEMORY;
    }

    shift_right(self->data_ + index, self->len_ - index);
    self->data_[index] = element;
    ++self->len_;

    return LARR_OK;
}

/**
 *  Move all elements in arr one index left, overwriting the first
 *  element and leaving the last element unmodified.
 */
static void shift_left(void **arr, size_t length);

/**
 *  Removes the element at index. All elements after index are shifted
 *  left, invaliding references to them.
 *
 *  @param self Must not be NULL.
 *  @param index Should be < len.
 *  @returns NULL If index >= len, otherwise the removed element.
 */
void* Array_remove(Array *self, size_t index) {
    assert(self);

    if (index >= self->len_) {
        return NULL;
    } else {
        void *const removed = self->data_[index];
        shift_left(self->data_ + index, self->len_ - index);
        --self->len_;

        return removed;
    }
}

void Array_clear(Array *self) {
    assert(self);

    self->len_ = 0;
}

static size_t round_up_to_next_highest_power_of_2(size_t x);

static void** realloc_and_move(Array *self, size_t capacity) {
    assert(self);

    if (self->capacity_ >= capacity) {
        return self->data_;
    } else {
        const size_t new_capacity = round_up_to_next_highest_power_of_2(capacity);
        void **const new_data = (void**) realloc(self->data_, sizeof(void*) * new_capacity);

        if (!new_data) {
            return NULL;
        }

        self->data_ = new_data;
        self->capacity_ = new_capacity;

        return new_data;
    }
}

/**
 *  Move all elements in arr one index right, overwriting the last
 *  element and leaving the first element unmodified.
 */
static void shift_right(void **arr, size_t length) {
    size_t i = length - 1; /* grr c89 */
    for (; i >= 1; --i) { /* weird, but avoids overflow/underflow */
        arr[i] = arr[i - 1];
    }
}

/**
 *  Move all elements in arr one index left, overwriting the first
 *  element and leaving the last element unmodified.
 */
static void shift_left(void **arr, size_t length) {
    if (length == 0) { /** to avoid underflow */
        return;
    } else {
        size_t i = 0;
        for (; i < length - 1; ++i) {
            arr[i] = arr[i + 1];
        }
    }
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
