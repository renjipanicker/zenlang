#pragma once

#include "ast.hpp"

inline std::string err(const std::string& filename, const Ast::Token& token) {
    std::stringstream msg;
#ifdef _WIN32
    // MSVC style error message
    msg << filename << "(" << token.row() << ", " << token.col() << "):";
#else
    // GCC style error message, or default.
    msg << filename << ":" << token.row() << ":";
#endif
    return msg.str();
}
