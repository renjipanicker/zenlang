typedef void native;

typedef bool native;

typedef byte native;
typedef short native;
typedef int native;
typedef long native;

typedef ubyte native;
typedef ushort native;
typedef uint native;
typedef ulong native;

typedef float native;
typedef double native;

default void = 0;
default bool = false;

default byte = 0;
default short = 0;
default int = 0;
default long = 0;

default ubyte = 0;
default ushort = 0;
default uint = 0;
default ulong = 0;

default float = 0;
default double = 0;

coerce bool => ubyte => byte => ushort => short => uint => int => ulong => long => float => double;

typedef size native;
default size = 0;
coerce int => size;

typedef char native;
typedef string native;
typedef datetime native;

coerce char => string;
coerce int => datetime;

default char = '';
default string = "";
default datetime = 0;

typedef type native;

template <F> future native;
template <F> functor native;

template <V>   pointer native;
template <V>   value   native;

template <V> ptr native;

template <V>   list native;
template <K,V> dict native;
template <K,V> tree native;

public routine void assert(...) native;
public routine void unused(...) native;
public routine void verify(...) native;
public routine void sizeof(...) native;
public routine void length(...) native;

typedef ArgList list<string> native;
public function (int code)main(const ArgList& argl) abstract;
public function (int passed)test() abstract;
