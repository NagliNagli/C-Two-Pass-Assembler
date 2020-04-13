/*
*********************************
Assembler Project - Mmn 14 2020A
FILENAME: firstTransition.c
FILE INFORMATION : This file parses a specific assembly language which i have assembled based on the project instructions.
It saves the data from an assembly file in data structures, and finds the errors which may occur. 
BY: Gal Nagli
DATE: MARCH 06 2020
*********************************
*/
/* ******Includes ****** */
#include "assembler.h"
#include <ctype.h>
#include <stdlib.h>

/* ****** Directives List ****** */
void parseDataDirc(lineInfo *line, int *IC, int *DC);
void parseStringDirc(lineInfo *line, int *IC, int *DC);
void parseExternDirc(lineInfo *line);
void parseEntryDirc(lineInfo *line);

const directive g_dircArr[] = 
{	/* Name | Parsing Function */
	{ "data", parseDataDirc } ,
	{ "string", parseStringDirc } ,
	{ "extern", parseExternDirc },
	{ "entry", parseEntryDirc },
	{ NULL } /* This value will represent the end of the array. */
};	

/* ******Commands List ****** */
const command g_cmdArr[] =	
{	/* Name | Opcode | NumOfParams */
	{ "mov", 0, 2 } , 
	{ "cmp", 1, 2 } ,
	{ "add", 2, 2 } ,
	{ "sub", 3, 2 } ,
	{ "lea", 4, 2 } ,
	{ "clr", 5, 1 } ,
	{ "not", 6, 1 } ,
	{ "inc", 7, 1 } ,
	{ "dec", 8, 1 } ,
	{ "jmp", 9, 1 } ,
	{ "bne", 10, 1 } ,
	{ "red", 11, 1 } ,
	{ "prn", 12, 1 } ,
	{ "jsr", 13, 1 } ,
	{ "rts", 14, 0 } ,
	{ "stop", 15, 0 } ,
	{ NULL } /* This value will represent the end of the array. */
}; 

/* ****** Externs decleration ****** */
extern labelInfo g_labelArr[MAX_LABELS_NUM];
extern int g_labelNum;
lineInfo *g_entryLines[MAX_LABELS_NUM];
extern int g_entryLabelsNum;
extern int g_dataArr[MAX_DATA_NUM];

/* ******Methods ****** */

/* addLabelToArr function adds the given label to the labelArr and increases labelNum. Returns a pointer to the label in the array. */
labelInfo *addLabelToArr(labelInfo label, lineInfo *line)
{
	/* Check if label is legal */
	if (!isLegalLabel(line->lineStr, line->lineNum, TRUE))
	{
		/* Illegal label name */
		line->isError = TRUE;
		return NULL;
	}

	/* Check if label already exists */
	if (isExistingLabel(line->lineStr))
	{
		printError(line->lineNum, "Label already exists.");
		line->isError = TRUE;
		return NULL;
	}

	/* Add the name to the label */
	strcpy(label.name, line->lineStr);

	/* Add the label to g_labelArr and to the lineInfo */
	if (g_labelNum < MAX_LABELS_NUM)
	{
		g_labelArr[g_labelNum] = label;
		return &g_labelArr[g_labelNum++];
	}

	/* Too many labels */
	printError(line->lineNum, "Too many labels - max is %d.", MAX_LABELS_NUM, TRUE);
	line->isError = TRUE;
	return NULL;
}

