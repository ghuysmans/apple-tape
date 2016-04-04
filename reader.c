/*
 * Tape converter (from WAV to binary dump)
 * Original version & explanations: http://www.pagetable.com/?p=32
 * Improvements:
 * - error checking
 * - optional ASCII conversion
 * - stats displayed
 * - comments!
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THRESHOLD (32768/30)


int main(int argc, char *argv[]) {
	int samples, bits=0; //progress
	int last = 0; //to "measure bits" in samples
	int want_positive = 1; //about the current sample
	unsigned char out = 0; //byte to be output
	int sync = 0; //0: waiting <70, 1: waiting [50;70[, 2: reading data
	int ascii = 0;
	int o;
	while ((o=getopt(argc, argv, "a")) != -1) {
		switch (o) {
		case 'a':
			ascii = 1;
			break;
		case '?':
			fprintf(stderr, "usage: %s [-a] <16-bit_mono.wav\n"
				"-a: convert to ASCII\n", argv[0]);
			return 1;
		}
	}
	//TODO enforce mono!
	for (samples=-44; samples<0 && !feof(stdin); samples++)
		getchar(); //discard the WAV header
	while (!feof(stdin)) {
		signed short sample = getchar() | getchar()<<8;
		if (want_positive && sample>THRESHOLD) {
			int delta = samples - last;
			if (delta < 50) {
				if (sync == 2) {
					out = out<<1 | (delta<32 ? 0 : 1);
					if (ascii)
						out &= 0x7F;
					if (!((++bits) & 7))
						putchar(out);
						//old bits will get shifted out naturally
				}
				else if (sync == 1)
					sync++;
			}
			else if (delta<70 && !sync)
				sync++;
			//update state
			want_positive--;
			last = samples;
		}
		else if (!want_positive && sample<-THRESHOLD)
			want_positive++;
		samples++;
	}
	if (bits) {
		int low = bits & 7;
		fprintf(stderr, "Extracted %d bits from %d samples.\n", bits, samples);
		if (low == 7)
			fprintf(stderr, "1 bit is missing.");
		else if (low)
			fprintf(stderr, "%d bits are missing.", 8-low);
		else
			return EXIT_SUCCESS;
		return 4;
	}
	else if (samples > 0) {
		fprintf(stderr, "Data not detected in %d samples.\n", samples);
		return 3;
	}
	else {
		fprintf(stderr, "The input file must be a 16-bit mono WAV!\n");
		return 2;
	}
}
