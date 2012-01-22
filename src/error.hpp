#pragma once

#include "ast.hpp"

inline z::fmt zfmt(const Ast::Token& token, const std::string& fmt) {
    std::stringstream msg;
#ifdef _WIN32
    // MSVC style error message
    msg << token.filename() << "(" << token.row() << ", " << token.col() << "):" << fmt;
#else
    // GCC style error message, or default.
    msg << token.filename() << ":" << token.row() << ":" << token.col() << ": error:" << fmt;
#endif
    return z::fmt(msg.str());
}
