#include "smallstring.h"
#include "ensure.h"
#include <unistd.h> // write


void smallstring_write(int out, const char* s, size_t len) {
	write(out,&len,sizeof(len));
	write(out,s,len);
}

void smallstring_read(string* out, int inp) {
	// null byte needed because linux sucks
	ensure_eq(sizeof(out->l),read(inp,&out->l,sizeof(out->l)));
	char* s = realloc((char*)out->s, out->l+1);
	ensure_eq(out->l,read(inp,s,out->l));
	s[out->l] = '\0';
	out->s = s;
}
