# name of the program (Minix service)
PROG=lab5

# source code files to be compiled
SRCS = lab5.c timer.c video_gr.c vbe.c utils.c keyboard.c i8042.h sprite.c

# additional compilation flags
# "-Wall -Wextra -Werror -I . -std=c11 -Wno-unused-parameter" are already set
CFLAGS += -pedantic -DLAB5

# list of library dependencies
DPADD += ${LIBLCF}
LDADD += -llcf

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>
