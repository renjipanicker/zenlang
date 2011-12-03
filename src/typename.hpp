#pragma once

struct GenMode {
    enum T {
        Normal,
        Import,
        TypeSpecMemberRef
    };
};

std::string getTypeSpecName(const Ast::TypeSpec& typeSpec, const GenMode::T& mode, const std::string& sep = "::");
std::string getQualifiedTypeSpecName(const Ast::QualifiedTypeSpec& qtypeSpec, const GenMode::T& mode, const std::string& sep = "::");
