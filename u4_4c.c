#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
	FILE    *f1;
	FILE    *f2;
	char    buf[100];
	int     i;

	if(argc != 3)   {
		printf("usage: u1_4a <inputfiles> <outputfile>!\n");
		exit(-1);
	}

	f1 = fopen(argv[1], "rb");

	if(f1 == NULL)  {
		printf("Error: Can't open '%s' for read!\n", argv[1]);
		exit(-1);
	}
	
	f2 = fopen(argv[2], "wt");

	if(f2 == NULL)  {
		printf("Error: can't open '%s' for write!\n", argv[2]);
		exit(-1);
	}
	
	while(!feof(f1))    {
		fgets(buf, 100, f1);

		for(i = 0; i < strlen(buf); i++)    {
			if(islower(buf[i])) {
				buf[i] += 'A'-'a';
			}
		 }
		
		fputs(buf, f2);  /* Puffer in die Ausgabedatei schreiben */
	}

	fclose(f1);
	fclose(f2);
	printf("Programm Ende!\n");

	return 0;
}