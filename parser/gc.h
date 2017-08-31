#include <list>

// if 

struct memory_pin {
    std::list<memory_pin*> cond_ref;
};

template<typename T>
struct trace_ptr {
    typedef T value_type;
};


