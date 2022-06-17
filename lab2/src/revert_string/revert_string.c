#include "revert_string.h"
#include "string.h"
#include "stdlib.h"

void RevertString(char *str)
{
        int str_len = strlen(str);
        char* reverted = malloc((str_len + 1) * sizeof(char));
        for (int i = 0, j = str_len - 1; i < str_len; i++, j--) {
                reverted[i] = str[j];
        }
        reverted[str_len] = '\0';
	strcpy(str, reverted);
}


