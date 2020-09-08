#include "event.hpp"

void Event::trigger_event(void* data) {
    for(auto& subscribing_function : subscribers){
        subscribing_function(data);
    }
}

void Event::subscribe(fn_on_trigger subscribing_function) {
    subscribers.push_back(subscribing_function)
}
