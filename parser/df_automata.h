#ifndef DF_AUTOMATA_H_INC
#define DF_AUTOMATA_H_INC

/*
 * Copyright (c) Shihira Fung, 2015
 */

#include <map>
#include <set>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace automata {

// EventType is assumed to be a finited (small quantity), and scalar (like
// charater, int, enumeration, etc.) type.
template <typename EventType, typename Stream>
class dstate_basic {
public:
    typedef EventType event_type;
    typedef Stream stream_type;
    typedef dstate_basic<event_type, stream_type> self_type;

    typedef std::function<int (event_type, stream_type&)> handler_func;

protected:
    typedef std::map<event_type,
            std::pair<const self_type*, handler_func>> listener_list;

    listener_list listeners_;

    typename listener_list::const_iterator search_(event_type key) const {
        auto i = listeners_.upper_bound(key);
        if(i == listeners_.begin() || !((--i)->second.first))
            return listeners_.end();
        return i;
    }

public:
    void operator=(const self_type&) = delete;

    bool allow_override = true;

    void print_list() const {
        for(auto& i : listeners_) {
            if(isprint(i.first))
                std::cout << i.first;
            else std::cout << int(i.first);

            std::cout << " --> " << i.second.first << std::endl;
        }
    }

    void add_listener(event_type e,
            const self_type& s, handler_func handler = nullptr) {
        add_listener(e, e, s, handler);
    }

    void add_listener(event_type e1, event_type e2,
            const self_type& s, handler_func handler = nullptr) {
        if(e1 > e2) std::swap(e1, e2);

        auto e2_orgi = search_(e2 + 1);
        typename listener_list::mapped_type e2_org =
            e2_orgi == listeners_.end() ?
                typename listener_list::mapped_type(nullptr, nullptr) :
                e2_orgi->second;

        if(allow_override) {
            auto i = listeners_.upper_bound(e1);
            while(i != listeners_.end() && i->first <= e2) {
                i = listeners_.erase(i);
            }
        } else {
            if(search_(e1) != listeners_.end()) {
                throw std::runtime_error(std::string(
                        "DFA doesn't allow overlapped states."));
            }

            if(search_(e2) != listeners_.end()) {
                throw std::runtime_error(std::string(
                        "DFA doesn't allow overlapped states."));
            }
        }

        listeners_[e1].first = &s;
        listeners_[e1].second = std::move(handler);
        listeners_[e2 + 1] = e2_org;
    }

    const self_type* transit(event_type e, stream_type& s) const {
        auto i = search_(e);
        if(i == listeners_.end())
            return nullptr;

        if(i->second.second)
            i->second.second(e, s);

        return i->second.first;
    }

    const self_type* peek_transition(event_type e) const {
        auto i = search_(e);
        if(i == listeners_.end())
            return nullptr;
        return i->second.first;
    }
};

template<typename StreamType>
using basic_dstate = dstate_basic<char, StreamType>;
template<typename StreamType>
using basic_wdstate = dstate_basic<wchar_t, StreamType>;
using dstate = basic_dstate<std::stringstream>;
using wdstate = basic_wdstate<std::wstringstream>;

template<typename C, typename T>
C stream_peek(std::basic_istream<C, T>& is)
{
    return is.peek();
}

template<typename C, typename T>
void stream_consume(std::basic_istream<C, T>& is)
{
    is.get();
}

template <typename State>
class df_automata {
    friend State;

protected:
    bool parse_state_ = true;

public:

    typedef State state_type;
    typedef typename state_type::event_type event_type;
    
    typedef typename state_type::handler_func handler_func;
    typedef typename state_type::stream_type stream_type;

    const state_type* start_state = nullptr;
    std::set<const state_type*> end_states;

    stream_type* stream = nullptr;

    bool is_end(const state_type& s) {
        return end_states.find(&s) != end_states.end();
    }

    bool good() const {
        return parse_state_;
    }

    void run(bool greedy = true) {
        if(!start_state)
            throw std::runtime_error("Bad start state.");
        if(!stream)
            throw std::runtime_error("Bad stream.");
        if(end_states.empty())
            throw std::runtime_error("Automata may never stop.");

        const state_type* cur_state = start_state;

        while(true) {
            event_type cur_event = stream_peek(*stream);
            const state_type* next_state = (cur_event == EOF) ? nullptr :
                cur_state->peek_transition(cur_event);

            //std::cout << cur_event << " transit to " << next_state << std::endl;

            if(!next_state) { // cannot go further
                if(is_end(*cur_state)) {
                    parse_state_ = true;
                    break;
                } else {
                    parse_state_ = false;
                    break;
                }
            } else if(!greedy && is_end(*cur_state)) {
                parse_state_ = true;
                break;
            }

            // till now no changes were made

            stream_consume(*stream);
            cur_state->transit(cur_event, *stream);
            cur_state = next_state;
        }
    }
};

}

#endif
