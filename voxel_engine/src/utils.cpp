#include "utils.hpp"

size_t hash_ivec3(ivec3 const& key) {
    // Using random primes
    uint hash = 456818903U;
    for(int i = 0; i < 3; i++) {
        hash = (hash * 832251403U) ^ (((uint)key[i]) * 1349392157U);
    }
    return hash;
}

size_t hash_ivec3(ivec3 const& key, int nonce) {
    return hash_ivec4(ivec4(key.x, key.y, key.z, nonce));
}

size_t hash_ivec4(ivec4 const& key) {
    // Using random primes
    uint hash = 456818903U;
    for(int i = 0; i < 4; i++) {
        hash = (hash * 832251403U) ^ (((uint)key[i]) * 1349392157U);
    }
    return hash;
}

void write_integer(byte* buffer, unsigned index, int integer) {
    integer = abs(integer);
    buffer[index + 2] = integer % 256;
    buffer[index + 1] = (integer >> 8) % 256;
    buffer[index + 0] = (integer >> 16) % 256; 
}

int bit_to_sign(int a) {
    if (a == 1){
        return -1;
    } else {
        return 1;
    }
}