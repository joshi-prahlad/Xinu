rfserver:	rdserver.o rdopen.o rdclose.o rdread.o rdwrite.o	\
		rddelete.o rdofile.o snderr.o sndok.o
		gcc -o rdserver rdserver.o rdopen.o			\
			rdclose.o rdread.o rdwrite.o rddelete.o		\
			rdofile.o snderr.o sndok.o

rdserver.o:	rdserver.c
		gcc -c rdserver.c

clean:
		rm -f *.o .,* core rdserver

rdopen.o:	rdopen.c
		gcc -c rdopen.c

rdclose.o:	rdclose.c
		gcc -c rdclose.c

rdread.o:	rdread.c
		gcc -c rdread.c

rdwrite.o:	rdwrite.c
		gcc -c rdwrite.c

rddelete.o:	rddelete.c
		gcc -c rddelete.c

rdofile.o:	rdofile.c
		gcc -c rdofile.c

snderr.o:	snderr.c
		gcc -c snderr.c

sndok.o:	sndok.c
		gcc -c sndok.c
