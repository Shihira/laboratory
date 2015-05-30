// cflags: -o <dirname>/../build/parser_test

#include "../number_parser.h"
#include "../string_parser.h"

using namespace std;
using namespace json;

#define TEST_N(str) { \
        try { \
                stringstream s(str); \
                number_parser p(s); \
                p.run();\
                cout << p.get<double>() << endl; \
        } catch(runtime_error& e) { \
                cout << e.what() << endl; \
        } \
}
#define TEST_S(str) { \
        try { \
                stringstream s(str); \
                string_parser p(s); \
                p.run();\
                cout << p.get() << endl; \
        } catch(runtime_error& e) { \
                cout << e.what() << endl; \
        } \
}

int main()
{

        TEST_N("0123");
        TEST_N("123456");
        TEST_N("-1.e89");
        TEST_N("-0.2088111");
        TEST_N("1.0e-89");
        TEST_N("1.289090e89");

        TEST_S("\"Tab:\\tLinefeed:\\nUnicode:\\u3042\"");
        TEST_S("\"Tab:\tLinefeed:\nUnicode:い\"");
}

