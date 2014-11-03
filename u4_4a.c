#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
	FILE    *f1;
	FILE    *f2;
	char    buf[100];
	int     i;

	// Hier wird ueberprueft, ob eine Ein- und Ausgabedatei als Parameter in der Konsole uebergeben wurde
	// Falls nicht wird das Programm sofort beendet
	if(argc != 3)   {
		printf("usage: u1_4a <inputfiles> <outputfile>!\n");
		exit(-1);
	}


	// Hier wird der erste KonsolenParameter als Dateiname interpretiert und im Lese- und Textmodus geoeffnet, d.h. beim Lesen wird nur 'lesbare' Zeichen beachtet
	f1 = fopen(argv[1], "rt");

	// Falls die Datei nicht geoeffnet wurde konnte, wird ein Fehler ausgegeben und das Programm abermals sofort beendet.
	if(f1 == NULL)  {
		printf("Error: Can't open '%s' for read!\n", argv[1]);
		exit(-1);
	}

	// Hier wird der zweite KonsolenParameter wieder Dateiname interpretiert und im Schreib- und Textmodus geoeffnet, d.h. die Datei mit diesem Namen wird neu erstellt (bzw. falls vorhanden, wird der Inhalt geloescht)
	f2 = fopen(argv[2], "wt");

	if(f2 == NULL)  {
		printf("Error: can't open '%s' for write!\n", argv[2]);
		exit(-1);
	}

	// Hier wird die LeseDatei solange durchlaufen bis das Dateiende erreicht (-> feof = end of file)
	while(!feof(f1))    {
		// hier werden die naechsten 100 Zeichen (bzw. bis zum Zeilenumbruch) in das Array buf gelesen
		fgets(buf, 100, f1);

		// es wird buf komplett durchlaufen und jeder Kleinbuchstabe in einen Grossbuchstaben umgewandelt
		for(i = 0; i < strlen(buf); i++)    {
			if(islower(buf[i])) {
				buf[i] += 'A'-'a';
			}
		 }

		fputs(buf, f2);  /* Puffer in die Ausgabedatei schreiben */
	}

	// Die Dateien werden geschlossen und zum Lesen/Schreiben für andere Programm freigegeben
	fclose(f1);
	fclose(f2);
	printf("Programm Ende!\n");

	return 0;
}
