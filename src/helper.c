#include "string.h"
#include "math.h"
#include "helper.h"

float map(float value, float iupper, float ilower, float oupper, float olower){
   return olower + ((oupper - olower) / (iupper - ilower)) * (value - ilower);
}

void LinkPot(struct parameter_t *p, float value){
    p->value = map(value, 0, 255, p->lowerBound, p->upperBound);
    return;
}

float unLinkPot(struct parameter_t *p){
    return map(p->value, p->lowerBound, p->upperBound, 0, 255);
}

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

int intToStr(int x, char str[], int d)
{
    int i = 0;
    if (x < 0.0f){
        x = -x;
    }
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
 
    str[i] = '\0';
    reverse(str);
    return i;
}

void ftoa(float n, char *res, int afterpoint)
{
    if (n < 0){
        n = -n;
        res[0] = '-';
    }else{
        res[0] = ' ';
    }

    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to string
    int i = intToStr(ipart, res + 1, 1);
    i = i + 1;
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot
 
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
 
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

float atof(const char* s){
	float rez = 0, fact = 1;
	if (*s == '-'){
		s++;
		fact = -1;
	};
	for (int point_seen = 0; *s; s++){
		if (*s == '.'){
			point_seen = 1; 
			continue;
		};
		int d = *s - '0';
		if (d >= 0 && d <= 9){
			if (point_seen) fact /= 10.0f;
			rez = rez * 10.0f + (float)d;
		};
	};
	return rez * fact;
}
