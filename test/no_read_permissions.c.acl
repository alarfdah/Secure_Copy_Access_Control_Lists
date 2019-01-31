#include <stdio.h>
#include <stdlib.h>

FILE *f = fopen("textfile.txt", "rb");
fseek(f, 0, SEEK_END);
long fsize = ftell(f);
fseek(f, 0, SEEK_SET);  //same as rewind(f);

char *string = malloc(fsize + 1);
fread(string, fsize, 1, f);
fclose(f);

string[fsize] = 0;
