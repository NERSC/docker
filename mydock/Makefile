


all: mydock


mydock: mydock.c
	gcc -o mydock ./mydock.c 

install:
	install -m 2775 -g docker mydock $(DESTDIR)/usr/bin/mydock
	
fixperms:
	chgrp docker ./mydock
	chmod g+s ./mydock
	chmod a+rx ./mydock


test:
	@sh ./t/lockdown.t
	@sh ./t/run.t

clean:
	rm mydock
