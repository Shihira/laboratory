#ifndef NUMBER_PARSER_H_INC
#define NUMBER_PARSER_H_INC

/*
 * Copyright (c) Shihira Fung, 2015
 */

#include "df_automata.h"

#include <cmath>
#include <stdexcept>

namespace json {

using std::stringstream;

class number_parser : public df_automata<state> {
        friend class value_parser;
protected:
        virtual stream_type& stream() { return snum_; }

        state start;
        state int_sign;
        state int_head;
        state int_part;
        state point;
        state frac_head;
        state frac_part;
        state exp_e;
        state exp_sign;
        state exp_head;
        state stop;

        stringstream& snum_;

        bool is_signed = false;
        bool is_exp_signed = false;
        bool is_frac = false;

        int64_t integer = 0;
        double fraction = 0;
        int fraction_precision = 1;
        int exponent = 0;

        template <typename T>
        handler_func save_digit(T& p_num, state& to) {
                return [&](char n, stringstream& s) -> state& {
                        s.ignore();
                        p_num *= 10;
                        p_num += n - '0';
                        return to;
                };
        }

        handler_func save_digit(bool& p_num, state& to) {
                return [&](char, stringstream& s) -> state& {
                        s.ignore();
                        p_num = true;
                        return to;
                };
        }

        handler_func save_digit(double& p_num, state& to) {
                return [&](char n, stringstream& s) -> state& {
                        s.ignore();
                        p_num += (n - '0') / pow10(fraction_precision);
                        fraction_precision++;
                        return to;
                };
        }

        handler_func error_handler() {
                return [&](char , stringstream&) -> state& {
                        throw std::runtime_error("Invalid number.");
                        return stop;
                };
        }

public:
        typedef number_parser self_type;

        number_parser(stringstream& snum) : snum_(snum) {
                // state transition table
                start.add_listener('-', save_digit(is_signed, int_sign));
                start.default_listener(redirect_this(int_sign));

                int_sign.add_listener('0', redirect_next(int_part));
                int_sign.add_listener('1', '9', save_digit(integer, int_head));
                int_sign.default_listener(error_handler());

                int_head.add_listener('0', '9', save_digit(integer, int_head));
                int_head.default_listener(redirect_this(int_part));

                int_part.add_listener('.', save_digit(is_frac, point));
                int_part.default_listener(redirect_this(frac_part));

                point.add_listener('0', '9', save_digit(fraction, frac_head));
                point.default_listener(error_handler());

                frac_head.add_listener('0', '9', save_digit(fraction, frac_head));
                frac_head.default_listener(redirect_this(frac_part));

                frac_part.add_listener('e', redirect_next(exp_e));
                frac_part.add_listener('E', redirect_next(exp_e));
                frac_part.default_listener(redirect_this(stop));

                exp_e.add_listener('+', redirect_next(exp_sign));
                exp_e.add_listener('-', save_digit(is_exp_signed, exp_sign));
                exp_e.default_listener(redirect_this(exp_sign));

                exp_sign.add_listener('0', '9', save_digit(exponent, exp_sign));
                exp_sign.default_listener(redirect_this(stop));

                start_state(start);
                end_state(stop);
        }

        template<typename NumType> NumType get() {
                double number = integer;

                if(is_frac) {
                        number += fraction;
                        number *= pow10((is_exp_signed ? -1 : 1) * exponent);
                }

                return (is_signed ? -1 : 1) * number;
        }
};

}

#endif
