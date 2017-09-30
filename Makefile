VPATH = o

P=libgit2
PKG_CONFIG_PATH:=/custom/libgit2/lib/pkgconfig
export PKG_CONFIG_PATH

CFLAGS+=-ggdb -fdiagnostics-color=always $(patsubst -I%,-isystem%, $(shell pkg-config --cflags $(P)))
LDLIBS+=$(shell pkg-config --libs $(P))

all: hook-store hook-restore

LINK=$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
COMPILE=$(CC) $(CFLAGS) -MMD -MT $@ -c -o $@ $<

# bleah, accumulation side effects...
O=$(patsubst %,o/%.o,$N) \
$(foreach name,$(N),$(eval targets:=$$(targets) $(name)))
S=$(patsubst %,src/%.c,$N)

N=hook-store repo note
hook-store: $O
	$(LINK)

N=hook-restore repo note
hook-restore: $O
	$(LINK)

o/%.o: src/%.c | o
	$(COMPILE)

clean:
	rm -rf o

o:
	mkdir $@

-include $(patsubst %, o/%.d,$(targets))
