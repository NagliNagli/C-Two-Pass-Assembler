/*
*********************************
Assembler Project - Mmn 14 2020A
FILENAME: assembler.h
FILE INFORMATION : this file is destined to act as general Header for the assembly project.
it contains the macros, data structures and method decleration needed for the project to work.
BY: Gal Nagli
DATE: MARCH 06 2020
*********************************
*/
#ifndef ASSEMBLER_H
	#define ASSEMBLER_H
#include <stdio.h>
#include <string.h>

/* ******** Macros ******** */
/* Helpers */
#define FOREVER				for(;;)
#define BYTE_SIZE			8
#define FALSE				0
#define TRUE				1

/* Constants */
#define MAX_DATA_NUM		4096
#define FIRST_ADDRESS		100 
#define MAX_LINE_LENGTH		80
#define MAX_LABEL_LENGTH	31
#define MEMORY_WORD_LENGTH	15
#define MAX_REGISTER_DIGIT	7
#define MAX_LINES_NUM		300
#define MAX_LABELS_NUM		MAX_LINES_NUM
#define OCTAL 8 
#define DECIMAL 10
#define ONE_DIGIT 1
#define TWO_DIGITS 2
#define THREE_DIGITS 3
#define FOUR_DIGITS 4

/* ********  Data Structures ******** */
typedef unsigned int bool; /* Only get TRUE or FALSE values */

/* *** First Read Structures  *** */

/* Labels Structure */
typedef struct
{
	int address;					/* The address it contains */
	char name[MAX_LABEL_LENGTH];	/* The name of the label */					
	bool isExtern;					/* Extern flag */
	bool isData;					/* Data flag (.data or .string) */
} labelInfo;

/* Directive And Commands */
typedef struct 
{
	char *name;
	void (*parseFunc)();
} directive;

typedef struct
{
	char *name;
	unsigned int opcode : 4;
	int numOfParams;
} command;

/* Operands */
/*The numbers are assigned as the bits which will be turned on regarding the matching opType.*/
typedef enum { NUMBER = 1, LABEL = 2, INDREGISTER = 4, REGISTER = 8, INVALID = -1 } opType; 

typedef struct
{
	int value;				/* Value */
	char *str;				/* String */
	opType type;			/* Type */
	int address;			/* The address of the operand in the memory */
} operandInfo;

/* Line Structure */
typedef struct
{
	int lineNum;				/* The number of the line in the file */
	int address;				/* The address of the first word in the line */
	char *originalString;		/* The original pointer, allocated by malloc */
	char *lineStr;				/* The text it contains (changed while using parseLine) */
	bool isError;				/* Represent whether there is an error or not */
	labelInfo *label;			/* A poniter to the lines label in labelArr */

	char *commandStr;			/* The string of the command or directive */

	/* Command line */
	const command *cmd;			/* A pointer to the command in g_cmdArr */
	operandInfo op1;			/* The 1st operand */
	operandInfo op2;			/* The 2nd operand */
} lineInfo;

/* *** Second Read Structures  *** */

typedef enum { EXTERNAL = 1, RELOCATABLE = 2, ABSOLUTE = 4 } areType;

/* Memory Word Structure */

typedef struct /* 15 bits */
{
	unsigned int are : 3;

	union /* 12 bits */
	{
		/* Commands (only 12 bits) */
		struct
		{
			unsigned int dest : 4;		/* Destination op addressing method ID */
			unsigned int src : 4;		/* Source op addressing method ID */
			unsigned int opcode : 4;	/* Command ID */
		} cmdBits;

		/* Registers (only 6 bits) */
		struct
		{
			unsigned int destBits : 3; /* DEST Register */
			unsigned int srcBits : 3; /* SRC Register*/
		} regBits;

		/* Other operands */
		int value : 12; /* (12 bits) */

	} valueBits; /* End of 12 bits union */

} memoryWord;


/* ****** Functions Declaration ****** */
/* utility.c methods, Further explanation is given before each function. */
int getCmdId(char *cmdName);
labelInfo *getLabel(char *labelName);
void trimLeftStr(char **ptStr);
void trimStr(char **ptStr);
char *getFirstTok(char *str, char **endOfTok);
bool isOneWord(char *str);
bool isWhiteSpaces(char *str);
bool isLegalLabel(char *label, int lineNum, bool printErrors);
bool isExistingLabel(char *label);
bool isExistingEntryLabel(char *labelName);
bool isRegister(char *str, int *value);
bool isIndirectRegister(char *str, int *value);
bool isCommentOrEmpty(lineInfo *line);
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma);
bool isDirective(char *cmd);
bool isLegalStringParam(char **strParam, int lineNum);
bool isLegalNum(char *numStr, int numOfBits, int lineNum, int *value);

/* firstTransition.c methods */
int firstTransitionRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC);

/* secondTransition.c methods */
int secondTransitionRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC);

/* main.c methods */
void printError(int lineNum, const char *format, ...);
int convertDecimalToOctal(int decimalNumber);

#endif
