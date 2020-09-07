#include "utils.hpp"

size_t hash_ivec3(ivec3 const& key) {
    // Using random primes
    return (((((456818903 + key.x) * 832251403) + key.y) * 1349392157) + key.z) * 1866190769;
}

size_t hash_ivec3(ivec3 const& key, int nonce) {
    return hash_ivec4(ivec4(key.x, key.y, key.z, nonce));
}

size_t hash_ivec4(ivec4 const& key) {
    // Using random primes
    return ((((((456818903 + key.x) * 832251403) + key.y) * 1349392157) + key.z) * 1866190769 + key.w)*74709703;
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