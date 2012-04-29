#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Application.hpp"

namespace ApplicationImpl {
    static z::HandlerList<int, Application::OnExit::Handler> onApplicationExitHandlerList;
}

void z::Application::onExit() {
    ::Application::OnExit::Handler::_In in;
    ApplicationImpl::onApplicationExitHandlerList.runHandler(0, in);
}

void Application::OnExit::addHandler(const int& priority, Handler* handler) {
    unused(priority);
    assert(priority == 0);
    Application::OnExit::add(handler);
    ApplicationImpl::onApplicationExitHandlerList.addHandler(0, handler);
}
