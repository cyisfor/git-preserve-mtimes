VPATH = o

P=libgit2
PKG_CONFIG_PATH:=/custom/libgit2/lib/pkgconfig
export PKG_CONFIG_PATH

CFLAGS+=-ggdb -fdiagnostics-color=always $(patsubst -I%,-isystem%, $(shell pkg-config --cflags $(P))) -I.
LDLIBS+=$(shell pkg-config --libs $(P))

all: store restore installer

LINK=$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
COMPILE=$(CC) $(CFLAGS) -MMD -MT $@ -c -o $@ $<

# bleah, accumulation side effects...
O=$(patsubst %,o/%.o,$N) \
$(foreach name,$(N),$(eval targets:=$$(targets) $(name)))
S=$(patsubst %,src/%.c,$N)

N=install note
installer: $O
	$(LINK)

N=store repo note smallstring
store: $O intern/libintern.a
	$(LINK)

N=restore repo note smallstring
restore: $O
	$(LINK)

intern/libintern.a: intern/CMakeCache.txt
	$(MAKE) -C intern

intern/CMakeCache.txt:
	cd intern && cmake \
		-DMMAP_PAGES=1 \
		-DPAGE_SIZE=512 \
		-DINLINE_UNSIGNED=1 \
		-DBUILD_STATIC=1 \
		-DCMAKE_BUILD_TYPE=Release

o/%.o: src/%.c | o
	$(COMPILE)

clean:
	rm -rf o

o:
	mkdir $@

-include $(patsubst %, o/%.d,$(targets))
