#include "smallstring.h"
#include "ensure.h"
#include <unistd.h> // write
#include <stdint.h>


void smallstring_write(int out, const char* s, uint16_t len) {
	write(out,&len,sizeof(len));
	write(out,s,len);
}

void smallstring_read(string* out, int inp) {
	// null byte needed because linux sucks
	uint16_t len;
	ensure_eq(sizeof(len),read(inp,&len,sizeof(len)));
	char* s = realloc((char*)out->s, len+1);
	ensure_eq(len,read(inp,s,len));
	s[len] = '\0';
	out->l = len;
	out->s = s;
}
