VPATH = o src

P=libgit2 sqlite3
PKG_CONFIG_PATH:=/custom/libgit2/lib/pkgconfig
export PKG_CONFIG_PATH

CFLAGS+=-ggdb -fdiagnostics-color=always $(patsubst -I%,-isystem%, $(shell pkg-config --cflags $(P))) -I. -Io
CFLAGS+=-fshort-enums
LDLIBS+=$(shell pkg-config --libs $(P))

all: make-prepare store restore installer

LINK=$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
COMPILE=$(CC) $(CFLAGS) -MMD -MT $@ -c -o $@ $<

# bleah, accumulation side effects...
O=$(patsubst %,o/%.o,$N) \
$(foreach name,$(N),$(eval targets:=$$(targets) $(name)))
S=$(patsubst %,src/%.c,$N)

N=make-prepare db itoa db.sql.gen note
make-prepare: $O
	$(LINK)

targets:=$(targets) dbstuff

prepare.gen.c o/prepare.gen.c: make-prepare src/prepare.sql | o
	./make-prepare <src/prepare.sql > $@.temp
	mv $@.temp $@

o/db.sql.gen.c: src/db.sql data_to_header_string/pack
	name=db_sql ./data_to_header_string/pack <src/db.sql > $@.temp
	mv $@.temp $@

o/%.d: src/%.c
	$(CC) $(CFLAGS) -MM -MG -MT o/$*.o -c -o $@ $<

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
