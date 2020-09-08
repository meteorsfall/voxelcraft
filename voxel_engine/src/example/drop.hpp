#ifndef _DROP_HPP_
#define _DROP_HPP_

#include "../utils.hpp"
#include "../entity.hpp"

class Drop {
public:
    Entity item;
    void iterate();
    void render();
};

#endif
