/*
 * Copyright (c) Shihira Fung, 2015
 */

#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <stdexcept>
#include <cstdint>

#include <iostream>

namespace json {

enum json_type_flags {
        json_type_string,       // std::string
        json_type_integer,      // int
        json_type_real,         // double
        json_type_object,       // std::map<json_value>
        json_type_array,        // std::vector<json_value>
        json_type_boolean,      // bool
        json_type_null,         // json_null
};

class json_value;

////////////////////////////////////////////////////////////////////////////////
// Traits

//// Defined JSON types
typedef std::string json_string;
typedef std::int64_t json_integer;
typedef double json_real;
typedef std::map<json_string, json_value> json_object;
typedef std::vector<json_value> json_array;
typedef bool json_boolean;

struct json_null {
        constexpr json_null() { };
        constexpr json_null(const json_null&) { };

        template<typename T>
        constexpr bool operator==(const T&) const { return false; }
        constexpr bool operator==(const json_null&) const { return true; }

        template<typename T, typename = typename std::enable_if
                <std::is_integral<T>::value>::type>
        constexpr operator T() const { return 0; }
};

static const json_null null;

//// is_json_type helpers
template<typename T> struct is_json_type : std::false_type { };
template<json_type_flags F> struct ijt_base_ : std::true_type
        { enum { type_flag = F }; };
template<> struct is_json_type<json_string > : ijt_base_<json_type_string > { };
template<> struct is_json_type<json_integer> : ijt_base_<json_type_integer> { };
template<> struct is_json_type<json_real   > : ijt_base_<json_type_real   > { };
template<> struct is_json_type<json_object > : ijt_base_<json_type_object > { };
template<> struct is_json_type<json_array  > : ijt_base_<json_type_array  > { };
template<> struct is_json_type<json_boolean> : ijt_base_<json_type_boolean> { };
template<> struct is_json_type<json_null   > : ijt_base_<json_type_null   > { };
//// is_alter: T is an alternative type, instead of a defined json type
template<typename T> struct is_alter : std::true_type { };
template<> struct is_alter<bool> : std::false_type { };
template<> struct is_alter<int64_t> : std::false_type { };
template<> struct is_alter<double> : std::false_type { };

//// class json_type_trait_base_
template<typename Data, typename Storage>
struct json_type_trait_base_ {
        typedef Data data_type;
        typedef Storage storage_type;
        enum {
                type_flag = is_json_type<storage_type>::type_flag,
                enabled = true,
        };
};

//// class json_type_trait : json_type_trait_base_
template<typename T, typename Satisfy = void>
struct json_type_trait { enum { enabled = false }; };

template<typename T>
struct json_type_trait<T,
        typename std::enable_if<is_json_type<T>::value>::type>
        : json_type_trait_base_<T, T> { };

template<typename T>
struct json_type_trait<T,
        typename std::enable_if<std::is_integral<T>::value
                && is_alter<T>::value>::type>
        : json_type_trait_base_<T, json_integer> { };

template<typename T>
struct json_type_trait<T,
        typename std::enable_if<std::is_floating_point<T>::value
                && is_alter<T>::value>::type>
        : json_type_trait_base_<T, json_real> { };

template<> struct json_type_trait<char*, void>
        : json_type_trait_base_<char*, json_string> { };
template<size_t N> struct json_type_trait<char[N], void>
        : json_type_trait_base_<char[N], json_string> { };

// Note, const T and const T* need declare seperatedly
template<typename T>
struct json_type_trait<const T>
        : json_type_trait_base_<const T,
        typename json_type_trait<T>::storage_type> { };
template<typename T>
struct json_type_trait<const T*>
        : json_type_trait_base_<const T*,
        typename json_type_trait<T*>::storage_type> { };

////////////////////////////////////////////////////////////////////////////////
// class json_value

class json_value {
protected:
        // Uses vtable to store the code to call the destructor of different
        // type, preventing inaccurate destruction led by `void*` or `free()`.
        struct storage_notype_ {
                virtual ~storage_notype_() { };
        };

