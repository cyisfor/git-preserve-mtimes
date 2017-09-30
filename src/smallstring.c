#include "smallstring.h"

void smallstring_write(int out, const char* s, size_t len) {
	write(out,&len,sizeof(len));
	write(out,s,len);
}

smallstring_read(int inp) {
	string ret;
	ensure_eq(sizeof(ret.l),read(inp,&ret.l,sizeof(ret.l)));
	ensure_eq(ret.l,read(inp,&ret.s,ret.l));
	return ret;
}
