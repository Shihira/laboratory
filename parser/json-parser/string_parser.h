#ifndef STRING_PARSER_H_INC
#define STRING_PARSER_H_INC

/*
 * Copyright (c) Shihira Fung, 2015
 */

#include "../df_automata.h"

namespace json {

using std::stringstream;
using namespace automata;

class string_parser : public df_automata<state> {
protected:
        state start;
        state reading;
        state backslash;
        state stop;

        stringstream& sstr_;
        stringstream result_;

        virtual stream_type& stream() { return sstr_; }

        handler_func error_handler() {
                return [&](char , stringstream&) -> state& {
                        throw std::runtime_error("Invalid string.");
                        return stop;
                };
        }

        handler_func save_char(state& to, char c = 0) {
                return [&to, c, this](char read_c, stringstream& s) -> state& {
                        s.ignore();

                        if(!c) result_ << read_c;
                        else result_ << c;
                        return to;
                };
        }

        handler_func save_as_utf8(state& to) {
                return [&](char, stringstream& s) -> state& {
                        s.ignore();

                        wchar_t wc = 0;
                        for(int i = 0; i < 4; i++) {
                                wc *= 16;
                                char cur_c = s.get();
                                if(!isxdigit(cur_c))
                                        return error_handler()(cur_c, s);
                                if(cur_c >= 'a' && cur_c <= 'f')
                                        wc += cur_c - 'a' + 10;
                                else if(cur_c >= 'A' && cur_c <= 'F')
                                        wc += cur_c - 'A' + 10;
                                else
                                        wc += cur_c - '0';
                        }

                        char u8_char[4] = {0};
                        if(wc < 0x0080) {
                                u8_char[0] = wc;
                        } else if(wc < 0x0800) {
                                u8_char[0] = 0xc0 | ((wc & 0xffff) >> 6);
                                u8_char[1] = 0x80 | ((wc & 0x003f));
                        } else {
                                u8_char[0] = 0xe0 | ((wc & 0xffff) >> 12);
                                u8_char[1] = 0x80 | ((wc & 0x0fff) >> 6);
                                u8_char[2] = 0x80 | ((wc & 0x003f));
                        }

                        result_ << u8_char;

                        return to;
                };
        }

public:
        string_parser(stringstream& sstr) : sstr_(sstr) {
                // state transition table
                start.add_listener('\"', redirect_next(reading));
                start.default_listener(error_handler());

                reading.add_listener('\\', redirect_next(backslash));
                reading.add_listener('\"', redirect_next(stop));
                reading.default_listener(save_char(reading));

                backslash.add_listener('\"', save_char(reading, '\"'));
                backslash.add_listener('\\', save_char(reading, '\\'));
                backslash.add_listener('/', save_char(reading, '/'));
                backslash.add_listener('b', save_char(reading, '\b'));
                backslash.add_listener('f', save_char(reading, '\f'));
                backslash.add_listener('n', save_char(reading, '\n'));
                backslash.add_listener('r', save_char(reading, '\r'));
                backslash.add_listener('t', save_char(reading, '\t'));
                backslash.add_listener('u', save_as_utf8(reading));
                backslash.default_listener(error_handler());

                start_state(start);
                end_state(stop);
        }

        std::string get() { return result_.str(); }
};

}

#endif
