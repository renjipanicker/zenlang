#include "zenlang.hpp"
#include "z/Application.hpp"

void z::application::onExit() const {
    z::Application::OnExit::Handler::_In in;
    z::Application::OnExit::list().runHandler(0, in);
}

z::string z::Application::appDir() {
    return z::app().path();
}

z::string z::Application::baseDir() {
    return z::app().base();
}

z::string z::Application::dataDir() {
    return z::app().data();
}
