CC      = gcc
PROF    = 

#Uncomment to compile in Cygwin
#CYGWIN = -DCYGWIN

#Uncomment the line below if you are getting undefined references to dlsym, dlopen, and dlclose.
#Comment it out if you get errors about ldl not being found.
NEED_DL = -ldl

#Some systems need this for dynamic linking to work.
EXPORT_SYMBOLS = -export-dynamic

# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket

#IMC2 - Comment out to disable IMC2 support
IMC = 1

W_FLAGS = -Wall -Werror -Wshadow -Wformat-security -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Wstrict-prototypes
C_FLAGS = -g2 $(W_FLAGS) $(SOLARIS_FLAG) $(PROF) $(EXPORT_SYMBOLS)
L_FLAGS = $(PROF) $(SOLARIS_LINK) -lm -lz $(NEED_DL)

C_FILES = 11.c act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c bounty.c \
	  build.c clans.c color.c comm.c comments.c const.c db.c editor.c fight.c finfo.c \
	  force.c fskills.c functions.c handler.c hashstr.c hotboot.c hunter.c interp.c \
	  magic.c makeobjs.c marriage.c mccp.c misc.c mud_comm.c mud_prog.c newarena.c \
	  pfiles.c planets.c player.c reset.c save.c sha256.c ships.c shops.c skills.c slicers.c \
	  space.c special.c swskills.c tables.c track.c tech.c update.c

ifdef IMC
   C_FILES := imc.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCSMAUG
endif

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h) 

all:
	$(MAKE) -s swr

# pull in dependency info for *existing* .o files
-include dependencies.d

swr: $(O_FILES)
ifdef CYGWIN
	rm -f swr
	dlltool --export-all --output-def swr.def $(O_FILES)
	dlltool --dllname swr.exe --output-exp swr.exp --def swr.def
	$(CC) -o swr.exe $(O_FILES) swr.exp $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w swr.exe
	chmod g+w $(O_FILES)
else
	rm -f swr
	$(CC) -export-dynamic -o swr $(O_FILES) $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w swr
	chmod g+w $(O_FILES)
endif

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
ifdef CYGWIN
	@rm -f swr.exe dependencies.d $(O_FILES)
else
	@rm -f swr dependencies.d $(O_FILES)
endif

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
	echo "  Compiling $@....";
	$(CC) -c $(C_FLAGS) $< -o $@
