// cflags: -o <dirname>/../build/json_value_test

#include <iostream>

#include "../json_value.h"

using namespace std;
using namespace json;

int main()
{
        json_value a = 2;
        json_value b = "Hello ";
        json_value c = json_array({1, 2});
        json_value d = json_object({
                {"Key1", 123},
                {"Key2", 1.2},
                {"Key3", "World"},
        });
        json_value f = true;

        json_value z = b;
        z += d["Key3"].S_;
        json_value y = a;
        y += json_integer(d["Key2"].R_);

        d["Key3"] = z;
        d["Key2"] = y;

        cout << b.S_ << endl;
        cout << d["Key3"].S_ << endl;
        cout << d["Key2"].I_ << endl;
        cout << f.B_ << endl;
}

