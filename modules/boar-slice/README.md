# boar-slice

A `slice` represents a virtual sub-section of a srfi-4 `u8vector`. It aims to provide a somewhat functional interface to stateful vector operations with known bounds. Using slices avoids manually juggling vector indices.

## Implementation 

A `slice` is a three-item Scheme list with the following fields:

1. A reference to a pre-allocated `u8vector`
2. A `fixnum` indicating how many bytes have already been written to the `u8vector`.
3. A `fixnum` indicating how many bytes should be written to the `u8vector` in the next `slice` operation.

These fields are available via `(car)`, `(cadr)`, `(caddr)` or the respective accessors:

1. `(slice-u8vector)` 
2. `(slice-bytes-written)` 
3. `(slice-bytes-to-write)`

Example of a slice with three bytes already written, and three more to write:

```scheme
(#u8(144 72 127 0 0 0) 3 3)
```

A given slice is considered "reusable", so running builtin operations over it will not "consume" the slice and automatically increment the `(slice-bytes-written)` field. The `(slice-end-pos)` function returns a total bytes written count inclusive of the current slice.

## Practical use

Use slices when:

- The underlying `u8vector` is long-lived, being written over many times.
- One always starts writing from the beginning of the `u8vector`.
- Every slice write operation is of a known size.
- One intends to make use of the entire slice size.

In other words: this is for low-level contiguous byte buffers.

Get a `slice` over a `u8vector` with:

```scheme
(make-slice initial-slice-size u8vector)
```

This will return a slice at the head of the vector.

```scheme
(make-slice 3 #u8(0 0 0 0 0 0))
  (#u8(0 0 0 0 0 0) 0 3)
```

A zero value is fine for general `u8vector` to `slice` conversion, since it can always be extended later.

One can use `(slice-ref n slice)` and `(slice-set! n new-value slice)` to get/set index `n` of a `slice`. The value of `n` is local to the slice—always starting at zero. 

The following will return index 3 of the underlying `u8vector`:

```scheme
(slice-ref 0 (#u8(144 72 127 11 22 33) 3 3))
  11
```

If `n` is beyond `(slice-bytes-to-write)`, the program will error, even if more `u8vector` space is actually available.

Rather than using `(slice-set!)`, one generally uses `(slice-fill! slice . bytes)` to fill the entire slice with desired bytes:

```scheme
(slice-fill! (#u8(144 72 127 0 0 0) 3 3) 128 72 0)
  (#u8(144 72 127 128 72 0) 3 3)
```

Note how the slice's write position has not changed. One must use `(extend-slice new-bytes-to-write slice)` to return a new slice over the next section of the `u8vector`. This is the fundamental iteration method, similar to `(cdr)` over lists. An error is thrown when the `u8vector` doesn't have enough space to allocate a new `slice`.

```scheme
(extend-slice 3 (#u8(1 2 3 0 0 0) 0 3))
  (#u8(1 2 3 0 0 0) 3 3)
```

Any higher level `fold` functionality is best left to other libraries, since the underlying `u8vector` is not intended to be fully traversed. That is to say: one should `fold` over a list of `slice`-manipulating functions rather than the buffer itself.
