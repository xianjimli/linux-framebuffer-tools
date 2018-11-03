all:
	$(CC) -g -Wall src/bitmap.c src/linux_fb.c src/gsnap.c -lm -o bin/gsnap
	$(CC) -g -Wall src/bitmap.c src/linux_fb.c src/fbshow.c -lm -o bin/fbshow
	$(CC) -g -Wall src/bitmap.c src/linux_fb.c src/fbdraw.c -lm -o bin/fbdraw

clean:
	rm -f bin/*
