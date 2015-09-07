#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
        typedef vector<size_t> len_list;

        len_list palindromeLength(const string& org_s) {
                string s = "\xff";
                for(char c: org_s) { s += c; s += -1; }

                len_list palin_radius(s.size(), 0);

                //for(char c: s) cout << c << " "; cout << endl;

                auto ri = palin_radius.begin(), rmax = ri;
                for(auto si = s.begin(); si != s.end(); ++si, ++ri) {
                        // start from 0 to rid invalid iterators
                        size_t& radius = *ri;
                        auto sym = rmax - (ri - rmax);
                        radius = (sym - *sym > rmax - *rmax) ? *sym : sym - (rmax - *rmax);

                        //cout << rmax - palin_radius.begin() << ':' << radius << endl;
                        // find the real radius
                        auto sl = string::reverse_iterator(si + 1) + radius,
                             sl_end = s.rend();
                        auto sr = si + radius,
                             sr_end = s.end();
                        for(; sl != sl_end && sr != sr_end && *sl == *sr;
                                ++sl, ++sr)
                                radius++;
                        /*
                        auto rl = len_list::reverse_iterator(ri + 1),
                             rl_end = rl + radius;
                        auto rr = ri,
                             rr_end = rr + radius;
                        for(; rl != rl_end && rr != rr_end;
                                ++rl, ++rr)
                                *rr = min(size_t(rl_end - rl), max(*rr, *rl));
                        */

                        //for(size_t c: palin_radius)
                        //        cout << c << " "; cout << endl;

                        if(rmax + *rmax < ri + *ri) rmax = ri;
                }

                return palin_radius;
        }

        string shortestPalindrome(string s) {
                len_list parlin_len = palindromeLength(s);
                auto max_pos = parlin_len.begin();
                for(auto i = max_pos; i != parlin_len.end(); ++i) {
                        *i -= 1;
                        if(i - *i != parlin_len.begin()) continue;
                        if(*i > *max_pos) max_pos = i;
                }

                return string(s.rbegin(), string::reverse_iterator(
                        s.begin() + *max_pos)) + s;
        }
};

int main()
{
        string s;

        s = "aacecaaa";
        cout << Solution().shortestPalindrome(s) << endl;

        s = "787898999";
        cout << Solution().shortestPalindrome(s) << endl;

        s = "homemohses";
        cout << Solution().shortestPalindrome(s) << endl;

        s = string(50000, 'a');
        cout << Solution().shortestPalindrome(s) << endl;
}
