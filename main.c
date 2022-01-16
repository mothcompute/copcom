#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

const char* cs[3] = {"MOVE", "WAIT", "SKIP"};

unsigned int nlen(char* s, char c) {
	unsigned int r = 0;
	while(s[r] != c) r++;
	return r;
}

int hexint(char* s) {
	long r = 0;
	int n;
	for(int i = 0; i < strlen(s); i++) {
		if(s[i] > 47 && s[i] < 58) n = s[i] - 48;
		else if(s[i] > 64 && s[i] < 71) n = s[i] - 55;
		else if(s[i] > 96 && s[i] < 103) n = s[i] - 87;
		else return -1;
		r <<= 4;
		r |= n;
	}
	return r;
}

int intget(char* s, char c) {
	int l = nlen(s, c);
	char* ns = malloc(l + 1);
	memcpy(ns, s, l);
	int r;
	ns[l] = 0;
	if(*s == 'x') r = hexint(ns + 1);
	else r = atoi(ns);
	free(ns);
	return r;
}

#define wslb(s) (intget(s, ' ') << 8) | ((intget(s + 1 + nlen(s, ' '), ' ') & 0x7F) << 1)

unsigned int toarr(char* d, unsigned int l, char*** line) {
	unsigned int r = 0, c = 0;
	*line = malloc(sizeof(char*));
	for(; c < l; r++) {
		unsigned int sl = nlen(d + c, '\n');
		if(sl == 0) {
			r--;
			c++;
			continue;
		}
		*line = realloc(*line, sizeof(char*) * r+1);
		(*line)[r] = malloc(sl + 1);
		memcpy((*line)[r], d + c, sl);
		(*line)[r][sl] = 0;
		c += sl + 1;
	}

	return r++;
}

int main(int argc, char** argv) {
	if(argc < 2 || !(argc & 1)) exit(!!printf("arg format: [src] [out] [src] [out]\n"));
	for(int ac = 0; ac < argc / 2; ac++) {
		char* srcfile = argv[1 + (ac * 2)];
		char* destfile = argv[2 + (ac * 2)];
		
		printf("; -- %s > %s --\n", srcfile, destfile);

		struct stat ssi, sdi;

		int si = open(srcfile, O_RDONLY);
		if(si == -1) {
			printf("error opening %s: %s\n", srcfile, strerror(errno));
			continue;
		}
		
		int di = open(destfile, O_CREAT | O_RDWR, 0666);
		if(di == -1) {
			printf("error opening %s: %s\n", destfile, strerror(errno));
			if(si != -1) close(si);
			continue;
		}

		fstat(si, &ssi);
		fstat(di, &sdi);

		char* sm = malloc(ssi.st_size);

		read(si, sm, ssi.st_size);

		close(si);

		char** line;
		unsigned int colnum = toarr(sm, ssi.st_size, &line);
		uint16_t* o = malloc(4 * colnum);
		for(unsigned int i = 0; i < colnum; i++) {
			if(strlen(line[i]) < 3) goto err;
			int command = 5;
			for(int c = 0; c < 3; c++) if(*line[i] == "mws"[c]) command = c;
			if(command == 5) goto err;
			if(line[i][1] != ' ') goto err;
			uint16_t l = 0, h = 0;
			switch(command) {
				case 0: // m
					h = (intget(line[i] + 2, ' ') & 0xFF) << 1;
					l = intget(line[i] + 3 + nlen(line[i] + 2, ' '), 0);
					break;
				case 1: // w
					h = (((intget(line[i] + 2, ' ') & 0xFF) << 8)
					| (intget(line[i] + 3 + nlen(line[i] + 2, ' '), ' ')
					& 0x7F) << 1) | 1;
					int c = nlen(line[i] + 2, ' ') + 3;
					l = wslb(line[i] + c + nlen(line[i] + c, ' ') + 1);
					break;
				case 2: // s
					h = (((intget(line[i] + 2, ' ') & 0xFF) << 8)
					| (intget(line[i] + 3 + nlen(line[i] + 2, ' '), ' ')
					& 0x7F) << 1) | 1;
					c = nlen(line[i] + 2, ' ') + 3;
					l = wslb(line[i] + c + nlen(line[i] + c, ' ') + 1) | 1;
					break;
			}

			o[(i<<1)|1] = l;
			o[(i<<1)] = h;

			printf("dc.w $%04X, $%04X; decode %s (%s)\n", h, l, cs[command], line[i]);
		}

		goto end;

err:
		for(unsigned int i = 0; i < colnum; i++) free(line[i]);
		free(line);
		close(di);
		free(sm);
		exit(!!printf("parse err\n"));

end:
		for(unsigned int i = 0; i < colnum; i++) free(line[i]);
		free(sm);
		free(line);
		ftruncate(di, 4 * colnum);
		write(di, o, 4 * colnum);
		close(di);

	}

}
