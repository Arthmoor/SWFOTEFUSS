#This is the SWFOTEFUSS Makefile.
#This should be usable on pretty much any Linux system.
#Refer to your 'man gcc' for explanations of these options
#To compile in FreeBSD or OpenBSD, use the gmake compiler.

#Change this to reflect whichever compiler you want to use.
CC      = g++
#CC      = clang++

# Automatically detect if we are running in a Cygwin environment
ifeq ($(findstring CYGWIN, $(shell uname -s)), CYGWIN)
    IS_CYGWIN = 1
endif

# Check if libdl is available on the system
HAS_DL := $(shell ld -ldl --verbose >/dev/null 2>&1 && echo yes)

ifeq ($(HAS_DL),yes)
    NEED_DL = -ldl
else
    NEED_DL =
endif

#OS Detection. Used to check for MacOS below.
UNAME_S := $(shell uname -s)

# Set EXPORT_SYMBOLS only if not on Darwin (MacOS)
ifneq ($(UNAME_S), Darwin)
    EXPORT_SYMBOLS = -export-dynamic
endif

#GCC 4.8+ memory checking option. https://github.com/google/sanitizers Does not work in Cygwin.
#These should only be enabled for testing environments. Disable for a live environment.
ifndef IS_CYGWIN
   M_FLAGS = -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer
endif

# These protect against stack smashing and buffer overflows in production.
# We add -D_FORTIFY_SOURCE=2 to perform runtime checks on string/memory functions.
HARDENING_FLAGS = -fstack-protector-strong -D_FORTIFY_SOURCE=2

W_FLAGS = -std=c++20 -Wall -Wshadow -Wformat-security -Winline -Wpointer-arith -Wcast-align -Wredundant-decls

C_FLAGS = -g2 $(W_FLAGS) $(M_FLAGS) $(HARDENING_FLAGS) $(EXPORT_SYMBOLS)
L_FLAGS = $(M_FLAGS) $(HARDENING_FLAGS) -lm -lz $(NEED_DL)

#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g2 -O $(M_FLAGS) $(HARDENING_FLAGS)

C_FILES = 11.c act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c bounty.c \
	  build.c clans.c color.c comm.c comments.c const.c db.c dns.c editor.c fight.c finfo.c \
	  force.c fskills.c functions.c handler.c hashstr.c hotboot.c hunter.c interp.c \
	  magic.c makeobjs.c marriage.c mccp.c misc.c mssp.c mud_comm.c mud_prog.c newarena.c \
	  pfiles.c planets.c player.c reset.c save.c sha256.c ships.c shops.c skills.c slicers.c \
	  space.c special.c swskills.c tables.c track.c tech.c update.c

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h) 

all:
	$(info    Using flags: $(C_FLAGS))
	$(MAKE) -s swr
	$(MAKE) -s dns

# pull in dependency info for *existing* .o files
-include dependencies.d

ifdef IS_CYGWIN
swr: $(O_FILES)
	rm -f swr.exe
	dlltool --export-all --output-def swr.def $(O_FILES)
	dlltool --dllname swr.exe --output-exp swr.exp --def swr.def
	$(CC) -o swr.exe $(O_FILES) swr.exp $(L_FLAGS)
	@echo -e "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo -e "Done compiling mud.";
	chmod g+w swr.exe
	chmod a+x swr.exe
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o swr.exe dependencies.d resolver.exe resolver.o *~

else
swr: $(O_FILES)
	rm -f swr
	$(CC) -export-dynamic -o swr $(O_FILES) $(L_FLAGS)
	@echo -e "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo -e "Done compiling mud.";
	chmod g+w swr
	chmod a+x swr
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o swr dependencies.d resolver resolver.o *~
endif

dns: resolver.o
	rm -f resolver
	$(CC) $(D_FLAGS) -o resolver resolver.o
	@echo -e "Done compiling DNS resolver.";
	chmod g+w resolver
	chmod a+x resolver
	chmod g+w resolver.o

indent:
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES)
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(H_FILES)

indentclean:
	rm *.c~ *.h~

new:
	$(MAKE) indentclean
	$(MAKE) clean
	$(MAKE) all

o/%.o: %.c
	echo "  Compiling $@...";
	$(CC) -c $(C_FLAGS) $< -o $@
