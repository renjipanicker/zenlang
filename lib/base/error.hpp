#pragma once

#include "base/ast.hpp"

namespace z {
inline z::string zfmt(const z::Ast::Token& token, const z::string& fmt) {
#ifdef _WIN32
    // MSVC style error message
    z::string str = z::string("%{f}(%{r}, %{c}): %{s}").arg("f", token.filename()).arg("r", token.row()).arg("c", token.col()).arg("s", fmt);
#else
    // GCC style error message, or default.
    z::string str = z::string("%{f}:%{r}:%{c}: error: %{s}").arg("f", token.filename()).arg("r", token.row()).arg("c", token.col()).arg("s", fmt);
#endif
    return str;
}
}
