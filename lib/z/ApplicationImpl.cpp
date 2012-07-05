#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Application.hpp"

void z::Application::onExit() const {
    ::Application::OnExit::Handler::_In in;
    ::Application::OnExit::list().runHandler(0, in);
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
