#include <iostream>
#include <stack>

using namespace std;

class Solution {
public:
        enum punct {
                parenthesis = 0,
                bracket = 1,
                brace = 2,
        };

        bool isValid(string s) {
                static const char lpunct[] = { '(', '[', '{' };
                static const char rpunct[] = { ')', ']', '}' };
                stack<punct> pstack;
                
                for(char c : s)
                for(int i = 0; i < 3; i++) {
                        if(c == lpunct[i])
                                pstack.push(static_cast<punct>(i));
                        else if(c == rpunct[i]) {
                                if(pstack.empty() || pstack.top() != i)
                                        return false;
                                pstack.pop();
                        }
                }

                return pstack.empty();
        }
};

int main()
{
        cout << Solution().isValid("{{[()]{{}}}}") << endl;
        cout << Solution().isValid("{{[(){{}}}}") << endl;
        cout << Solution().isValid("]") << endl;
        cout << Solution().isValid("[") << endl;
}
