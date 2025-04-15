#ifndef SV_H
#define SV_H

#include <stddef.h>


typedef struct sv {
    const char *data;
    size_t len;
} sv;

// Returns a string-view from a 0-terminated C string
sv sv_from_cstr(const char *s);
// Returns an empty string-view
sv sv_null();
// Returns true if the string-view is empty
int sv_is_empty(sv s);
// Returns true if s1 and s2 are equal, false otherwise
int sv_eq(sv s1, sv s2);
// Removes n characters at the left of the string-view
sv sv_chop_left(sv s, size_t n);
// Returns a string-view of the first whitespace separated token
sv sv_chop_tok(sv s);
// Returns true if `s` starts with `prefix`
int sv_starts_with(sv s, sv prefix);
// Tries to parse a hex number from `s`. Sets `success` accordingly.
uint32_t sv_u32_hex(sv s, int *success);


#ifdef SV_IMPLEMENTATION

sv sv_from_cstr(const char *cstr) {
    sv s;
    s.data = cstr;
    s.len = 0;
    while(*(cstr++))
        s.len++;
    return s;
}

void sv_to_cstr(sv s, char *buf, size_t buf_size) {
    if(buf_size == 0) return;
    size_t i;
    for(i = 0; i < s.len && i < buf_size - 1; i++)
        buf[i] = s.data[i];
    buf[i] = 0;
}

sv sv_null() {
    sv s;
    s.data = NULL;
    s.len = 0;
    return s;
}

int sv_is_empty(sv s) {
    return s.len == 0 || s.data == NULL;
}

static int is_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

sv sv_skipspace(sv s) {
    while(s.len && is_space(*s.data)) {
        s.data++;
        s.len--;
    }
    if(s.len == 0)
        s.data = NULL;
    return s;
}

int sv_eq(sv s1, sv s2) {
    if(s1.len != s2.len) return 0;
    for(size_t i = 0; i < s1.len; i++) {
        if(s1.data[i] != s2.data[i])
            return 0;
    }
    return 1;
}

sv sv_chop_left(sv s, size_t n) {
    if(n < s.len) {
        s.data += n;
        s.len -= n;
    } else {
        s = sv_null();
    }
    return s;
}

sv sv_tok(sv s) {
    s = sv_skipspace(s);
    size_t i;
    for(i = 0; i < s.len && !is_space(s.data[i]); i++);
    s.len = i;
    return s;
}

sv sv_chop_tok(sv s) {
    s = sv_skipspace(s);
    while(s.len && !is_space(*s.data)) {
        s.data++;
        s.len--;
    }
    return s;
}

int sv_starts_with(sv s, sv prefix) {
    if(prefix.len > s.len) return 0;
    for(size_t i = 0; i < prefix.len; i++) {
        if(s.data[i] != prefix.data[i])
            return 0;
    }
    return 1;
}

uint32_t sv_u32_hex(sv s, int *success) {
    uint32_t r = 0;
    for(size_t i = 0; i < s.len; i++) {
        r = r * 16;
        char c = s.data[i];
        if(c >= '0' && c <= '9') {
            r += c - '0';
        } else if(c >= 'a' && c <= 'f') {
            r += c - 'a' + 0xa;
        } else if(c >= 'A' && c <= 'F') {
            r += c - 'A' + 0xa;
        } else {
            *success = 0;
            return 0;
        }
    }
    *success = 1;
    return r;
}

#endif /* SV_IMPLEMENTATION */
#endif  /* SV_H */