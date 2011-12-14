#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WidgetImpl.hpp"

Widget::Handle::Impl::ChildList& Widget::Handle::_child() const {
    return ref(wdata).childList;
}

