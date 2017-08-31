#define EXPOSE_EXCEPTION

#include "../../common/unit_test.h"
#include "../df_automata.h"

using namespace std;
using namespace automata;
using namespace shrtool::unit_test;

TEST_CASE(test_state)
{
    dstate s1, s2, s3, s4, s5;
    s1.add_listener(-3, '\1', s2);
    s1.add_listener('a', 'd', s2); // a b c d
    s1.add_listener('e', 'h', s1); // e f g h
    s1.add_listener('x', 'z', s4); // x y z
    s1.add_listener('\x7d', '\x7f', s5);

    s1.allow_override = false;
    assert_except(s1.add_listener('c', s3), std::runtime_error);
    assert_except(s1.add_listener('g', 'j', s4), std::runtime_error);

    s1.allow_override = true;
    s1.add_listener('g', 'j', s4); // g h i j
    s1.add_listener('i', s3);

    assert_equal_print(s1.peek_transition(-3), &s2);
    assert_equal_print(s1.peek_transition(-2), &s2);
    assert_equal_print(s1.peek_transition(-1), &s2);
    assert_equal_print(s1.peek_transition(-128), (dstate*)nullptr);
    assert_equal_print(s1.peek_transition('\0'), &s2);
    assert_equal_print(s1.peek_transition('\5'), (dstate*)nullptr);
    assert_equal_print(s1.peek_transition('b'), &s2);
    assert_equal_print(s1.peek_transition('d'), &s2);
    assert_equal_print(s1.peek_transition('i'), &s3);
    assert_equal_print(s1.peek_transition('e'), &s1);
    assert_equal_print(s1.peek_transition('g'), &s4);
    assert_equal_print(s1.peek_transition('h'), &s4);
    assert_equal_print(s1.peek_transition('k'), (dstate*)nullptr);
    assert_equal_print(s1.peek_transition('u'), (dstate*)nullptr);
    assert_equal_print(s1.peek_transition('x'), &s4);
    assert_equal_print(s1.peek_transition('y'), &s4);
    assert_equal_print(s1.peek_transition('\x7d'), &s5);
    assert_equal_print(s1.peek_transition('\x7e'), &s5);
    assert_equal_print(s1.peek_transition('\x7f'), &s5);

    s2.add_listener(0, 127, s3);
    s2.add_listener('{', s4);
    assert_equal_print(s2.peek_transition('}'), &s3);
    assert_equal_print(s2.peek_transition('{'), &s4);
}

TEST_CASE(test_dfa_comment)
{
    /* C Language legacy comment DFA */
    dstate start, pre_star, content, pre_slash, end;

    /*
    cout
        << &start << endl
        << &pre_star << endl
        << &content << endl
        << &pre_slash << endl
        << &end << endl;
    */

    stringstream word("/* {axcd*!x**/blahblah");

    start.add_listener('/', pre_star);
    pre_star.add_listener('*', content);
    content.add_listener(0, 127, content);
    content.add_listener('*', pre_slash);
    pre_slash.add_listener(0, 127, content);
    pre_slash.add_listener('*', pre_slash);
    pre_slash.add_listener('/', end);

    df_automata<dstate> dfa;
    dfa.stream = &word;
    dfa.start_state = &start;
    dfa.end_states.insert(&end);

    dfa.run();

    assert_true(dfa.good());
    assert_true(word.peek() == 'b');
}

int main(int argc, char* argv[])
{
    return test_main(argc, argv);
}

