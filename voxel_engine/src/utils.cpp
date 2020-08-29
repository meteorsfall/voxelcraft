#include "utils.hpp"

void write_integer(byte* buffer, unsigned i, int magnitude){
    magnitude = abs(magnitude);
    buffer[i + 2] = magnitude % 256;
    buffer[i + 1] = (magnitude >> 8) % 256;
    buffer[i + 0] = (magnitude >> 16) % 256; 
}

int bit_to_sign(int a){
    if (a == 1){
        return -1;
    } else {
        return 1;
    }
}