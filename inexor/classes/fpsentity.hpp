#pragma once

/// trigger handler
struct fpsentity : extentity
{
    int triggerstate, lasttrigger;
    fpsentity() : triggerstate(TRIGGER_RESET), lasttrigger(0) {} 
};
