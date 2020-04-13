/*
*********************************
Assembler Project - Mmn 14 2020A
FILENAME: main.c
FILE INFORMATION : This file will manage the process of the assembler, it will call the first and second read methods.
afterwards, it will create the output files: file.ob,file.ent.file.ext.
BY: Gal Nagli
DATE: MARCH 06 2020
*********************************
*/
/* ======== Includes ======== */
#include "assembler.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/* ====== Global Data Structures ====== */
/* Labels Array */
labelInfo g_labelArr[MAX_LABELS_NUM]; 
int g_labelNum = 0;
/* Entry Lines */
lineInfo *g_entryLines[MAX_LABELS_NUM]; /**/
int g_entryLabelsNum = 0;
/* Data Array */
int g_dataArr[MAX_DATA_NUM];

/* ====== Methods ====== */
/* getNumOctalLength function will return octal number length, which will be useful for the print function */
int getNumOctalLength(int num)
{
	int l = !num;
	while(num)
	{
		l++;
		num /= OCTAL;
	}
	return l;
}

/* ====== Methods ====== */
/* getNumDecimalLength function will return decimal number length, which will be useful for the print function */
int getNumDecimalLength(int num)
{
	int l = !num;
	while(num)
	{
		l++;
		num /= DECIMAL;
	}
	return l;
}
/* convertDecimalToOctal function will convert decimal number to octal */

int convertDecimalToOctal(int decimalNumber)
{
	int octalNumber = 0, i = 1;
	
	while(decimalNumber != 0)
	{
		octalNumber+= (decimalNumber % OCTAL) * i;
		decimalNumber /= OCTAL;
		i *= DECIMAL;
	}
	return octalNumber;
}

/* printError function prints an error with the line number. */
void printError(int lineNum, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[Error] At line %d: ", lineNum);
	vprintf(format, args);
	printf("\n");
	va_end(args);
}

/* fprintfDest function prints the dest value as decimal, with atleast 4 digits. */

void fprintfDest(FILE *file, int num)
{
	/* Add zeros first, to make the length 4 digits. */
	int length;
	length = getNumDecimalLength(num);
	if(length == ONE_DIGIT)
		fprintf(file, "000");
	else if(length == TWO_DIGITS)
		fprintf(file, "00");
	else if(length == THREE_DIGITS)
		fprintf(file,"0");
	fprintf(file, "%d", num);
}

/* fprintfICDC function prints the IC and DC value as decimal */

void fprintfICDC(FILE *file, int num)
{
	fprintf(file, "\t%d", num);
}

/* fprintfEnt function prints the entry value as decimal */

void fprintfEnt(FILE *file, int num)
{
	fprintf(file, "%d", num);
}
/* fprintfData function print's the data as octal representation, with 5 digits. */

void fprintfData(FILE *file, int num)
{
	int length;
	length = getNumOctalLength(num);
	if(length == ONE_DIGIT)
		fprintf(file, "0000");
	else if(length == TWO_DIGITS)
		fprintf(file, "000");
	else if(length == THREE_DIGITS)
		fprintf(file,"00");
	else if(length == FOUR_DIGITS)
		fprintf(file,"0");
	num = convertDecimalToOctal(num);
	fprintf(file, "%d", num);
}

/* fprintfExt function prints the Ext value as decimal */

void fprintfExt(FILE *file, int num)
{
	int length;
	length = getNumDecimalLength(num);
	if(length == THREE_DIGITS)
		fprintf(file,"0");
	fprintf(file, "%d", num);
}
/* openFile function creates a file (for writing) from a given name and ending, and returns a pointer to it. */
FILE *openFile(char *name, char *ending, const char *mode)
{
	FILE *file;
	char *mallocStr = (char *)malloc(strlen(name) + strlen(ending) + 1), *fileName = mallocStr;
	sprintf(fileName, "%s%s", name, ending);

	file = fopen(fileName, mode);
	free(mallocStr);

	return file;
}

/* createObjectFile function creates the .obj file*/
void createObjectFile(char *name, int IC, int DC, int *memoryArr)
{
	int i;
	FILE *file;
	file = openFile(name, ".ob", "w");

	/* Print IC and DC */
	fprintfICDC(file, IC);
	fprintf(file, "\t\t");
	fprintfICDC(file, DC);

	/* Print all of memoryArr */
	for (i = 0; i < IC + DC; i++)
	{
		fprintf(file, "\n");
		fprintfDest(file, FIRST_ADDRESS + i); /* adding the 100 to the IC print */
		fprintf(file, "\t\t");
		fprintfData(file, memoryArr[i]);
	}

	fclose(file);
}

