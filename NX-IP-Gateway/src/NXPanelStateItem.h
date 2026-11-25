// NXPanelStateItem.h
// Panel state item templates with on-change callbacks, MQTT topics and conversion functions

#ifndef _NXPANELSTATEITEM_h
#define _NXPANELSTATEITEM_h

#include <array>
#include <type_traits>
#include <WString.h>



// Forward declarations
class NXPanelState;
class IPanelStateItem;

// Callback for value changes
typedef void(*IPanelStateItemCallback)(IPanelStateItem* item);

// Base interface for panel state item
class IPanelStateItem {
    private:
    public:       
        constexpr IPanelStateItem(const char* ns_, uint8_t index_, const char* name_, IPanelStateItemCallback callback_ = nullptr) : 
            ns(ns_), index(index_), name(name_), callback(callback_) {}
        const String id() const { return String(ns) + "/" + String(index) + "/" + String(name); }
        virtual String toString() const = 0;
    protected:
        const char* ns;
        uint8_t index;
        const char* name; 
        IPanelStateItemCallback callback;
        void notifyGlobalChange();
};


template<typename T>
class NXPanelStateItem : IPanelStateItem {
private:
    T value;
    bool undef = true;
public:
    constexpr NXPanelStateItem(const char* ns_, uint8_t index_, const char* name_,
                     IPanelStateItemCallback callback_ = nullptr)
        : IPanelStateItem(ns_, index_, name_, callback_) {}

    NXPanelStateItem& operator=(const T& v) {
        if (v != value || undef) {
            undef = false;
            value = v;
            if (callback)
                callback(this);
            notifyGlobalChange();
        }
        return *this;
    }

    inline T get() const { return value; }

    String toString() const override {
        if (undef)
            return "?";
        if constexpr (std::is_same_v<T, bool>)
            return value ? "ON" : "OFF";
        else
            return String(value);
    }

};

class AggregateState : IPanelStateItem {
public:
    constexpr AggregateState(const char* ns_, uint8_t index_, const char* name_,
                     IPanelStateItemCallback callback_ = nullptr)
        : IPanelStateItem(ns_, index_, name_, callback_), value(AggregateStateValue::unknown) {}

    enum class AggregateStateValue {
        unknown, armed_away, armed_custom_bypass, armed_home, armed_night, armed_vacation, arming, disarmed, disarming, pending, triggered
    };

    AggregateState& operator=(const AggregateStateValue& v) {
        if (v != value) {
            value = v;
            if (callback)
                callback(this);
            notifyGlobalChange();
        }
        return *this;
    }

    inline AggregateStateValue get() const { return value; }

    String toString() const override {
        return String(AggregateStateStrings[(size_t)value]);
    }

private:
    AggregateStateValue value;
    static constexpr std::array<const char*, 11> AggregateStateStrings {"", "armed_away", "armed_custom_bypass", "armed_home", "armed_night", "armed_vacation", "arming", "disarmed", "disarming", "pending", "triggered"}; 
};

#endif