        // storage accept only final type (builtin json type) as parameter.
        template<typename T>
        struct storage_ : storage_notype_ {
                typedef storage_<T> self_type;
                typedef T storage_type;
                enum { type_flag = json_type_trait<T>::type_flag };

                storage_type* data = 0;

                storage_(const storage_type& other)
                        { data = new storage_type(other); }
                virtual ~storage_() { if(data) delete data; };
        };

protected:
        json_type_flags type_;
        storage_notype_* value_ = 0;

public:
        ////////////////////////////////////////////////////////////////////////
        // constructor/destructor
        json_value() { assign(null); }

        template<typename T, typename std::enable_if
                <json_type_trait<T>::enabled, int>::type = 0>
        json_value(const T& other)
                { assign(other); }

        json_value(const json_value& other)
                { copy(other); }

        ~json_value() { if(value_) delete value_; }

        ////////////////////////////////////////////////////////////////////////
        // properties
        json_type_flags type() const { return type_; }

        template <typename T,
                typename trait = json_type_trait<T>,
                typename storage_type = typename trait::storage_type>
        const storage_type& value() const {
                return *(dynamic_cast<storage_<storage_type>*>(value_)->data);
        }

        template <typename T,
                typename trait = json_type_trait<T>,
                typename storage_type = typename trait::storage_type>
        storage_type& value() {
                return *(dynamic_cast<storage_<storage_type>*>(value_)->data);
        }

        // All producing work would be forwarded and done here.
        template<typename T,
                typename trait = json_type_trait<T>,
                typename storage_type = typename trait::storage_type>
        void assign(const T& other) {
                if(value_) delete value_;
                value_ = new storage_<storage_type>(other);
                type_ = static_cast<json_type_flags>(
                                storage_<storage_type>::type_flag);
        }

        void copy(const json_value& other) {
                if(&other == this) return; // important!

                switch(other.type_) {
                case json_type_string : assign(other.vals()); break;
                case json_type_integer: assign(other.vali()); break;
                case json_type_real   : assign(other.valr()); break;
                case json_type_object : assign(other.valo()); break;
                case json_type_array  : assign(other.vala()); break;
                case json_type_boolean: assign(other.valb()); break;
                case json_type_null   : assign(other.valn()); break;
                }
        }

        // type abbreviation
        json_string & vals() { return value<json_string >(); }
        json_integer& vali() { return value<json_integer>(); }
        json_real   & valr() { return value<json_real   >(); }
        json_object & valo() { return value<json_object >(); }
        json_array  & vala() { return value<json_array  >(); }
        json_boolean& valb() { return value<json_boolean>(); }
        json_null   & valn() { return value<json_null   >(); }

        const json_string & vals() const { return value<json_string >(); }
        const json_integer& vali() const { return value<json_integer>(); }
        const json_real   & valr() const { return value<json_real   >(); }
        const json_object & valo() const { return value<json_object >(); }
        const json_array  & vala() const { return value<json_array  >(); }
        const json_boolean& valb() const { return value<json_boolean>(); }
        const json_null   & valn() const { return value<json_null   >(); }

        ////////////////////////////////////////////////////////////////////////
        // operators

        json_value& operator=(const json_value& other)
                { copy(other); return *this; }

        json_value& operator[](json_integer index) {
                if(type_ != json_type_array)
                        throw std::runtime_error("TypeError: Not an array.");
                return value<json_array>()[index];
        }

        json_value& operator[](const json_string& index) {
                if(type_ != json_type_object)
                        throw std::runtime_error("TypeError: Not an object.");
                return value<json_object>()[index];
        }

        template<typename T>
        void operator+=(const T& other) {
                value<T>() += other;
        }

        json_array& operator+=(const json_array& other) {
                json_array& self = value<json_array>();
                self.insert(self.end(), other.begin(), other.end());
                return self;
        }
};

}

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

        json_value z = b;
        z += d["Key3"].vals();

        cout << b.vals() << endl;
        cout << z.vals() << endl;
}
