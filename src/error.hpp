#pragma once

#include "ast.hpp"

inline z::Exception err(const std::string& src, const Ast::Token& token, const char* fmt, ...) {
    va_list vlist;
    va_start(vlist, fmt);
    std::string txt = z::ssprintfv(fmt, vlist);
    va_end(vlist);

    std::stringstream msg;
#ifdef _WIN32
    // MSVC style error message
    msg << token.filename() << "(" << token.row() << ", " << token.col() << "):" << txt;
#else
    // GCC style error message, or default.
    msg << token.filename() << ":" << token.row() << ":" << token.col() << ": error:" << txt;
#endif
    return z::Exception(src, z::fmt("%{s}").add("s", msg.str()));
}
