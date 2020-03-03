CC      = gcc
PROF    = 
L_FLAGS = $(PROF)

#Uncomment to compile in Cygwin
#CYGWIN = -DCYGWIN

#IMC2 - Comment out to disable IMC2 support
IMC = 1

W_FLAGS = -Wall -Werror -Wshadow -Wformat-security -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Wstrict-prototypes

C_FLAGS = -g2 $(W_FLAGS) $(PROF)

C_FILES = 11.c act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c bounty.c \
	  build.c clans.c color.c comm.c comments.c const.c db.c editor.c fight.c finfo.c \
	  force.c fskills.c functions.c handler.c hashstr.c hunter.c interp.c \
	  magic.c makeobjs.c marriage.c md5.c misc.c mud_comm.c mud_prog.c newarena.c \
	  pfiles.c planets.c player.c reset.c save.c ships.c shops.c skills.c slicers.c \
	  space.c special.c swskills.c tables.c track.c tech.c update.c

ifdef IMC
   C_FILES := imc.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCSMAUG
endif

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h) 

all:
	$(MAKE) -s swr

swr: $(O_FILES)
ifdef CYGWIN
	rm -f swr
	$(CC) $(L_FLAGS) -o swr.exe $(O_FILES) -lm
	echo "Done compiling mud.";
	chmod g+w swr.exe
	chmod g+w $(O_FILES)
else
	rm -f swr
	$(CC) $(L_FLAGS) -o swr $(O_FILES) -lm
	echo "Done compiling mud.";
	chmod g+w swr
	chmod g+w $(O_FILES)
endif

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
ifdef CYGWIN
	rm -f swr.exe $(O_FILES)
else
	rm -f swr $(O_FILES)
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
