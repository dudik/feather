default:
	gcc feather.c -std=c99 -pedantic -lX11 -o feather -lfontconfig -lXft -I/usr/include/freetype2