/* addNumberToData function adds the given number to the g_dataArr and increases DC. Returns if it succeeded. */
bool addNumberToData(int num, int *IC, int *DC, int lineNum)
{
	/* Check if there is enough space in g_dataArr for the data */
	if (*DC + *IC < MAX_DATA_NUM)
	{
		g_dataArr[(*DC)++] = num;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/* addStringToData function adds the given str to the g_dataArr and increases DC. Returns if it succeeded. */
bool addStringToData(char *str, int *IC, int *DC, int lineNum)
{
	do
	{
		if (!addNumberToData((int)*str, IC, DC, lineNum))
		{
			return FALSE;
		}
	} while (*str++);

	return TRUE;
}

/* labelEnd function finds the label in line->lineStr and add it to the label list. */
/* Returns a pointer to the next char after the label, or NULL is there isn't a legal label. */
char *findLabel(lineInfo *line, int IC)
{
	char *labelEnd = strchr(line->lineStr, ':');
	labelInfo label = { 0 };
	label.address = FIRST_ADDRESS + IC;

	/* Find the label (or return NULL if there isn't) */
	if (!labelEnd)
	{
		return NULL;
	}
	*labelEnd = '\0';

	/* Check if the ':' came after the first word */
	if (!isOneWord(line->lineStr))
	{
		*labelEnd = ':'; /* Fix the change in line->lineStr */
		return NULL;
	}

	/* Check of the label is legal and add it to the labelList */
	line->label = addLabelToArr(label, line);

	return labelEnd + 1; /* +1 to make it point at the next char after the \0 */
}

/* removeLastLabel function ommits the last label in labelArr by updating g_labelNum. */
/* Used to remove the label from a entry/extern line. */
void removeLastLabel(int lineNum)
{
	g_labelNum--;
	printf("[Warning] At line %d: The assembler ignored the label before the directive.\n", lineNum);
}

/* parseDataDirc function parses a .data directive. */
void parseDataDirc(lineInfo *line, int *IC, int *DC)
{
	char *operandTok = line->lineStr, *endOfOp = line->lineStr;
	int operandValue;
	bool foundComma;

	/* Make the label a data label (is there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = FIRST_ADDRESS + *DC;
	}

	/* Check if there are params */
	if (isWhiteSpaces(line->lineStr))
	{
		/* No parameters */
		printError(line->lineNum, "No parameter.");
		line->isError = TRUE;
		return;
	}

	/* Find all the params and add them to g_dataArr */
	FOREVER
	{
		/* Get next param or break if there isn't */
		if (isWhiteSpaces(line->lineStr))
		{
			break;
		}
		operandTok = getFirstOperand(line->lineStr, &endOfOp, &foundComma);

		/* Add the param to g_dataArr */
		if (isLegalNum(operandTok, MEMORY_WORD_LENGTH - 3, line->lineNum, &operandValue))
		{
			if (!addNumberToData(operandValue, IC, DC, line->lineNum))
			{
				/* Not enough memory */
				line->isError = TRUE;
				return;
			}
		}
		else
		{
			/* Illegal number */
			line->isError = TRUE;
			return;
		}

		/* Change the line to start after the parameter */
		line->lineStr = endOfOp;
	}

	if (foundComma)
	{
		/* Comma after the last param */
		printError(line->lineNum, "Do not write a comma after the last parameter.");
		line->isError = TRUE;
		return;
	}
}

/* parseStringDirc function parses a .string directive. */
void parseStringDirc(lineInfo *line, int *IC, int *DC)
{
	/* Make the label a data label (if there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = FIRST_ADDRESS + *DC;
	}

	trimStr(&line->lineStr);

	if (isLegalStringParam(&line->lineStr, line->lineNum))
	{
		if (!addStringToData(line->lineStr, IC, DC, line->lineNum))
		{
			/* Not enough memory */
			line->isError = TRUE;
			return;
		}
	}
	else
	{
		/* Illegal string */
		line->isError = TRUE;
		return;
	}
}

/* parseExternDirc function parses a .extern directive. */
void parseExternDirc(lineInfo *line)
{
	labelInfo label = { 0 }, *labelPointer;

	/* If there is a label in the line, remove the it from labelArr */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

	trimStr(&line->lineStr);
	labelPointer = addLabelToArr(label, line);

	/* Make the label an extern label */
	if (!line->isError)
	{
		labelPointer->address = 0;
		labelPointer->isExtern = TRUE;
	}
}

/* parseEntryDirc function parses a .entry directive. */
void parseEntryDirc(lineInfo *line)
{
	/* If there is a label in the line, remove the it from labelArr */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

	/* Add the label to the entry labels list */
	trimStr(&line->lineStr);

	if (isLegalLabel(line->lineStr, line->lineNum, TRUE))
	{
		if (isExistingEntryLabel(line->lineStr))
		{
			printError(line->lineNum, "Label already defined as an entry label.");
			line->isError = TRUE;
		}
		else if (g_entryLabelsNum < MAX_LABELS_NUM)
		{
			g_entryLines[g_entryLabelsNum++] = line;
		}
	}
}

/* parseDirective function parses the directive and in a directive line. */
void parseDirective(lineInfo *line, int *IC, int *DC)
{
	int i = 0;
	while (g_dircArr[i].name)
	{
		if (!strcmp(line->commandStr, g_dircArr[i].name))
		{
			/* Call the parse function for this type of directive */
			g_dircArr[i].parseFunc(line, IC, DC);
			return;
		}
		i++;
	}
	
	/* line->commandStr isn't a real directive */
	printError(line->lineNum, "No such directive as \"%s\".", line->commandStr);
	line->isError = TRUE;
}

