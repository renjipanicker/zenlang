#pragma once
#include "base/ast.hpp"

namespace z {
    struct Generator {
        virtual void run() = 0;
        struct Impl {
            inline Impl(const z::Ast::Project& project) : _project(project) {init();}
        private:
            void init();
        protected:
            const z::Ast::Project& _project;
            typedef std::set<z::string> FileList;
            FileList _hppFileList;
            FileList _cppFileList;
            FileList _zppFileList;
            FileList _otherFileList;
            FileList _guiFileList;
            z::string _pch;
            z::string _pchfile;
        };
    };

    struct Indent {
        inline Indent() {
            ind[_indent] = 32;
            _indent += 4;
            ind[_indent] = 0;
        }
        inline ~Indent() {
            ind[_indent] = 32;
            _indent -= 4;
            ind[_indent] = 0;
        }
        inline static const char* get() {return ind;}
        inline static void init() {
            if(_indent < 0) {
                memset(ind, 32, Size);
                _indent = 0;
                ind[_indent] = 0;
            }
        }
    private:
        static const int Size = 1024;
        static char ind[Size];
        static int _indent;
    };
}

#define INDENT z::Indent _ind_
