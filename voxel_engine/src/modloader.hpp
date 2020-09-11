#ifndef _MODLOADER_HPP_
#define _MODLOADER_HPP_

class Mod {
public:
    Mod(const char* modname);
    Mod(const Mod&) = delete;
    Mod& operator=(const Mod&) = delete;
    ~Mod();

    void call(const char* function_name);
private:
    void* instance;
};

#endif
