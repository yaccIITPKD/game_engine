#include "engine.h"


funcdef string
string_copy(Arena *arena, string str)
{
	if (!str.len)
		return S("");

	bytes data = alloc_slice(arena, u8, str.len); 
	memcpy(data.raw, str.raw, str.len);

	return string_from_bytes(data);
}

funcdef string
string_from_bytes(bytes data)
{
	return string {
		data.raw, data.len
	};
}


funcdef string
string_concat(Arena *arena, string a, string b)
{
	u64 total_len = a.len + b.len;
	
	bytes data = alloc_slice(arena, u8, total_len);

	memcpy(data.raw, a.raw, a.len);
	memcpy(data.raw + a.len, b.raw, b.len);

	return string_from_bytes(data);
}


// like printf formating, but returns a new string
funcdef string
string_format(Arena *arena, const char *fmt_string, ...)
{
	va_list args;

	va_start(args, fmt_string);
	int len = vsnprintf(nullptr, 0, fmt_string, args);
	va_end(args);

	if (len < 0) return {};

	bytes buf = alloc_slice(arena, u8, len + 1);

	va_start(args, fmt_string);
	vsnprintf((char *)buf.raw, len + 1, fmt_string, args);
	va_end(args);

	return { buf.raw, (u64) len };
}


funcdef string
string_from_cstring(Arena *arena, char *cstring)
{
	bytes data = alloc_slice(arena, u8, strlen(cstring));
	memcpy(data.raw, cstring, data.len);
	return string_from_bytes(data);
}


funcdef slice<string>
strings_from_cstrings(Arena *arena, int count, char **cstrings)
{
	slice<string> result = alloc_slice(arena, string, count);

	for (int i=0; i<count; ++i)
	{
		char *cstring = cstrings[i];
		string str = string_from_cstring(arena, cstring);
		result[i] = str;
	}

	return result;
}

funcdef string
string_to_cstring(Arena *arena, string s)
{
	bytes data = alloc_slice(arena, u8, s.len + 1);
	memcpy(data.raw, s.raw, s.len);
	data[s.len] = '\0';

	return string_from_bytes(data);
}
