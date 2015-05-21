#include <functional>
#include <type_traits>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

// EventType is assumed to be a finited (small quantity), and scalar (like
// charater, int, enumeration, etc.) type.
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
                        if(listener.first(e)) {
                                return listener.second(e, s);
                        }
                        i++;
                }

                return default_listener_(e, s);
        }
};

template <typename State>
class df_automata {
public:
        typedef State state_type;
        typedef typename state_type::event_type event_type;
        typedef typename state_type::handler_func handler_func;
        typedef typename state_type::stream_type stream_type;

private:
        state_type* start_state_ = nullptr;
        state_type* end_state_ = nullptr;

protected:
        void start_state(state_type& s) { start_state_ = &s; }
        void end_state(state_type& s) { end_state_ = &s; }

        static handler_func redirect_to(state_type& to) {
                return [&](event_type, stream_type&)
                        -> state_type& { return to; };
        }

        virtual stream_type& stream() = 0;

public:
        void run() {
                state_type* cur_state = start_state_;

                while(cur_state != end_state_) {
                        event_type cur_event = stream().get();
                        if(stream().eof()) break;
                        cur_state = &(cur_state->transit(cur_event, stream()));
                }
        }
};

#define member(func) (std::bind( \
        &std::remove_pointer<decltype(this)>::type::func, \
        this, std::placeholders::_1, std::placeholders::_2))
        

typedef state_basic<char, std::stringstream> state;

using namespace std;

class re_example : public df_automata<state> {
        // ab+c*a?
        //                        c
        //                       ( )
        //                     ,- 4-,
        //                   ,c     |
        //  1 --a-> 2 --b-> 3 ----> 5 ----> 6
        //   \      |      ( )      \     /
        //    `-> error     b        `-a-'

        state start;
        state after_a;
        state loop_b;
        state loop_c;
        state after_c;
        state finished;

        stringstream str;

protected:
        stream_type& stream() { return str; }

        state& error_handler(char, stringstream&) {
                success = false;
                return finished;
        }

public:
        bool success = true;

        re_example(string s) : str(s) {

                start.add_listener('a', redirect_to(after_a));
                start.default_listener(member(error_handler));
                after_a.add_listener('b', redirect_to(loop_b));
                after_a.default_listener(member(error_handler));
                loop_b.add_listener('b', redirect_to(loop_b));
                loop_c.add_listener('c', redirect_to(loop_c));
                loop_b.default_listener(redirect_to(after_c));
                loop_c.add_listener('c', redirect_to(loop_c));
                loop_c.default_listener(redirect_to(after_c));
                after_c.add_listener('a', redirect_to(finished));
                after_c.default_listener(redirect_to(finished));

                start_state(start);
                end_state(finished);
        }
};

int main()
{
        re_example ree("abbbbba");
        ree.run();
        cout << ree.success << endl;
}
