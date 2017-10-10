VPATH = o src

P=libgit2
PKG_CONFIG_PATH:=/custom/libgit2/lib/pkgconfig
export PKG_CONFIG_PATH

CFLAGS+=-ggdb -fdiagnostics-color=always $(patsubst -I%,-isystem%, $(shell pkg-config --cflags $(P))) -I. -Io
CFLAGS+=-fshort-enums
LDLIBS+=$(shell pkg-config --libs $(P))

all: store restore installer

LINK=$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
COMPILE=$(CC) $(CFLAGS) -MMD -MT $@ -c -o $@ $<

# bleah, accumulation side effects...
O=$(patsubst %,o/%.o,$N) \
$(foreach name,$(N),$(eval targets:=$$(targets) $(name)))
S=$(patsubst %,src/%.c,$N)

targets:=$(targets) dbstuff

prepare.gen.h prepare.gen.c o/prepare.gen.c o/prepare.gen.h: make-prepare src/prepare.sql | o
	./make-prepare o/prepare.gen.h < src/prepare.sql

o/db.sql.gen.c: src/db.sql data_to_header_string/pack
	name=db_sql ./data_to_header_string/pack <src/db.sql > $@.temp
	mv $@.temp $@

o/%.d: src/%.c | o
	$(CC) $(CFLAGS) -MM -MG -MT o/$*.o -c -o $@ $<

N=install note
installer: $O
	$(LINK)

N=store repo note dbstuff
store: $O intern/libintern.a
	$(LINK)

N=restore repo note dbstuff
restore: $O
	$(LINK)

intern/libintern.a: intern/CMakeCache.txt | intern
	$(MAKE) -C intern

intern/CMakeCache.txt: | intern
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

intern:
	bash setup.sh

o:
	mkdir $@

-include $(patsubst %, o/%.d,$(targets))
