#pragma once
#include "gui/Widget.hpp"

namespace Widget {
template <typename T>
inline T& impl(const Widget::Handle& widget) {
    Widget::Handle::Impl* wdata = widget.wdata;
    T* wh = dynamic_cast<T*>(wdata);
    return z::ref(wh);
}
}
