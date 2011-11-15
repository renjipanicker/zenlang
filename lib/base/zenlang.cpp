#include "pch.hpp"
#include "zenlang.hpp"

template <typename InitT>
struct InitList {
    inline InitList() : _head(0), _tail(0), _next(0) {}

    inline void push(InitT* inst) {
        if(_tail == 0) {
            assert(_head == 0);
            _head = inst;
            _next = _head;
        } else {
            ref(_tail)._next = inst;
        }
        _tail = inst;
    }

    inline InitT* next() {
        InitT* n = _next;
        if(n != 0) {
            _next = ref(_next)._next;
        }
        return n;
    }

private:
    InitT* _head;
    InitT* _tail;
    InitT* _next;
};

#if defined(UNIT_TEST)
static InitList<TestInstance> s_testList;
TestInstance::TestInstance() : _next(0) {
    s_testList.push(this);
}
#endif

static InitList<MainInstance> s_mainList;

MainInstance::MainInstance() : _next(0) {
    s_mainList.push(this);
}

CallContext g_context = CallContext();
CallContext& CallContext::get() {
    return g_context;
}

void CallContext::run() {
    while(_list.size() > 0) {
        FunctionList::Ptr ptr;
        if(_list.pop(ptr))
            ptr->run();
    }
}

#if defined(Z_EXE)
int main(int argc, char* argv[]) {

#if defined(UNIT_TEST)
    TestInstance* ti = s_testList.next();
    while(ti != 0) {
        ref(ti).enque(g_context);
        ti = s_testList.next();
    }
#endif

    ArgList argl;
    MainInstance* mi = s_mainList.next();
    while(mi != 0) {
        ref(mi).enque(g_context, argl);
        mi = s_mainList.next();
    }

    g_context.run();
    return 0;
}
#endif
