#include <functional>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

template <typename EventType, typename Stream>
class state_basic {
public:
        typedef EventType event_type;
        typedef Stream stream_type;
        typedef state_basic<event_type, stream_type> self_type;

        typedef std::function<bool (event_type)> handle_pred;
        typedef std::function<self_type& (event_type, stream_type&)> handler_func;

protected:
        typedef std::vector<std::pair<handle_pred, handler_func>> listener_list;

        listener_list listeners_;
        handler_func default_listener_;

public:
        void add_listener(handle_pred pred, handler_func handler) {
                handle_pred func_p = pred;
                handler_func func_h = handler;

                listeners_.push_back(std::make_pair(
                        std::move(pred), std::move(handler)));
        }

        template <typename EqualTo = std::equal_to<event_type>>
        void add_listener(event_type e, handler_func handler) {
                add_listener([=](event_type E) -> bool
                        { return EqualTo()(e, E); },
                        handler);
        }

        template <typename GreaterEqual = std::greater_equal<event_type>,
                 typename LessEqual = std::less_equal<event_type>>
        void add_listener(event_type e1, event_type e2, handler_func handler) {
                add_listener([&](event_type e) -> bool
                        { return GreaterEqual()(e, e1) && LessEqual(e, e2); },
                        handler);
        }

        void default_listener(handler_func handler) {
                default_listener_ = handler;
        }

        self_type& transit(event_type e, stream_type& s) {
                int i = 0;
                for(auto listener : listeners_) {
                        printf("%d ", i);
                        if(listener.first(e)) {
                                return listener.second(e, s);
                        }
                        i++;
                }

                printf(":-( ");
                return default_listener_(e, s);
        }
};
        

typedef state_basic<char, std::stringstream> state;

using namespace std;

state start;
state after_a;
state loop_b;
state loop_c;
state after_c;
state finished;

state::handler_func transit_to(state& s) {
        return [&](char, stringstream&) -> state& { return s; };
}

int main()
{
        // ab+c*a?
        //                        c
        //                       ( )
        //                     ,- 4
        //                   ,c    \
        //  1 --a-> 2 --b-> 3 ----> 5 ----> 6
        //   \      |      ( )      \     /
        //    `-> error     b        `-a-'

        bool success = true;

        auto error_handler = [&](char, stringstream&) ->state&
                { success = false; return finished; };

        start.add_listener('a', transit_to(after_a));
        start.default_listener(error_handler);
        after_a.add_listener('b', transit_to(loop_b));
        after_a.default_listener(error_handler);
        loop_b.add_listener('b', transit_to(loop_b));
        loop_c.add_listener('c', transit_to(loop_c));
        loop_b.default_listener(transit_to(after_c));
        loop_c.add_listener('c', transit_to(loop_c));
        loop_c.default_listener(transit_to(after_c));
        after_c.add_listener('a', transit_to(finished));
        after_c.default_listener(transit_to(finished));

        stringstream str("abbbbba");

        state* cur_state = &start;
        while(cur_state != &finished) {
                char cur_event = str.get();
                if(str.eof()) break;
                cout << cur_event << endl;
                cur_state = &(cur_state->transit(cur_event, str));
                cout << cur_state << ' ' << &finished << endl;
        }

        cout << success << endl;
}
