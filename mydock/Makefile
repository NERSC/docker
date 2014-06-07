


all: mydock


mydock: mydock.c
	gcc -o mydock ./mydock.c 

fixperms:
	chgrp docker ./mydock
	chmod g+s ./mydock
	chmod a+rx ./mydock
