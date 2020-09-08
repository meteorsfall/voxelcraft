#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include "includes.hpp"

/**
 * @mainpage
 *
 * Documenation for @ref VoxelEngine.
 */

/**
 * @defgroup VoxelEngine VoxelEngine
 * 
 * @ref VoxelEngine is the Game Engine that aimes to provide a fast and extensible
 * interface for creating Voxel Worlds
 */

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
// Mod, but works on negatives
#define pos_mod(a, b) ( (((a) % (b)) + (b)) % (b) )

#define HIDE_DOXYEN /// \cond HIDDEN_SYMBOLS

#define UNHIDE_DOXYEN /// \endcond

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define dbg(fmt, ...) printf("%20s:%-10d " fmt "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

#define FRAME_TIMER false

typedef unsigned char byte;

/// This function will hash an ivec3
size_t hash_ivec3(ivec3 const& key);
/// This function will hash an ivec3,
/// with a unique nonce so that this hash doesn't overlap with other hashes of the same cube
size_t hash_ivec3(ivec3 const& key, int nonce);
/// This function will hash an ivec4
size_t hash_ivec4(ivec4 const& key);

/// This function will write the absolute value of an integer to the buffer at index i
void write_integer(byte* buffer, unsigned index, int integer);

int bit_to_sign(int a);

#endif