/* createEntriesFile function creates the .ent file, which contains the addresses for the .entry labels */
void createEntriesFile(char *name)
{
	int i;
	FILE *file;

	/* Don't create the entries file if there aren't entry lines */
	if (!g_entryLabelsNum)
	{
		return;
	}

	file = openFile(name, ".ent", "w");

	for (i = 0; i < g_entryLabelsNum; i++)
	{
		fprintf(file, "%s\t\t", g_entryLines[i]->lineStr);
		fprintfEnt(file, getLabel(g_entryLines[i]->lineStr)->address);

		if (i != g_entryLabelsNum - 1)
		{
			fprintf(file, "\n");
		}
	}

	fclose(file);
}

/* createExternFile function creates the .ext file, which contains the addresses for the extern labels operands */
void createExternFile(char *name, lineInfo *linesArr, int linesFound)
{
	int i;
	labelInfo *label;
	bool firstPrint = TRUE; /* This bool meant to prevent the creation of the file if there aren't any externs */
	FILE *file = NULL;

	for (i = 0; i < linesFound; i++)
	{
		/* Check if the 1st operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 2 && linesArr[i].op1.type == LABEL)
		{
			label = getLabel(linesArr[i].op1.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintfExt(file, linesArr[i].op1.address);
				firstPrint = FALSE;
			}
		}

		/* Check if the 2nd operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].op2.type == LABEL)
		{
			label = getLabel(linesArr[i].op2.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintfExt(file, linesArr[i].op2.address);
				firstPrint = FALSE;
			}
		}
	}

	if (file)
	{
		fclose(file);
	}
}

/* clearData function resets all the globals and free all the malloc blocks. */
void clearData(lineInfo *linesArr, int linesFound, int dataCount)
{
	int i;

	/* --- Reset Globals --- */

	/* Reset global labels */
	for (i = 0; i < g_labelNum; i++)
	{
		g_labelArr[i].address = 0;
		g_labelArr[i].isData = 0;
		g_labelArr[i].isExtern = 0;
	}
	g_labelNum = 0;

	/* Reset global entry lines */
	for (i = 0; i < g_entryLabelsNum; i++)
	{
		g_entryLines[i] = NULL;
	}
	g_entryLabelsNum = 0;

	/* Reset global data */
	for (i = 0; i < dataCount; i++)
	{
		g_dataArr[i] = 0;
	}

	/* Free malloc blocks */
	for (i = 0; i < linesFound; i++)
	{
		free(linesArr[i].originalString);
	}
}

/* parseFile function parses a file, and creates the output files. */
void parseFile(char *fileName)
{
 	FILE *file = openFile(fileName, ".as", "r");
	lineInfo linesArr[MAX_LINES_NUM];
	int memoryArr[MAX_DATA_NUM] = { 0 }, IC = 0, DC = 0, numOfErrors = 0, linesFound = 0;

	/* Open File */
	if (file == NULL)
	{
		printf("[Info] Can't open the file \"%s.as\".\n", fileName);
		return;
	}
	printf("[Info] Successfully opened the file \"%s.as\".\n", fileName);

	/* First Read */
	numOfErrors += firstTransitionRead(file, linesArr, &linesFound, &IC, &DC);
	/* Second Read */
	numOfErrors += secondTransitionRead(memoryArr, linesArr, linesFound, IC, DC);

	/* Create Output Files */
	if (numOfErrors == 0)
	{
		/* Create all the output files */
		createObjectFile(fileName, IC, DC, memoryArr);
		createExternFile(fileName, linesArr, linesFound); 
		createEntriesFile(fileName);
		printf("[Info] Created output files for the file \"%s.as\".\n", fileName);
	}
	else
	{
		/* print the number of errors. */
		printf("[Info] A total number of %d error%s found in \"%s.as\".\n", numOfErrors, (numOfErrors > 1) ? "s were" : " was", fileName);
	}

	/* Free all malloc pointers, and reset the globals. */
	clearData(linesArr, linesFound, IC + DC);

	/* Close File */
	fclose(file);
}

/* Main function. Calls the "parsefile" method for each file name in argv. */
int main(int argc, char *argv[])
{
	int i;

	if (argc < 2)
	{
		printf("[Info] no file names were given.\n");
		return 1;
	}

		
	for (i = 1; i < argc; i++)
	{
		parseFile(argv[i]);
		printf("\n");
	}

	return 0;
}
