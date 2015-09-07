#include <iostream>
#include <vector>
#include <list>

using namespace std;

class Solution {
public:
        typedef vector<int> ints;

        vector<int> maxSlidingWindow(vector<int>& nums, int k) {
                ints result;
                list<ints::iterator> max_list;

                ints::iterator win_beg = nums.begin(),
                        win_end = win_beg + k;

                for(ints::reverse_iterator i(win_end);
                        i != ints::reverse_iterator(win_beg); i++) {
                        if(max_list.empty())
                                max_list.push_front((i + 1).base());
                        if(*max_list.front() < *i)
                                max_list.push_front((i + 1).base());
                }

                while(win_beg < nums.end()) {
                        result.push_back(*max_list.front());
                        if(win_beg == max_list.front())
                                max_list.pop_front();

                        win_beg ++;
                        win_end ++;

                        if(win_end > nums.end()) break;

                        while(*max_list.back() < *(win_end - 1)
                                && !max_list.empty())
                                max_list.pop_back();
                        max_list.push_back(win_end - 1);

                }

                return result;
        }
};

int main()
{
        {
                vector<int> nums = { };
                vector<int> result = Solution().maxSlidingWindow(nums, 0);
                for(int r : result)
                        cout << r << " "; cout << endl;
        }

        {
                vector<int> nums = { 1, 3, -1, -3, 5, 3, 6, 7 };
                vector<int> result = Solution().maxSlidingWindow(nums, 3);
                for(int r : result)
                        cout << r << " "; cout << endl;
        }

        {
                vector<int> nums = {
                        -41, -38, 2, -46, -17, -60,
                        -56, 32, -26, 59, -39, 28,
                        39, -53, 49, -6, -28, -32,
                        -52, 24, -4, 41, -9, -11,
                        -51, 54, -3, 6, 56, -19 };

                vector<int> result = Solution().maxSlidingWindow(nums, 8);
                for(int r : result)
                        cout << r << " "; cout << endl;
        }
}
