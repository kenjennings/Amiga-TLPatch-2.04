/* TLPatch.c V1.0                   */
/* Translator.library patch utility */
/* Richard Sheppard - Jan. 7, 1991  */
/* Toronto, Canada                  */
/* (source for Lattice C ??)        */

/* TLPatch204.c                     */
/* translator.library patch utility */
/* Kenneth Jennings - Dec. 23, 1994 */
/* Miami, Florida                   */
/* Source for SAS/C 6.51            */


#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <exec/execbase.h>
#include <exec/nodes.h>
#include <proto/exec.h>

void AddFile(void);
void Validate(void);
void Extract(void);
int  GetLine(char line[], int max);
int  GetExcept(char line[], int max);
int  StrIndex(char source[], char searchfor[]);

FILE *fp1, *fp2;					/* input file (exception table), outfile */
FILE *fp3;							/* input file (translator.library)       */
#ifdef DEBUG_ON
FILE *fp4;							/* Debugging file                        */
#endif

char pattern[] = "[A";				/* 1st search pattern                    */
char line[256];						/* line buffer                           */
LONG codesize;						/* size of code & xtbl hunk              */
LONG jmptab[28];					/* jump table array                      */

WORD code[1024];					/* original code at start                */
LONG reloc[45] =					/* reloc table at end of file            */
{
	0x000003EC, 0x00000003,	0x00000000, 0x00000012,
	0x0000000E, 0x00000002,	0x00000003, 0x00000000,
	0x0000007E, 0x0000007A,	0x00000076, 0x00000012,
	0x00000000, 0x000005DC,	0x000005D8, 0x000005D4,
	0x000005D0, 0x000005CC,	0x000005C8, 0x000005C4,
	0x000005C0, 0x000005BC,	0x000005B8, 0x000005B4,
	0x000005B0, 0x000005AC,	0x00000574, 0x00000352,
	0x0000033A, 0x0000019C,	0x00000132, 0x00000001,
	0x00000000, 0x00000006,	0x00000001, 0x00000000,
	0x00000016, 0x00000001, 0x00000000, 0x00000060,
	0x00000000, 0x000003F2, 0x000003E9, 0x00000000,
	0x000003F2
};

struct ExecBase *ExecBase = NULL;
struct Node *node;




