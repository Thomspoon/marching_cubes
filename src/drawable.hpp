#pragma once

// Drawable.hpp
//
// Description: Interface class for drawable objects which uses CRTP

#include "wrappers.hpp"

template <typename Child>
class Drawable {
public:
    template <typename... Args>
    static Child create(Args&&... args) {
        return Child::create(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void update(Args&&... args) {
        static_cast<Child*>(this)->update(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void draw(Args&&... args) {
        static_cast<Child*>(this)->draw(std::forward<Args>(args)...);
    }

protected:
    Drawable() {}
    virtual ~Drawable() {}
};