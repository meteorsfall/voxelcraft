#ifndef _EVENT_HPP_
#define _EVENT_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The type of a subscribing function that will be passed arbitrary data from the event trigger.
using fn_on_trigger = std::function<void(void*)>;

/// The @ref Event class allows the game to react to various events
class Event {
public:
    /// Triggers an event, and passes in any data related to the event
    /**
     * @param data A pointer to information related to the event
     */
    void trigger_event(void* data);
    /// Allows subscribers to react to an Event occurring. 
    /**
     * When the event is trigger, all subscribing_function's will be called in an arbitrary order.
     * @param subscribing_function The function that will be called upon an Event being triggered.
     * A pointer to information related to the event
     * will be passed in as a parameter upon calling the subscribing_function.
     */
    void subscribe(fn_on_trigger subscribing_function);
private:
    vector<fn_on_trigger> subscribers;
};

/**@}*/

#endif
