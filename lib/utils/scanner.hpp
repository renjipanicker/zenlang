#pragma once

namespace z {
    struct Token {
        z::string text;
    };

    struct Scanner {
    protected:
        Scanner(const int& eofTok);
        virtual ~Scanner();

    public:
        void append(const std::string& in);
        virtual void lex() = 0;
        void done();
        virtual void parse(const int& id, Token* token) = 0;
        inline const int& row() const {return _row;}

    protected:
        enum TokenType {
            ttNormal,
            ttString
        };

    protected:
        Token* getToken(const TokenType& tt = ttNormal);
        void newLine();

    protected:
        int _eofTok;
        std::string _buffer;
        std::string _textbfr;
        std::string::const_iterator _start;
        std::string::const_iterator _text;
        std::string::const_iterator _marker;
        std::string::const_iterator _cursor;
        std::string::const_iterator _limit;
        int  _cond;
        int  _state;
        char _yych;
        int  _yyaccept;
        int _row;
    private:
        z::olist<Token> _tokenList;
    };
}
