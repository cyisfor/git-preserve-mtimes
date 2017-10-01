int main(int argc, char *argv[])
{
	// may be relative, but ok
	const char* exe = argv[0];
	size_t elen = strlen(exe);
	char* s = memrchr(exe,'/',elen);
	if(s) {
		*s = '\0';
	}
	puts(exe);
	
	return 0;
}
