#include "engine.h"

// ~gauresh @NOTE: base type procedure implementations


template<typename T> list<T>
list_make(slice<T> buf) {
	return {
		buf.raw,
		0,
		buf.len
	};
}

template<typename T> void
append(list<T> *l, T value)
{
	assert(l);
	assert((l->len < l->capacity) && "fixed size list buffer overflow");

	l->raw[l->len] = value;
	l->len += 1;
}

template<typename T> void
append_slice(list<T> *l, slice<T> values)
{
	assert(l);
	assert((l->len + values.len) <= l->capacity && "fixed size list buffer overflow");

	if (values.len == 0) {
		return;
	}

	memcpy(
		l->raw + l->len,
		values.raw,
		values.len * sizeof(T)
	);

	l->len += values.len;
}

template<typename T> void
insert_slice(list<T> *l, u64 index, slice<T> values)
{
	assert(l);
	assert(index <= l->len && "insert index out of bounds");
	assert((l->len + values.len) <= l->capacity && "fixed size list buffer overflow");

	if (values.len == 0) {
		return;
	}

	T *dst = l->raw + index;

	memmove(
		dst + values.len,
		dst,
		(l->len - index) * sizeof(T)
	);

	memcpy(
		dst,
		values.raw,
		values.len * sizeof(T)
	);

	l->len += values.len;
}

template<typename T> void
clear(list<T> *l)
{
	l->len = 0;
}
