CFLAGS+=-ggdb -fdiagnostics-color=always

all: test_git ddate/ddate.o ddate-stub.o

test_git: o/test_git.o o/git.o o/repo.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lgit2

o/%.o: source/%.c | o
	$(CC) $(CFLAGS) -c -o $@ $^

ddate/ddate.o:
	$(MAKE) -C ddate ddate.o

ddate-stub.o: source/ddate-stub.c
	gcc $(CFLAGS) -Iddate -c -o $@ $^

o:
	mkdir $@
