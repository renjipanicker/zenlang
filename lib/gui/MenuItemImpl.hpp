#pragma once
#include "gui/MenuItem.hpp"

//namespace MenuItem {
//    struct HandleImpl : public Widget::Handle::Impl {
//#if defined(WIN32)
//        inline HandleImpl() : _id(0) {}
//        uint32_t _id;
//#elif defined(GTK)
//        inline HandleImpl() : _menuItem(0) {}
//        GtkWidget* _menuItem;
//#elif defined(OSX)
//        inline HandleImpl() : _id(0) {}
//        uint32_t _id;
//#elif defined(IOS)
//        inline HandleImpl() : _id(0) {}
//        uint32_t _id;
//#else
//#error "Unimplemented GUI mode"
//#endif
//    private:
//        inline HandleImpl(const HandleImpl& /*src*/) {}
//    };
//
//    inline HandleImpl& impl(const Widget::Handle& widget) {return Widget::impl<HandleImpl>(widget);}
//}
