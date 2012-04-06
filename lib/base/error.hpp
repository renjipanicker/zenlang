#pragma once

#if defined(UN_AMALGAMATED)
#include "base/ast.hpp"
#endif

inline z::string zfmt(const Ast::Token& token, const z::string& fmt) {
#ifdef _WIN32
    // MSVC style error message
    z::string str = z::string("%{f}(%{r}, %{c}): %{s}").arg("f", token.filename()).arg("r", token.row()).arg("c", token.col()).arg("s", fmt);
#else
    // GCC style error message, or default.
    z::string str = z::string("%{f}:%{r}:%{c}: error: %{s}").arg("f", token.filename()).arg("r", token.row()).arg("c", token.col()).arg("s", fmt);
#endif
    return str;
}
