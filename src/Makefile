CC      = g++
#PROF    = -p

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

#Memory debugging flags. Cygwin lacks libasan support required for this to work.
ifndef CYGWIN
M_FLAGS = -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer
endif

W_FLAGS = -std=c++1z -Wall -Wshadow -Wformat-security -Winline -Wpointer-arith -Wcast-align -Wredundant-decls

C_FLAGS = -g2 $(W_FLAGS) $(M_FLAGS) $(SOLARIS_FLAG) $(PROF) $(EXPORT_SYMBOLS)
L_FLAGS = $(M_FLAGS) $(PROF) $(SOLARIS_LINK) -lm -lz $(NEED_DL)
#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g2 -O -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer $(PROF) $(SOLARIS_LINK)

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

ifdef CYGWIN
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
