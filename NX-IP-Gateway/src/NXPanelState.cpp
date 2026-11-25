// 
// 
// 

#include "NXPanelState.h"

void onStateChangeQ0(IPanelStateItem* item) {
    MQTT.send(item->id(), item->toString(), true, 0);
}

void onStateChangeQ1(IPanelStateItem* item) {
    MQTT.send(item->id(), item->toString(), true, 1);
}

void PartitionStatus::calculateAggregateState()
{
    if (sirenOn.get() || steadySirenOn.get())
        aggregateState = AggregateState::AggregateStateValue::triggered;
    else if (exit1.get() || exit2.get())
        aggregateState = AggregateState::AggregateStateValue::arming;
    else if (entry.get())
        aggregateState = AggregateState::AggregateStateValue::pending;
    else if (armed.get() && entryGuard.get())
        aggregateState = AggregateState::AggregateStateValue::armed_home;
    else if (armed.get() && zoneBypassed.get())
        aggregateState = AggregateState::AggregateStateValue::armed_custom_bypass;
    else if (armed.get())
        aggregateState = AggregateState::AggregateStateValue::armed_away;
    else
        aggregateState = AggregateState::AggregateStateValue::disarmed;

}
