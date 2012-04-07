#pragma once

namespace z {
    struct Token {
        inline Token(const z::string& t, const int& r, const int& c) : text(t), row(r), col(c) {}
        z::string text;
        size_t row;
        size_t col;
    };

    struct Scanner {
    protected:
        enum TokenMode {
            tmNormal,
            tmExtended
        };

    protected:
        Scanner(const int& eofTok);
        virtual ~Scanner();

    public:
        void readFile(const z::string& filename, const z::string& source);
        void readString(const z::string& str);

    private:
        void readStream(std::istream& is);
        void append(const std::string& in);
        virtual void lex() = 0;
        void done();

    protected:
        virtual z::string text(const int& id, const std::string& in);

    protected:
        inline void setExtendedMode() {
            _text = _cursor;
            _tokenMode = tmExtended;
        }

    private:
        virtual void parse(const int& id, Token* token) = 0;

    protected:
        void send(const int& id);
        void newLine();

    protected:
        int _eofTok;
        TokenMode _tokenMode;
        std::string _buffer;
        std::string _textbfr;
        std::string::const_iterator _start;
        std::string::const_iterator _text;
        std::string::const_iterator _marker;
        std::string::const_iterator _cursor;
        std::string::const_iterator _limit;
        std::string::const_iterator _sol;
        int  _cond;
        int  _state;
        char _yych;
        int  _yyaccept;
        int _row;
    private:
        z::olist<Token> _tokenList;
    };
}

#define NEXT() { _start = _cursor; goto yy0; }
