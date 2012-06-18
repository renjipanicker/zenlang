#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Application.hpp"

namespace ApplicationImpl {
//@    static z::HandlerListS<int, Application::OnExit::Handler> onApplicationExitHandlerList;
}

void z::Application::onExit() {
    ::Application::OnExit::Handler::_In in;
    ::Application::OnExit::list().run(0, in);
}

z::string Application::appDir() {
    return z::app().path();
}

z::string Application::baseDir() {
    return z::app().base();
}

z::string Application::dataDir() {
    return z::app().data();
}
