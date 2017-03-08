/* Translate204.c            */
/* adapted from Translate.c  */
/* adapted from RKM pp 3-145 */

#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/exec.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <libraries/translator.h>
#include <proto/exec.h>
#include <proto/translator.h>

struct Library *TranslatorBase = 0;
UBYTE *phonemes[500];
WORD rtncode;

extern struct Library *OpenLibrary();

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <text>\n", *argv);
		exit(0);
	}

	if ( (TranslatorBase = (struct Library *)OpenLibrary("translator.library", 0L)) == NULL)
	{
		printf("Can't open the translator library\n");
		exit(-100);
	}

	if ( (rtncode = Translate(argv[1], strlen(argv[1]), (STRPTR)phonemes, 500)) != 0)
		printf("Translator error - %d\n", rtncode);
	else
	{
		printf("\n    Text = %s\n", argv[1]);
		printf("Phonemes = %s\n\n", phonemes);
	}

	if (TranslatorBase != 0)
		CloseLibrary(TranslatorBase);

	exit(0);
}
