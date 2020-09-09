#ifndef _DROP_HPP_
#define _DROP_HPP_

#include "../utils.hpp"
#include "../entity.hpp"
#include "../event.hpp"
#include "player.hpp"

class Drop {
public:
    Entity item;
    float scale = 0.2f;
    Drop(vec3 position);
};

class DropMod {
public:
    DropMod();
private:
    vector<Drop> drops;
};

#endif
