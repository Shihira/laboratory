#include <iostream>

using namespace std;

class Solution {
public:
        bool isPowerOfTwo(int n) {
                if(n == 0) return false;
                while(!(n & 1)) n >>= 1;
                return n == 1;
        }
};

int main()
{
        cout << Solution().isPowerOfTwo(0) << endl;
        cout << Solution().isPowerOfTwo(8) << endl;
        cout << Solution().isPowerOfTwo(64) << endl;
        cout << Solution().isPowerOfTwo(1073741824) << endl;
        cout << Solution().isPowerOfTwo(2147483648) << endl;
        cout << (int(2147483648) == int(-2147483648)) << endl;
        cout << Solution().isPowerOfTwo(63) << endl;
}
