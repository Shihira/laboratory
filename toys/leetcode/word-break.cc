#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

class Solution {
public:
        int longest_term;
        vector<int> table;
        typedef unordered_set<string> dict_t;
        typedef string::const_iterator istr_t;

        bool breakfrom(string& s, istr_t cur, dict_t& dict) {
                int head_pos = cur - s.begin();
                if(cur == s.end())
                        return 1;
                if(table[head_pos] >= 0)
                        return table[head_pos];

                for(auto i = cur; i < s.end() && i - cur < longest_term; i++) {
                        auto result = dict.find(string(cur, i + 1));
                        //cout << string(cur, i+1) << (result == dict.end() ? " notfound" : " found") << endl;
                        if(result == dict.end()) continue;
                        if(!breakfrom(s, i + 1, dict)) continue;

                        table[head_pos] = 1;
                        return 1;
                }

                table[head_pos] = 0;
                return 0;
        }

        bool wordBreak(string s, unordered_set<string>& wordDict) {
                unordered_set<char> char_set;

                longest_term = 0;
                for(auto term : wordDict) {
                        longest_term = max(longest_term, int(term.size()));
                        for(char c : term) char_set.insert(c);
                }

                table.resize(s.size(), -1);
                return breakfrom(s, s.begin(), wordDict);
        }
};

int main()
{
        string s;
        Solution::dict_t dict;

        s = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab";
        dict = {"a","aa","aaa","aaaa","aaaaa","aaaaaa","aaaaaaa","aaaaaaaa","aaaaaaaaa","aaaaaaaaaa","ba"};
        cout << Solution().wordBreak(s, dict) << endl;

        s = "leetcode";
        dict = {"leet", "le", "co", "code"};
        cout << Solution().wordBreak(s, dict) << endl;
}
