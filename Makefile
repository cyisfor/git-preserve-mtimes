VPATH = o src

PKG_CONFIG_PATH:=libgit2/
export PKG_CONFIG_PATH

CFLAGS+=-ggdb -fdiagnostics-color=always -I. -Io
CFLAGS+=-fshort-enums

P=libgit2
LDLIBS+=$(subst -lgit2,,$(shell pkg-config --static --libs $(P)))

CFLAGS+=$(patsubst -I%,-isystem%, $(shell pkg-config --static --cflags $(P)))
all: store restore installer

LINK=$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
COMPILE=$(CC) $(CFLAGS) -MMD -MT $@ -c -o $@ $<

# bleah, accumulation side effects...
O=$(patsubst %,o/%.o,$N) \
$(foreach name,$(N),$(eval targets:=$$(targets) $(name)))
S=$(patsubst %,src/%.c,$N)

targets:=$(targets) dbstuff

N=http-parser
$O: libgit2/deps/http-parser/http_parser.c
	$(COMPILE)

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

N=store repo note dbstuff http-parser
store: $O intern/libintern.a libgit2/libgit2.a
	$(LINK)

N=restore repo note dbstuff http-parser
restore: $O libgit2/libgit2.a
	$(LINK)

intern/libintern.a: intern/CMakeCache.txt | intern
	$(MAKE) -C intern

libgit2/Makefile: libgit2/CMakeLists.txt | libgit2
	cd libgit2 && cmake \
		-DBUILD_SHARED_LIBS=OFF \
		-DTHREADSAFE=OFF \
		-DBUILD_CLAR=OFF

libgit2/libgit2.a: libgit2/Makefile
	$(MAKE) -C libgit2

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

libgit2 intern:
	bash setup.sh

o:
	mkdir $@

-include $(patsubst %, o/%.d,$(targets))
