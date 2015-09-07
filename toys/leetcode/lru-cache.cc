#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <map>

using namespace std;

class LRUCache{
public:
        const size_t capacity_;
        unordered_map<int, int> cache_;
        unordered_map<int, size_t> key_order_;
        map<size_t, int> order_key_;

        size_t auto_increment_;

        LRUCache(int capacity)
                : capacity_(capacity),
                auto_increment_(1) { }

        void use(int key) {
                // erase old information
                auto cur_order = key_order_.find(key);
                if(cur_order != key_order_.end()) {
                        order_key_.erase(order_key_.find(key_order_[key]));
                }

                // update time stamp
                key_order_[key] = auto_increment_;
                order_key_[auto_increment_] = key;
                auto_increment_ += 1;
        }

        void remove_earliest() {
                // find the min element
                auto lru = order_key_.begin();
                key_order_.erase(key_order_.find(lru->second));
                cache_.erase(cache_.find(lru->second));
                order_key_.erase(lru);
        }

        int get(int key) {
                auto cache_pair = cache_.find(key);

                if(cache_pair == cache_.end())
                        return -1;
                else {
                        use(key);
                        return cache_pair->second;
                }
        }

        void set(int key, int value) {
                cache_[key] = value;
                use(key);

                if(cache_.size() > capacity_)
                        remove_earliest();
        }

};

template<typename T>
void print_everything(const T& mapping) {
        cout << "{";
        for(auto p: mapping)
                cout << p.first << ": " << p.second << ", ";
        cout << "}" << endl;
}

int main()
{
        LRUCache cache(4);

        cache.set(3, 31);
        cache.set(5, 51);
        cache.set(7, 71);
        cache.set(9, 91);
        cout << cache.get(3) << endl;
        cache.set(2, 21);
        cout << cache.get(5) << endl;
        cache.set(7, 72);
        cache.set(4, 41);
        cout << cache.get(7) << endl;
        cout << cache.get(9) << endl;

        print_everything(cache.cache_);
}
