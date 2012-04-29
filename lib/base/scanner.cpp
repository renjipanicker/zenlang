#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/scanner.hpp"

z::Scanner::Scanner(const int& eofTok) : _eofTok(eofTok), _tokenMode(tmNormal), _cond(0), _state(0), _yych(0), _yyaccept(0), _row(1) {
    _buffer = "";
    _start   = _buffer.begin();
    _text    = _buffer.begin();
    _marker  = _buffer.begin();
    _cursor  = _buffer.begin();
    _limit   = _buffer.end();
    _sol     = _buffer.begin();
}

z::Scanner::~Scanner() {
}

void z::Scanner::newLine() {
    ++_row;
    _sol = _cursor;
}

z::string z::Scanner::text(const int& id, const std::string& in) {
    unused(id);
    return z::e2s(in);
}

void z::Scanner::send(const int& id) {
    size_t idx = _start - _buffer.begin();
    assert(_cursor >= _start);
    size_t len = _cursor - _start;
    if(_tokenMode == tmExtended) {
        assert(_text >= _buffer.begin());
        assert(_text < _cursor);
        idx = _text - _buffer.begin();
        len = _cursor - _text - 1;
        _tokenMode = tmNormal;
    }

    const std::string rv = _textbfr + _buffer.substr(idx, len);
    _textbfr = "";
    z::Token* t = new Token(text(id, rv), _row, _cursor - _sol - len);
    _tokenList.add(t);
    parse(id, t);
}

void z::Scanner::append(const std::string& in) {
    std::string::const_iterator start = _start;
    if(_tokenMode == tmExtended) {
        assert(_text <= start);
        start = _text;
    }
    if(_marker < start) {
        start = _marker;
    }

    assert(start >= _buffer.begin());
    size_t startIndex  = start  - _buffer.begin();

    size_t textIndex = 0;
    if(_tokenMode == tmExtended) {
        textIndex = _text - start;
    }

    size_t markerIndex = 0;
    assert(_marker >= start);
    if(_marker > start) {
        markerIndex = _marker - start;
    }

    size_t cursorIndex = 0;
    assert(_cursor >= start);
    if(_cursor > start) {
        cursorIndex = _cursor - start;
    }

    size_t solIndex = 0;
    assert(_sol >= start);
    if(_sol > start) {
        solIndex = _sol - start;
    }

    if(_buffer.size() > 0) {
        _buffer = _buffer.substr(startIndex);
    }
    _buffer += in;

    _start  = _buffer.begin();
    if(_tokenMode == tmExtended) {
        _text   = _buffer.begin() + textIndex;
    }
    _marker = _buffer.begin() + markerIndex;
    _cursor = _buffer.begin() + cursorIndex;
    _limit  = _buffer.end();
    _sol    = _buffer.begin() + solIndex;

    assert(_cursor >= _start);
    if(_tokenMode == tmExtended) {
        assert(_text >= _buffer.begin());
    }
}

void z::Scanner::done() {
    parse(_eofTok, 0);
}

void z::Scanner::readStream(std::istream& is) {
    while(!is.eof()) {
        char buf[1025];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        std::streamsize got = is.gcount();
        std::string s(buf, (size_t)got);
        if(is.eof()) {
            s += std::string(10, ' ');
        }
        append(s);
        lex();
    }
    done();
}

void z::Scanner::readFile(const z::string& filename, const z::string& source) {
    std::ifstream is;
    is.open(z::s2e(filename).c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        throw z::Exception(source, z::string("Error opening file: %{s}").arg("s", filename));
    }

    readStream(is);
}

void z::Scanner::readString(const z::string& str) {
    std::stringstream is;
    is.str(z::s2e(str).val());
    readStream(is);
}