void main(int argc,char *argv[])
{
	int c,opt;
	int x    = 0;
	int xcnt = 0;
	int size = 0;

	printf("\n\tTLPatch    - translator.library patch utility V1.0\n");
	printf("\t               For WB 1.3, Rel. 33.2 library\n");
	printf("\t               Richard Sheppard - 1991\n\n");

	printf("\n\tTLPatch204 - translator.library patch utility V2.04\n");
	printf("\t               For WB 2.04, Rel. 37.1 library\n");
	printf("\t               Kenneth Jennings - 1994\n\n");

	while (--argc > 0) 
	{
		if ( (*++argv)[0] == '-')	/* parse command line arguments */
		{
			while (opt = *++argv[0])
			{
				switch (opt)
				{
					case 'x':
					case 'X':
						Extract();
						exit(0);
						break;

					default:
						break;
				}
			}
		}
	}

	if ( (fp1 = fopen("ram:except.tbl","r")) == NULL)
	{
		printf("*** Can't open ram:except.tbl\n");
		printf("--- Copy your except.tbl to ram: or\n");
		printf("--- use \"%s -x\" to extract table.\n",*argv);
		exit(0);
	}
	else if ( (fp2 = fopen("ram:xtable.new","w")) == NULL)
	{
		printf("*** Can't open ram:xtable.new\n");
		fclose(fp1);
		exit(0);
	}
	else if ( (fp3 = fopen("sys:libs/translator.library","rb")) == NULL)
	{
		printf("*** Can't open sys:libs/translator.library\n");
		fclose(fp2);
		fclose(fp1);
		exit(0);
	}
#ifdef DEBUG_ON
	else if ( (fp4 = fopen("ram:tl.debug","w")) == NULL)
	{
		printf("*** Can't open ram:tl.debug\n");
		fclose(fp3);
		fclose(fp2);
		fclose(fp1);
		exit(0);
	}
#endif

	if ( !(ExecBase = (struct ExecBase *)OpenLibrary("exec.library", 0L)) )
		printf("*** Can't open ExecBase\n");
	else
	{
		if ( (node = (struct Node *)FindName( &ExecBase->LibList,
											  "translator.library") ) != 0)
		{
			printf("\tRemoving resident translator.library\n");
			Remove(node);
		}
	}

	printf("\tReading exception table...\n");
#ifdef DEBUG_ON
	fprintf(fp4, "Reading exception table...\n");
#endif

	while ( (c = GetLine(line, 256) ) > 0)
	{
		size = size + c;
#ifdef DEBUG_ON
		fprintf(fp4, "Read %d chars, size is now (%d, 0x%0X) bytes: %s\n", 
				c, size, size, line);
#endif
		if (StrIndex(line, pattern) >= 0)
		{
			jmptab[x] = (LONG)(size - c + 0x70);/* table size = 0x70 (112) Bytes */
#ifdef DEBUG_ON
			fprintf(fp4, "Calc jmptab[%d] = (%d, 0x%0X)\n", x, jmptab[x], jmptab[x]);
#endif
			x++;
			pattern[1]++;						/* increment search character */

			if(pattern[1] == 91)
				pattern[1] = 48;				/* "0"  - start of numbers */

			if(pattern[1] == 49)
				pattern[1] = 32;				/* " "  - start of punctuation */

			if(pattern[1] == 33)
				pattern[0] = 0;					/* NULL */
		}

		fprintf(fp2,"%s",line);					/* write new xtable */
		xcnt++;									/* exception count */
	}

	fputc('\0',fp2);							/* NULL at end of exception table */

	size++;										/* size + NULL */
#ifdef DEBUG_ON
	fprintf(fp4, "Wrote NULL, size is now (%d, 0x%0X) bytes, DeltaLONG %d\n", 
			size, size, size%4 );
#endif

	if (size % 4)
	{
		for (x = (size % 4); x < 4; x++)
		{
			fputc('\0', fp2);					/* align table to LONG boundary */
			size++;
#ifdef DEBUG_ON
			fprintf(fp4, "Filling NULL, size is now (%d, 0x%0X) bytes, DeltaLONG %d\n", 
						size, size, size%4);
#endif
		}
	}

	codesize = (size + 0x77C) / 4;				/* 0x77C (1916) = code + jmptab - hunks */
#ifdef DEBUG_ON
	fprintf(fp4, "Calc codesize = (%d, 0x%0X) is (size (%d, 0x%0X) + (1916, 0x77C) ) / 4\n", 
						codesize, codesize, size, size);
#endif

	fclose(fp2);
	fclose(fp1);

	printf("\tReading original code...\n");
#ifdef DEBUG_ON
	fprintf(fp4, "Reading original code...\n");
#endif

	Validate();

	fclose(fp3);

	if (codesize <= 0xFFFF)						/* Hunks in 64K blocks */
	{
		code[10] = 0x0000;						/* patch hunk size */
		code[11] = (WORD)codesize;
	}
	else
	{
		code[10] = (WORD)codesize / 0xFFFF;		/* patch hunk size */
		code[11] = (WORD)codesize % 0xFFFF;
	}
	code[16] = code[10];						/* 2nd copy of hunk size */
	code[17] = code[11];
#ifdef DEBUG_ON
	fprintf(fp4, "Calc Hunks [10/16] = 0x%0X, [11/17] = 0x%0X\n", 
				code[10], code[11]);
#endif

	if ( (fp1 = fopen("ram:translator.library","wb")) == NULL)
	{
		printf("*** Can't create ram:translator.library\n");
		exit(0);
	}

#ifdef DEBUG_ON
	fprintf(fp4, "Writing code and jumptable...\n");
#endif
	fwrite((char *)&code,2,0x398,fp1);	/* write code: 0x1CC L, 0x398 W, 0x730 (1840)B */
	fwrite((char *)&jmptab,2,0x38,fp1);	/* write jmptbl: 0x1C L, 0x38 W, 0x70 (112)B   */

#ifdef DEBUG_ON
	fprintf(fp4, "Appending new exceptions table...\n");
#endif
	AddFile();							/* append new xtable ?? bytes                  */

#ifdef DEBUG_ON
	fprintf(fp4, "Writing reloc code...\n");
#endif
	fwrite((char *)&reloc,2,0x5A,fp1);	/* write reloc: 0x2D L, 0x5A W, 0xB4 (180)B    */

	fclose(fp1);

	remove("ram:xtable.new");

	printf("\n\tThe new translator.library is now in ram:\n");
	printf("\t\t\t%d exceptions\n\n",xcnt);

	if(ExecBase)
		CloseLibrary((struct Library *)ExecBase);

#ifdef DEBUG_ON
	fprintf(fp4, "Finished\n");
	fclose(fp4);
#endif

	exit(0);
}



