struct entry {
	string path;
	struct timespec mtime;
};
struct entry read_entry(int inp) {
	struct entry ret;
	smallstring_read(&ret.path, inp);
	read(inp,&ret.mtime,sizeof(ret.mtime));
	return ret;
}
