#ifndef VALUE_PARSER_H_INC
#define VALUE_PARSER_H_INC

#include <memory>

#include "json_value.h"
#include "df_automata.h"
#include "number_parser.h"
#include "string_parser.h"

namespace json {

////////////////////////////////////////////////////////////////////////////////
// class_parser
class array_parser : public df_automata<state> {
protected:
        stringstream& sarr_;
        stream_type& stream() { return sarr_; }

        state start;
        state lbracket;
        state value;
        state stop;

        json_array array_;

        handler_func error_handler() {
                return [this](char , stringstream&) -> state& {
                        throw std::runtime_error("Invalid array.");
                        return stop;
                };
        }

        handler_func parse_value(state&);

public:
        array_parser(stringstream& sarr) : sarr_(sarr) {
                // set the second parameter of redirect_next as true to swallow
                // white spaces before redirecting.
                start.add_listener('[', redirect_next(lbracket, true));
                start.default_listener(error_handler());

                lbracket.add_listener(']', redirect_next(stop, true));
                lbracket.default_listener(parse_value(value));

                value.add_listener(',', redirect_next(lbracket, true));
                value.add_listener(']', redirect_next(stop, true));
                value.default_listener(error_handler());

                start_state(start);
                end_state(stop);
        }

        json_array& get() { return array_; }
};

////////////////////////////////////////////////////////////////////////////////
// object_parser
class object_parser : public df_automata<state> {
protected:
        stringstream& sobj_;
        stream_type& stream() { return sobj_; }

        state start;
        state lbrace;
        state key;
        state colon;
        state value;
        state stop;

        json_object object_;
        json_object::key_type cur_key_;

        handler_func error_handler() {
                return [this](char , stringstream&) -> state& {
                        throw std::runtime_error("Invalid object.");
                        return stop;
                };
        }

        handler_func parse_key(state& s) {
                return [this, &s](char, stringstream& ss) -> state& {
                        string_parser parser(ss);
                        parser.run();
                        cur_key_ = parser.get();
                        ss >> std::ws;
                        return s;
                };
        }

        handler_func parse_value(state&);

public:
        object_parser(stringstream& sobj) : sobj_(sobj) {
                start.add_listener('{', redirect_next(lbrace, true));
                start.default_listener(error_handler());

                lbrace.add_listener('}', redirect_next(stop, true));
                lbrace.add_listener('"', parse_key(key));
                lbrace.default_listener(error_handler());

                key.add_listener(':', redirect_next(colon, true));
                key.default_listener(error_handler());

                colon.default_listener(parse_value(value));

                value.add_listener(',', redirect_next(lbrace, true));
                value.add_listener('}', redirect_next(stop, true));
                value.default_listener(error_handler());

                start_state(start);
                end_state(stop);
        }

        json_object& get() { return object_; }
};

////////////////////////////////////////////////////////////////////////////////
// value_parser
class value_parser : public df_automata<state> {
protected:
        stringstream& sval_;
        stream_type& stream() { return sval_; }

        state start;
        state stop;

        json_value value_;

        template <typename Parser>
        handler_func parse_type() {
                return [this](char, stringstream& s) -> state& {
                        Parser parser(s);
                        parser.run();
                        value_ = parser.get();
                        s >> std::ws;
                        return stop;
                };
        }

        handler_func parse_integer() {
                return [this](char, stringstream& s) -> state& {
                        number_parser parser(s);
                        parser.run();
                        if(parser.is_frac)
                                value_ = parser.get<json_real>();
                        else    value_ = parser.get<json_integer>();
                        s >> std::ws;

                        return stop;
                };
        }

        handler_func error_handler() {
                return [this](char , stringstream&) -> state& {
                        throw std::runtime_error("Invalid value.");
                        return stop;
                };
        }

        handler_func parse_and_compare(const std::string& str, json_value v) {
                return [this, str, v](char c, stringstream& s) -> state& {
                        for(char cmp_c : str)
                                if(s.get() != cmp_c)
                                        error_handler()(c, s);
                        value_ = v;
                        s >> std::ws;

                        return stop;
                };
        }

public:
        value_parser(stringstream& sval) : sval_(sval) {
                start.add_listener('"', parse_type<string_parser>());
                start.add_listener('-', parse_integer());
                start.add_listener('0', '9', parse_integer());
                start.add_listener('[', parse_type<array_parser>());
                start.add_listener('{', parse_type<object_parser>());
                start.add_listener('t', parse_and_compare("true", true));
                start.add_listener('f', parse_and_compare("false", false));
                start.add_listener('n', parse_and_compare("null", json_null()));
                start.default_listener(error_handler());

                start_state(start);
                end_state(stop);
        }

        json_value& get() { return value_; }
};


inline array_parser::handler_func array_parser::parse_value(state& to)
{
        return [this, &to](char, stringstream& s) -> state& {
                value_parser vp(s);
                vp.run();
                array_.push_back(vp.get());
                s >> std::ws;
                return to;
        };
}

inline object_parser::handler_func object_parser::parse_value(state& to)
{
        return [this, &to](char, stringstream& s) -> state& {
                value_parser vp(s);
                vp.run();
                object_[cur_key_] = vp.get();
                s >> std::ws;
                return to;
        };
}

}

#endif
