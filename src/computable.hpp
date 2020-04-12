#pragma once

#include <cstdint>
#include <iostream>
#include <functional>
#include <memory>
#include <variant>

#include <glad/glad.h>

#include "wrappers.hpp"

template <typename Child, typename Internal>
class Computable {
public:

    template <typename... Args>
    static std::shared_ptr<Child> create(Args&&... args) {
        return Child::create_impl(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void update(Args&&... args) {
        static_cast<Child*>(this)->update_impl(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void dispatch(Args&&... args) {
        static_cast<Child*>(this)->dispatch_impl(std::forward<Args>(args)...);
    }

protected:
    virtual ~Computable() {}
};