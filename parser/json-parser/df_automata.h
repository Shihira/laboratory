#ifndef DFA_AUTOMATA_H_INC
#define DFA_AUTOMATA_H_INC

/*
 * Copyright (c) Shihira Fung, 2015
 */

#include <functional>
#include <vector>
#include <iostream>
#include <sstream>

namespace json {

// EventType is assumed to be a finited (small quantity), and scalar (like
// charater, int, enumeration, etc.) type.
template <typename EventType, typename Stream>
class state_basic {
public:
        typedef EventType event_type;
        typedef Stream stream_type;
        typedef state_basic<event_type, stream_type> self_type;

        typedef std::function<bool (event_type)> handle_pred;
        typedef std::function<self_type&
                (event_type, stream_type&)> handler_func;

protected:
        typedef std::vector<std::pair<handle_pred, handler_func>> listener_list;

        listener_list listeners_;
        handler_func default_listener_;

public:
        void operator=(const self_type&) = delete;

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
                add_listener([e1,e2,&handler](event_type e) -> bool
                        { return GreaterEqual()(e, e1) && LessEqual()(e, e2); },
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

typedef state_basic<char, std::stringstream> state;

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

        static handler_func redirect_this(state_type& to) {
                return [&](event_type, stream_type&) -> state_type& {
                        return to;
                };
        }

        static handler_func redirect_next(state_type& to, bool no_ws = false) {
                return [no_ws, &to](event_type, stream_type& s) -> state_type& {
                        s.ignore();
                        if(no_ws) s >> std::ws;
                        return to;
                };
        }

        virtual stream_type& stream() = 0;

public:
        void run() {
                state_type* cur_state = start_state_;

                while(cur_state != end_state_) {
                        stream_type& s = stream();
                        event_type cur_event = s.peek();
                        // if(cur_event == EOF) break;
                        //printf("%c %p->", cur_event, cur_state);
                        cur_state = &(cur_state->transit(cur_event, s));
                        //printf("%p\n", cur_state);
                }
        }
};

}

#endif
