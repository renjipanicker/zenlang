#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "scanner.hpp"

z::Scanner::Scanner(const int& eofTok) : _eofTok(eofTok), _cond(0), _state(0), _yych(0), _yyaccept(0), _row(1) {
    _buffer = "";
    _start   = _buffer.begin();
    _text    = _buffer.begin();
    _marker  = _buffer.begin();
    _cursor  = _buffer.begin();
    _limit   = _buffer.end();
}

z::Scanner::~Scanner() {
}

void z::Scanner::newLine() {
    ++_row;
}

z::Token* z::Scanner::getToken(const TokenType& tt) {
    size_t idx = _start - _buffer.begin();
    assert(_cursor >= _start);
    size_t len = _cursor - _start;
    if(tt == ttString) {
        idx = _text - _buffer.begin();
        len = _cursor - _text - 1;
    }

    const std::string rv = _textbfr + _buffer.substr(idx, len);
    _textbfr = "";
    z::Token* t = new Token();
    _tokenList.add(t);
    t->text = z::e2s(rv);
    return t;
}

void z::Scanner::append(const std::string& in) {
    size_t startIndex  = _start  - _buffer.begin();
    size_t textIndex   = 0;
    if(_text > _start) {
        _textbfr += std::string(_text, _start);
        textIndex = _text - _start;
    }
    size_t markerIndex = 0;
    if(_marker > _start) {
        markerIndex = _marker - _start;
    }
    size_t cursorIndex = 0;
    assert(_cursor >= _start);
    if(_cursor > _start) {
        cursorIndex = _cursor - _start;
    }

    _buffer = _buffer.substr(startIndex) + in;

    _start  = _buffer.begin(); // + startIndex;
    _text   = _buffer.begin() + textIndex;
    _marker = _buffer.begin() + markerIndex;
    _cursor = _buffer.begin() + cursorIndex;
    _limit  = _buffer.end();
    assert(_cursor >= _start);
}

void z::Scanner::done() {
    parse(_eofTok, 0);
}
