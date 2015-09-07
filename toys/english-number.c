#include <stdio.h>

const char* teens[] = {
        "ten",
        "eleven",
        "twelve",
        "thirteen",
        "fourteen",
        "fifteen",
        "sixteen",
        "seventeen",
        "eighteen",
        "nineteen",
};

const char* ones[] = {
        "zero",
        "one",
        "two",
        "three",
        "four",
        "five",
        "six",
        "seven",
        "eight",
        "nine",
};

const char* tens[] = {
        "vacant", // not used
        "vacant", // not used
        "twenty",
        "thirty",
        "forty",
        "fifty",
        "sixty",
        "seventy",
        "eighty",
        "ninety",
};

const char* groups[] = {
        "vacant", // not used
        "thousand", // 1,000
        "million", // 1,000,000
        "billion", // 1,000,000,000
        "trillion", // 10^12
        "quadrillion", // 10^15
        "quintillion", // 10^18
        "sextillion",
        "septillion",
        "octillion",
        "nonillion",
        "decillion",
        "undecillion",
        "duodecillion",
        "tredecillion",
        "quattuordecillion",
        "quindecillion",
        "exdecillion",
        "septendecillion",
        "octodecillion",
        "novemdecillion",
        "vigintillion",
};

#define MAX_DIGIT ((int)(3*(sizeof(groups) / sizeof(char*) - 1)))
#define ACTUAL_IDX (len - (MAX_DIGIT - i))

int cmp(int a[3], const char wc[3]/* wildcard */)
{
        int i, result = 1;
        for(i = 0; i < 3 && result; i++)
                if(wc[i] <= '9' && wc[i] >= '0')
                        result &= (a[i] == (wc[i] - '0'));

        return result;
}

int main()
{
        int digit[MAX_DIGIT] = {0}, len = 0;

        char c;
        while((c = getchar()) != EOF &&
                (c <= '9' && c >= '0') &&
                len < MAX_DIGIT) { // not <= : if equal no input is allowed
                digit[len] = c - '0';
                len++;
        }

        // right align
        int i;
        for(i = MAX_DIGIT - 1; i >= 0; i--)
                digit[i] = ACTUAL_IDX >= 0 ? digit[ACTUAL_IDX] : 0;

        // echo
        printf("Interpretation: ");
        for(i = 0; i < MAX_DIGIT; i ++) {
                static int eff = 0;
                if(!digit[i] && !eff && i != MAX_DIGIT - 1)
                        continue;
                if(eff && (MAX_DIGIT - i) % 3 == 0) putchar(',');
                printf("%d", digit[i]);
                eff++;
        }
        putchar('\n');

        for(i = 0; i < MAX_DIGIT; i += 3) {
                static int zero_group = 0;
                if(cmp(digit + i, "000") && zero_group < (MAX_DIGIT / 3 - 1)) {
                        zero_group++;
                        continue;
                }
                else if(cmp(digit + i, "00?"))
                        printf("%s", ones[digit[i+2]]);
                else if(cmp(digit + i, "01?"))
                        printf("%s", teens[digit[i+2]]);
                else if(cmp(digit + i, "0?0"))
                        printf("%s", tens[digit[i+1]]);
                else if(cmp(digit + i, "0??"))
                        printf("%s-%s", tens[digit[i+1]], ones[digit[i+2]]);
                else if(cmp(digit + i, "?00"))
                        printf("%s hundred", ones[digit[i]]);
                else if(cmp(digit + i, "?0?"))
                        printf("%s hundred and %s", ones[digit[i]], ones[digit[i+2]]);
                else if(cmp(digit + i, "?1?"))
                        printf("%s hundred and %s", ones[digit[i]], teens[digit[i+2]]);
                else if(cmp(digit + i, "??0"))
                        printf("%s hundred and %s", ones[digit[i]], tens[digit[i+1]]);
                else if(cmp(digit + i, "???"))
                        printf("%s hundred and %s-%s", ones[digit[i]], tens[digit[i+1]], ones[digit[i+2]]);

                if(i / 3 < MAX_DIGIT / 3 - 1)
                        printf(" %s ", groups[MAX_DIGIT / 3 - i / 3 - 1]);
        }
        putchar('\n');

        return 0;
}

