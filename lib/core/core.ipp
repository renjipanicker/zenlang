typedef void native;

typedef bool native;

typedef byte native;
typedef short native;
typedef int native;
typedef long native;

typedef float native;
typedef double native;

coerce bool => byte => short => int => long => float => double;

default bool = false;
default byte = 0;
default short = 0;
default int = 0;
default long = 0;
default float = 0;
default double = 0;

typedef char native;
typedef string native;
typedef date native;
typedef time native;
typedef datetime native;

coerce char => string;
coerce date => datetime;
coerce time => datetime;

default char = '';
default string = "";

typedef iterator native;
typedef type_of native;

template <F> future native;
template <F> functor native;

//typedef any native;

template <V>   list native;
template <K,V> dict native;
template <K,V> tree native;

public routine void assert() native;
public routine void unused() native;
public routine void check() native;

public function ()addHandler();
public function (int code)main(const list<string>& argl);
public function (int passed)test();
