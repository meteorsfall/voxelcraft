#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include "includes.hpp"

using std::function;
using std::vector;
using std::optional;
using std::nullopt;
using std::pair;
using std::map;
using std::unordered_map;
using std::string;
using std::set;
using std::tuple;
using std::sort;

#define UNUSED(x) ((void)x)
#define len(x) (sizeof(x) / sizeof((x)[0]))
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define dbg(fmt, ...) printf("%20s:%-10d " fmt "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

#define FRAME_TIMER false

typedef unsigned char byte;

size_t hash_ivec3(ivec3 const& key);
size_t hash_ivec3(ivec3 const& key, int nonce);
size_t hash_ivec4(ivec4 const& key);

void write_integer(byte* buffer, unsigned i, int magnitude);

int bit_to_sign(int a);

#endif