/* areLegalOpTypes function returns if the operands' types are legal (depending on the command). */
bool areLegalOpTypes(const command *cmd, operandInfo op1, operandInfo op2, int lineNum)
{
	/* --- Check First Operand --- */
	/* "lea" command (opcode is 4) can only get a label as the 1st op */
	if (cmd->opcode == 4 && op1.type != LABEL)
	{
		printError(lineNum, "Source operand for \"%s\" command must be a label.", cmd->name);
		return FALSE;
	}

	/* --- Check Second Operand --- */
	/* 2nd operand can be a number only if the command is "cmp" (opcode is 1) or "prn" (opcode is 12).*/
	if (op2.type == NUMBER && cmd->opcode != 1 && cmd->opcode != 12)
	{
		printError(lineNum, "Destination operand for \"%s\" command can't be a number.", cmd->name);
		return FALSE;
	}

	return TRUE;
}

/* parseOpInfo function updates the type and value of operand. */
void parseOpInfo(operandInfo *operand, int lineNum)
{
	int value = 0;

	if (isWhiteSpaces(operand->str))
	{
		printError(lineNum, "Empty parameter.");
		operand->type = INVALID;
		return;
	}

	/* Check if the type is NUMBER */
	if (*operand->str == '#')
	{
		operand->str++; /* Remove the '#' */

		/* Check if the number is legal */
		if (isspace(*operand->str)) 
		{
			printError(lineNum, "There is a white space after the '#'.");
			operand->type = INVALID;
		}
		else
		{
			operand->type = isLegalNum(operand->str, MEMORY_WORD_LENGTH - 3, lineNum, &value) ? NUMBER : INVALID;
		}
	 }
	/* Check if the type is INDREGISTER */
	else if (isIndirectRegister(operand->str, &value))
		operand->type = INDREGISTER;
	/* Check if the type is REGISTER */
	else if (isRegister(operand->str, &value))
		operand->type = REGISTER;
	/* Check if the type is LABEL */
	else if (isLegalLabel(operand->str, lineNum, FALSE))
		operand->type = LABEL;
	/* The type is INVALID */
	else
	{
		printError(lineNum, "\"%s\" is an invalid parameter.", operand->str);
		operand->type = INVALID;
		value = -1;
	}

	operand->value = value;
	}

/* parseCmdOperand function parses the operands in a command line. */
void parseCmdOperands(lineInfo *line, int *IC, int *DC)
{
	char *startOfNextPart = line->lineStr;
	bool foundComma = FALSE;
	int numOfOpsFound = 0;

	/* Reset the op types */
	line->op1.type = INVALID;
	line->op2.type = INVALID;
	/* Get the parameters */
	FOREVER
	{
		/* If both of the operands are registers, or indirect registers they will only take 1 memory word (instead of 2) */
		if (!(line->op1.type == REGISTER && line->op2.type == REGISTER) && !(line->op1.type == REGISTER && line->op2.type == INDREGISTER) && !(line->op1.type == INDREGISTER && line->op2.type == REGISTER) && !(line->op1.type == INDREGISTER && line->op2.type == INDREGISTER))
		{
			/* Check if there is enough memory */
			if (*IC + *DC < MAX_DATA_NUM)
			{
				++*IC; /* Count the last command word or operand. */
			}
			else
			{
				line->isError = TRUE;
				return;
			}
		}
		
		/* Check if there are still more operands to read */
		if (isWhiteSpaces(line->lineStr) || numOfOpsFound > 2)
		{
			/* If there are more than 2 operands it's already illegal */
			break;
		}

		/* If there are 2 ops, make the destination become the source op */
		if (numOfOpsFound == 1)
		{
			line->op1 = line->op2;
			/* Reset op2 */
			line->op2.type = INVALID;
		}

		/* Parse the opernad*/
		line->op2.str = getFirstOperand(line->lineStr, &startOfNextPart, &foundComma);
		parseOpInfo(&line->op2, line->lineNum);

		if (line->op2.type == INVALID)
		{
			line->isError = TRUE;
			return;
		}

		numOfOpsFound++;
		line->lineStr = startOfNextPart;
	} /* End of while */

	/* Check if there are enough operands */
	if (numOfOpsFound != line->cmd->numOfParams) 
	{
		/* There are more/less operands than needed */
		if (numOfOpsFound <  line->cmd->numOfParams)
		{
			printError(line->lineNum, "Not enough operands.", line->commandStr);
		}
		else
		{
			printError(line->lineNum, "Too many operands.", line->commandStr);
		}

		line->isError = TRUE;
		return;
	}

	/* Check if there is a comma after the last param */
	if (foundComma)
	{
		printError(line->lineNum, "Don't write a comma after the last parameter.");
		line->isError = TRUE;
		return;
	}
	/* Check if the operands' types are legal */
	if (!areLegalOpTypes(line->cmd, line->op1, line->op2, line->lineNum))
	{
		line->isError = TRUE;
		return;
	}
}

