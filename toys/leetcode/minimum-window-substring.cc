#include <iostream>
#include <unordered_map>
#include <list>

using namespace std;

class Solution {
public:
        string minWindow(string s, string t) {
                unordered_map<char, int> char_count;
                unordered_map<char, int> char_count_min;
                for(char c: t) {
                        if(char_count.find(c) == char_count.end()) {
                                char_count[c] = 0;
                                char_count_min[c] = 1;
                        } else 
                                char_count_min[c]++;
                }

                list<string::iterator> interval;

                size_t char_suff = 0;
                string::iterator min_f = s.end(), min_b = s.end();
                for(auto iter_s = s.begin(); iter_s != s.end(); ++iter_s) {
                        if(char_count.find(*iter_s) == char_count.end())
                                continue;

                        interval.push_back(iter_s);
                        ++char_count[*iter_s];

                        if(char_count[*iter_s] == char_count_min[*iter_s])
                                ++char_suff;

                        while(char_count[*interval.front()] >
                                        char_count_min[*interval.front()]) {
                                --char_count[*interval.front()];
                                interval.pop_front();
                        }

                        if(char_suff == char_count.size() &&
                                ((min_b == s.end() || min_f == s.end()) ||
                                interval.back() - interval.front() <
                                min_b - min_f)) {
                                min_f = interval.front();
                                min_b = interval.back();
                        }
                }

                if(min_f == s.end()) return "";
                else return string(min_f, min_b + 1);
        }
};

int main()
{
        string s, t;

        s = "ADOBECODEBANC",
        t = "ABC";
        cout << Solution().minWindow(s, t) << endl;

        s = "a",
        t = "b";
        cout << Solution().minWindow(s, t) << endl;

        s = "a",
        t = "a";
        cout << Solution().minWindow(s, t) << endl;

        s = "a",
        t = "aa";
        cout << Solution().minWindow(s, t) << endl;

        s = "aa",
        t = "aa";
        cout << Solution().minWindow(s, t) << endl;

        s = "aaaaaaaaaaaabbbbbcdd";
        t = "abcdd";
        cout << Solution().minWindow(s, t) << endl;
}
