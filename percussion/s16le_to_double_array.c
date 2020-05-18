#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	short sample;
	printf("double %s[] = {", argv[1]);

	while (fread(&sample, 2, 1, stdin) == 1 && fread(&sample, 2, 1, stdin) == 1) { // skip each other for stereo to mono
		printf("\t%f,\n", sample / (32768.0));
	}

	puts("\t2.0,"); // end of sample
	puts("};");
}
