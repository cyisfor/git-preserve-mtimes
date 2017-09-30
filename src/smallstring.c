#include "smallstring.h"

void smallstring_write(int out, const char* s, size_t len) {
	write(out,&len,sizeof(len));
	write(out,s,len);
}

void smallstring_read(string* out, int inp) {
	ensure_eq(sizeof(out->l),read(inp,&out->l,sizeof(out->l)));
	ensure_eq(out->l,read(inp,&out->s,out->l));
}
