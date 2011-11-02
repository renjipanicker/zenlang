#pragma once

#include "ast.hpp"

inline std::string err(const Ast::Unit& unit, const Ast::Token& token) {
    std::stringstream msg;
#ifdef _WIN32
    // MSVC style error message
    msg << unit.filename() << "(" << token.row() << ", " << token.col() << "):";
#else
    // GCC style error message, or default.
    msg << unit.filename() << ":" << token.row() << ":";
#endif
    return msg.str();
}