/* parseCommand function parses the command in a command line. */
void parseCommand(lineInfo *line, int *IC, int *DC)
{
	int cmdId = getCmdId(line->commandStr);

	if (cmdId == -1)
	{
		line->cmd = NULL;
		if (*line->commandStr == '\0')
		{
			/* The command is empty, but the line isn't empty so it's only a label. */
			printError(line->lineNum, "Can't write a label to an empty line.", line->commandStr);
		}
		else
		{
			/* Illegal command. */
			printError(line->lineNum, "No such command as \"%s\".", line->commandStr);
		}
		line->isError = TRUE;
		return;
	}

	line->cmd = &g_cmdArr[cmdId];
	parseCmdOperands(line, IC, DC);
}

/* allocString function returns the same string in a different part of the memory by using malloc. */
char *allocString(const char *str) 
{
	char *newString = (char *)malloc(strlen(str) + 1);
	if (newString) 
	{
		strcpy(newString, str); 
	}

	return newString;
}

/* parseLine function parses a line, and print it's errors if there are any. */
void parseLine(lineInfo *line, char *lineStr, int lineNum, int *IC, int *DC)
{
	char *startOfNextPart = lineStr;

	line->lineNum = lineNum;
	line->address = FIRST_ADDRESS + *IC;
	line->originalString = allocString(lineStr);
	line->lineStr = line->originalString;
	line->isError = FALSE;
	line->label = NULL;
	line->commandStr = NULL;
	line->cmd = NULL;

	if (!line->originalString)
	{
		printf("[Error] Not enough memory - malloc falied.");
		return;
	}

	/* Check if the line is a comment */
	if (isCommentOrEmpty(line))
	{	
		return;
	}

	/* Find label and add it to the label list */
	startOfNextPart = findLabel(line, *IC);
	if (line->isError)
	{
		return;
	}
	/* Update the line if startOfNextPart isn't NULL */
	if (startOfNextPart)
	{
		line->lineStr = startOfNextPart;
	}

	/* Find the command token */
	line->commandStr = getFirstTok(line->lineStr, &startOfNextPart);
	line->lineStr = startOfNextPart;
	/* Parse the command / directive */
	if (isDirective(line->commandStr))
	{
		line->commandStr++; /* Remove the '.' from the command */
		parseDirective(line, IC, DC);
	}
	else
	{
		parseCommand(line, IC, DC);
	}

	if (line->isError)
	{
		return;
	}
}

/* readLine function puts a line from 'file' in 'buf'. Returns if the line is shorter than maxLength. */
bool readLine(FILE *file, char *buf, size_t maxLength)
{
	char *endOfLine;

	if (!fgets(buf, maxLength, file))
	{
		return FALSE;
	}

	/* Check if the line is too long (no '\n' was present). */
	endOfLine = strchr(buf, '\n');
	if (endOfLine)
	{
		*endOfLine = '\0';
	}
	else
	{
		char c;
		bool ret = (feof(file)) ? TRUE : FALSE; /* Return FALSE, unless it's the end of the file */

		/* Keep reading chars until you reach the end of the line ('\n') or EOF */
		do
		{
			c = fgetc(file);
		} while (c != '\n' && c != EOF);

		return ret;
	}

	return TRUE;
}

/* firstTransitionRead function readds the file for the first time, line by line, and parses it. */
/* Returns how many errors were found. */
int firstTransitionRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC)
{
	char lineStr[MAX_LINE_LENGTH + 2]; /* +2 for the \n and \0 at the end */
	int errorsFound = 0;

	*linesFound = 0;

	/* Read lines and parse them */
	while (!feof(file))
	{
		if (readLine(file, lineStr, MAX_LINE_LENGTH + 2)) 
		{
			/* Check if the file is too lone */
			if (*linesFound >= MAX_LINES_NUM)
			{
				printf("[Error] File is too long. Max lines number in file is %d.\n", MAX_LINES_NUM);
				return ++errorsFound;
			}

			/* Parse a line */
			parseLine(&linesArr[*linesFound], lineStr, *linesFound + 1, IC, DC);

			/* Update errorsFound */
			if (linesArr[*linesFound].isError)
			{
				errorsFound++;
			}

			/* Check if the number of memory words needed is small enough */
			if (*IC + *DC >= MAX_DATA_NUM)
			{
				/* dataArr is full. Stop reading the file. */
				printError(*linesFound + 1, "Too much data and code. Max memory words is %d.", MAX_DATA_NUM);
				printf("[Info] Memory is full. Stoping to read the file.\n");
				return ++errorsFound;
			}
			++*linesFound;
		}
		else if (!feof(file))
		{
			/* Line is too long */
			printError(*linesFound + 1, "Line is too long. Max line length is %d.", MAX_LINE_LENGTH);
			errorsFound++;
			 ++*linesFound;
		}
	}

	return errorsFound;
}
