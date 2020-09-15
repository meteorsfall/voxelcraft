#ifndef _MODLOADER_HPP_
#define _MODLOADER_HPP_

#include "utils.hpp"

class Mod {
public:
    Mod(const char* modname);
    Mod(const Mod&) = delete;
    Mod& operator=(const Mod&) = delete;
    ~Mod();

    void call(const char* function_name);
    void set_input_state(void* input_state, int length);
private:
    void* instance;
    void* context;
};

#endif