int GetLine(char line[], int max)
{
	int i = 0;
	int c;

	while( (--max > 0) && ( (c = fgetc(fp1)) != EOF) && (c != '\n') )
		line[i++] = c;

	line[i] = '\0';

	return i;
}


int GetExcept(char line[], int max)
{
	int i = 0;
	int c;

	while ( (--max > 0)              && 
		    ( (c=fgetc(fp3)) != EOF) && 
			(c != '\\')              && 
			(c != '`') )
		line[i++] = c;

	if (c == '\\' || c == '`')
		line[i++] = c;

	line[i] = '\0';

	return i;
}


int StrIndex(char source[], char searchfor[])
{
	int i;

	for (i = 0; source[i] != '\0'; i++)
	{
		if ( (source[i]   == searchfor[0])  && 
			 (source[i+1] == searchfor[1]) )
			return i;
	}

	return -1;
}


void AddFile()
{
	int c;
#ifdef DEBUG_ON
	int cnt = 0;
#endif

	if ( (fp2 = fopen("ram:xtable.new","r")) == NULL)
	{
		printf("*** Can't reead ram:xtable.new\n");
#ifdef DEBUG_ON
		fprintf(fp4, "*** Can't read ram:xtable.new\n");
		fclose(fp4);
#endif
		fclose(fp1);
		exit(0);
	}

	while ( (c = getc(fp2)) != EOF)
	{
#ifdef DEBUG_ON
		cnt++;
#endif
		putc(c,fp1);
	}

	fclose(fp2);

#ifdef DEBUG_ON
	fprintf(fp4, "Wrote (%d, 0x%0X) chars, DeltaWORD %d, DeltaLONG %d\n", 
				cnt, cnt, cnt%2, cnt%4);
#endif

	return;
}


void Validate()
{
	int x;

	fread((char *)&code,2,0x398,fp3); /* read 0x398 words (thru byte 0x72F) */

	if ( (code[0x2e] != 0x3337) || 	/* Check words at byte location 0x5c, 0x5e */
		 (code[0x2f] != 0x2e31) )
	{
		printf("*** WARNING - not V37.1, unpredictable results could occur!\n");

		for(x = 0; x < 2; x++)
			printf("%04x ", code[0x5c + x]);

		printf("\n");
	}
	return;
}


void Extract()
{
	int c;

	printf("\tExtracting exception table ...\n");

	if ( (fp1 = fopen("ram:except.tbl","w")) == NULL)
	{
		printf("*** Can't create ram:except.tbl\n");
		exit(0);
	}
	else if ( (fp3 = fopen("sys:libs/translator.library","rb")) == NULL)
	{
		printf("*** Can't open sys:libs/translator.library\n");
		fclose(fp1);

		exit(0);
	}

	Validate();

	fread((char *)&code, 2, 0x38, fp3);	/* read jump table Bytes 0x730 thru 0x7af */

	while ( (c = GetExcept(line, 256)) > 0)
	{
		fprintf(fp1,"%s",line);

		if(line[1] != '\\' && line[1] != '`')
			fprintf(fp1,"\n");
	}

	fclose(fp3);
	fclose(fp1);

	printf("\n\tThe exception table is now in ram:except.tbl\n\n");

	return;
}

/* end of TLPatch204.c */
