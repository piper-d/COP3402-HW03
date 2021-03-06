//Homework 3 | parser.c | 12/2/21 | COP3402

//Group
//Dylan Piper
//Jake Goldberg


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 1000
#define MAX_SYMBOL_COUNT 100

instruction* code;
int cIndex = 0;
symbol* table;
int tIndex = 0;
int level = 0;
int tokenIndex = 0;

//Token data structure
typedef struct {
	char type[3]; // Token type (1-33)
	char value[12]; //Token value (identifier or number)
} Token;

//current token
Token* token;

void emit(int opname, int level, int mvalue);
void addToSymbolTable(int k, char n[], int v, int l, int a, int m);
void program(lexeme* list);
void block(lexeme* list);
void constant(lexeme* list);
int variable(lexeme* list);
void procedure(lexeme* list);
void statement(lexeme* list);
void expression(lexeme* list);
void condition(lexeme* list);
void term(lexeme* list);
void factor(lexeme* list);
void mark();
int mdc(lexeme* list);
int findSymbol(lexeme token, int kind);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();



instruction* parse(lexeme* list, int printTable, int printCode)
{
	code = NULL;

	//Allocates memory for table and code arrays
	table = calloc(MAX_SYMBOL_COUNT, sizeof(symbol));
	code = calloc(MAX_CODE_LENGTH, sizeof(instruction));

	//calls the initial program funcion which recursivly runs the program
	program(list);

	//Prints the decired output based on input parameters
	if (printTable == 1)
		printsymboltable();
	if (printCode == 1)
		printassemblycode();

	//Shows the VM the end of the code array
	code[cIndex].opcode = -1;

	//Frees table if it hasn't already been freed
	if (table != NULL)
		free(table);

	//Returns the code array for the vm to use
	return code;
}

void program(lexeme* list) {
	//JMP
	emit(7, 0, 0);

	//Adds main symbol to the symbol table
	addToSymbolTable(3, "main", 0, level, 0, 0);

	level = -1;

	block(list);

	//Checks to see if the last token is a period
	if (list[tokenIndex].type != periodsym) {
		printparseerror(1);
	}

	//HALT
	emit(9, 0, 3);

	for (int i = 0; i < cIndex; i++) {
		if (code[cIndex].opcode == 5) {
			code[cIndex].m = table[code[cIndex].m].addr;
		}
	}

	code[0].m = table[0].addr;
}

void block(lexeme* list) {
	level++;

	int procedure_idx = tIndex - 1;

	//Checks if constant
	constant(list);

	//Checks if variable
	int x = variable(list);

	//Checks if procedure
	procedure(list);

	table[procedure_idx].addr = cIndex * 3;

	if (level == 0) {
		//INC
		emit(6, 0, x);
	}
	else {
		//INC
		emit(6, 0, x + 3);
	}

	//Checks if statement
	statement(list);

	mark();

	level--;
}

//This function iterates through the token list to check grammar rules and to declare and save the constants in the program
void constant(lexeme* list) {
	if (list[tokenIndex].type == constsym) {
		do {
			tokenIndex++;
			if (list[tokenIndex].type != identsym) {
				printparseerror(2);
			}

			int symidx = mdc(list);
			if (symidx != -1) {
				printparseerror(19);
			}

			char identName[12];
			strcpy(identName, list[tokenIndex].name);

			tokenIndex++;

			if (list[tokenIndex].type != assignsym) {
				printparseerror(2);
			}

			tokenIndex++;

			if (list[tokenIndex].type != numbersym) {
				printparseerror(2);
			}

			addToSymbolTable(1, identName, list[tokenIndex].value, level, 0, 0);

			tokenIndex++;
		} while (list[tokenIndex].type == commasym);

		if (list[tokenIndex].type != semicolonsym) {
			if (list[tokenIndex++].type == identsym) {
				printparseerror(13);
			}
			else {
				printparseerror(14);
			}
		}
		tokenIndex++;

	}
}

//This function iterates through the token list to check grammar rules and to declare and save any variables
int variable(lexeme* list) {
	int numVars = 0;
	if (list[tokenIndex].type == varsym) {

		do {
			numVars++;
			tokenIndex++;
			if (list[tokenIndex].type != identsym) {
				printparseerror(3);
			}
			int symidx = mdc(list);
			if (symidx != -1) {
				printparseerror(18);
			}
			if (level == 0) {
				addToSymbolTable(2, list[tokenIndex].name, 0, level, numVars - 1, 0);
			}
			else {
				addToSymbolTable(2, list[tokenIndex].name, 0, level, numVars + 2, 0);
			}

			tokenIndex++;

		} while (list[tokenIndex].type == commasym);

		if (list[tokenIndex].type != semicolonsym) {
			if (list[tokenIndex].type == identsym) {
				printparseerror(13);
			}
			else {
				printparseerror(14);
			}
		}
		tokenIndex++;

	}
	return numVars;
}

//This function iterates through the token list to check grammar rules and to declare and save any procedures
void procedure(lexeme* list) {

	while (list[tokenIndex].type == procsym) {
		tokenIndex++;
		if (list[tokenIndex].type != identsym) {
			printparseerror(4);
		}
		int symidx = mdc(list);
		if (symidx != -1) {
			printparseerror(19);
		}
		addToSymbolTable(3, list[tokenIndex].name, 0, level, 0, 0);

		tokenIndex++;

		if (list[tokenIndex].type != semicolonsym) {
			printparseerror(4);
		}

		tokenIndex++;

		block(list);
		if (list[tokenIndex].type != semicolonsym) {
			printparseerror(14);
		}

		tokenIndex++;

		emit(2, 0, 0);
	}
}

//This function iterates through the token list to check grammar rules and to declare and save any statements
void statement(lexeme* list) {
	if (list[tokenIndex].type == identsym) {
		int symidx = findSymbol(list[tokenIndex], 2);
		if (symidx == -1) {
			if (findSymbol(list[tokenIndex], 1) != findSymbol(list[tokenIndex], 3)) {
				printparseerror(6);
			}
			else {
				printparseerror(19);
			}
		}

		tokenIndex++;

		if (list[tokenIndex].type != assignsym) {
			printparseerror(5);
		}

		tokenIndex++;

		expression(list);

		emit(4, level - table[symidx].level, table[symidx].addr);
		return;
	}
	if (list[tokenIndex].type == beginsym) {
		do {
			tokenIndex++;
			statement(list);
		} while (list[tokenIndex].type == semicolonsym);
		if (list[tokenIndex].type != endsym) {
			if (list[tokenIndex].type == identsym || list[tokenIndex].type == beginsym || list[tokenIndex].type == ifsym || list[tokenIndex].type == whilesym || list[tokenIndex].type == readsym || list[tokenIndex].type == writesym || list[tokenIndex].type == callsym) {
				printparseerror(15);
			}
			else {
				printparseerror(16);
			}
		}
		tokenIndex++;
		return;
	}
	if (list[tokenIndex].type == ifsym) {
		tokenIndex++;
		condition(list);

		int jpcidx = cIndex;
		emit(8, 0, 0);

		if (list[tokenIndex].type != thensym) {
			printparseerror(8);
		}

		tokenIndex++;

		statement(list);

		if (list[tokenIndex].type == elsesym) {
			int jmpidx = cIndex;
			emit(7, 0, 0);
			code[jpcidx].m = cIndex * 3;
			tokenIndex++;
			statement(list);
			code[jmpidx].m = cIndex * 3;
		}
		else {
			code[jpcidx].m = cIndex * 3;
		}
		return;
	}
	if (list[tokenIndex].type == whilesym) {
		tokenIndex++;
		int loopidx = cIndex;
		condition(list);

		if (list[tokenIndex].type != dosym) {
			printparseerror(9);
		}

		tokenIndex++;

		int jpcidx = cIndex;
		emit(8, 0, 0);

		statement(list);
		emit(7, 0, loopidx * 3);

		code[jpcidx].m = cIndex * 3;
		return;
	}
	if (list[tokenIndex].type == readsym) {
		tokenIndex++;
		if (list[tokenIndex].type != identsym) {
			printparseerror(6);
		}
		int symIdx = findSymbol(list[tokenIndex], 2);
		if (symIdx == -1) {
			if (findSymbol(list[tokenIndex], 1) != findSymbol(list[tokenIndex], 3)) {
				printparseerror(18);
			}
			else {
				printparseerror(19);
			}
		}
		tokenIndex++;
		emit(9, 0, 2);

		emit(4, level - table[symIdx].level, table[symIdx].addr);

		return;
	}
	if (list[tokenIndex].type == writesym) {
		tokenIndex++;
		expression(list);
		emit(9, 0, 1);
		return;
	}
	if (list[tokenIndex].type == callsym) {
		tokenIndex++;
		int symIdx = findSymbol(list[tokenIndex], 3);
		if (symIdx == -1) {
			if (findSymbol(list[tokenIndex], 1) != findSymbol(list[tokenIndex], 2)) {
				printparseerror(7);
			}
			else {
				printparseerror(19);
			}
		}

		tokenIndex++;

		emit(5, level - table[symIdx].level, table[symIdx].addr);
	}
}

//Checks grammar rules and saves conditions
void condition(lexeme* list) {
	if (list[tokenIndex].type == oddsym) {
		tokenIndex++;
		expression(list);
		emit(2, 0, 6);
	}
	else {
		expression(list);
		if (list[tokenIndex].type == eqlsym) {
			tokenIndex++;
			expression(list);
			emit(2, 0, 8);
		}
		else if (list[tokenIndex].type == neqsym) {
			tokenIndex++;
			expression(list);
			emit(2, 0, 9);
		}
		else if (list[tokenIndex].type == lsssym) {
			tokenIndex++;
			expression(list);
			emit(2, 0, 10);
		}
		else if (list[tokenIndex].type == leqsym) {
			tokenIndex++;
			expression(list);
			emit(2, 0, 11);
		}
		else if (list[tokenIndex].type == gtrsym) {
			tokenIndex++;
			expression(list);
			emit(2, 0, 12);
		}
		else if (list[tokenIndex].type == geqsym) {
			tokenIndex++;
			expression(list);
			emit(2, 0, 13);
		}
		else {
			printparseerror(10);
		}
	}
}

//Checks grammar rules and saves expressions
void expression(lexeme* list) {
	if (list[tokenIndex].type == subsym) {
		tokenIndex++;
		term(list);
		emit(2, 0, 1);
		while (list[tokenIndex].type == addsym || list[tokenIndex].type == subsym) {
			if (list[tokenIndex].type == addsym) {
				tokenIndex++;
				term(list);
				emit(2, 0, 2);
			}
			else {
				tokenIndex++;
				term(list);
				emit(2, 0, 3);
			}
		}
	}
	else {
		if (list[tokenIndex].type == addsym) {
			tokenIndex++;
		}
		term(list);
		while (list[tokenIndex].type == addsym || list[tokenIndex].type == subsym) {
			if (list[tokenIndex].type == addsym) {
				tokenIndex++;
				term(list);
				emit(2, 0, 2);
			}
			else {
				tokenIndex++;
				term(list);
				emit(2, 0, 3);
			}
		}
	}
	if (list[tokenIndex].type == lparensym || list[tokenIndex].type == identsym || list[tokenIndex].type == numbersym || list[tokenIndex].type == oddsym) {
		printparseerror(17);
	}
}

void term(lexeme* list) {
	factor(list);
	while (list[tokenIndex].type == multsym || list[tokenIndex].type == divsym || list[tokenIndex].type == modsym) {
		if (list[tokenIndex].type == multsym) {
			tokenIndex++;
			factor(list);
			emit(2, 0, 4);
		}
		else if (list[tokenIndex].type == divsym) {
			tokenIndex++;
			factor(list);
			emit(2, 0, 5);
		}
		else {
			tokenIndex++;
			factor(list);
			emit(2, 0, 7);
		}
	}
}

void factor(lexeme* list) {
	if (list[tokenIndex].type == identsym) {
		int symIdx_var = findSymbol(list[tokenIndex], 2);
		int symIdx_const = findSymbol(list[tokenIndex], 1);

		if (symIdx_var == -1 && symIdx_const == -1) {
			if (findSymbol(list[tokenIndex], 3) != -1) {
				printparseerror(11);
			}
			else {
				printparseerror(19);
			}
		}

		if (symIdx_var == -1) {
			emit(1, 0, table[symIdx_const].val);
		}
		else if (symIdx_const == -1 || table[symIdx_var].level > table[symIdx_const].level) {
			emit(3, level - table[symIdx_var].level, table[symIdx_var].addr);
		}
		else {
			emit(1, 0, table[symIdx_const].val);
		}

		tokenIndex++;
	}
	else if (list[tokenIndex].type == numbersym) {
		emit(1, 0, list[tokenIndex].value);
		tokenIndex++;
	}
	else if (list[tokenIndex].type == lparensym) {
		tokenIndex++;
		expression(list);
		if (list[tokenIndex].type != rparensym) {
			printparseerror(12);
		}
		tokenIndex++;
	}
	else {
		printparseerror(17);
	}
}

//This function iterates thorough the token list in reverse and marks any token that satisfies the conditions
void mark()
{
	int i = tIndex - 1;

	for (i; i >= 0; i--)
	{
		if (table[i].mark == 0 && table[i].level == level)
		{
			table[i].mark = 1;
			continue;
		}
		else if (table[i].mark == 0 && table[i].level < level)
			break;
	}
}

//This function linearly iterates through the token list to find the desired token
//If there is more than one instance, it returns the index of the token with the highest level
int findSymbol(lexeme token, int kind) {
	int maxIndex = -1;
	for (int i = 0; i <= tokenIndex; i++) {
		if (strcmp(token.name, table[i].name) == 0) {
			if (kind == table[i].kind && table[i].mark == 0) {
				maxIndex = i;
			}
		}
	}
	return maxIndex;
}

//This function is a multiple delcaration check
//When called, the function linearly iterates through the token list to see if the token is declared more than once
int mdc(lexeme* list) {
	for (int i = 0; i <= tIndex; i++) {
		if (strcmp(table[i].name, list[tokenIndex].name) == 0) {
			if (table[i].mark == 0) {
				if (table[i].level == level) {
					return i;
				}
			}
		}
	}
	return -1;
}

void emit(int opname, int level, int mvalue)
{
	code[cIndex].opcode = opname;
	code[cIndex].l = level;
	code[cIndex].m = mvalue;

	cIndex++;
}

void addToSymbolTable(int k, char n[], int v, int l, int a, int m)
{
	table[tIndex].kind = k;
	strcpy(table[tIndex].name, n);
	table[tIndex].val = v;
	table[tIndex].level = l;
	table[tIndex].addr = a;
	table[tIndex].mark = m;

	tIndex++;
}


void printparseerror(int err_code)
{
	switch (err_code)
	{
	case 1:
		printf("Parser Error: Program must be closed by a period\n");
		break;
	case 2:
		printf("Parser Error: Constant declarations should follow the pattern 'ident := number {, ident := number}'\n");
		break;
	case 3:
		printf("Parser Error: Variable declarations should follow the pattern 'ident {, ident}'\n");
		break;
	case 4:
		printf("Parser Error: Procedure declarations should follow the pattern 'ident ;'\n");
		break;
	case 5:
		printf("Parser Error: Variables must be assigned using :=\n");
		break;
	case 6:
		printf("Parser Error: Only variables may be assigned to or read\n");
		break;
	case 7:
		printf("Parser Error: call must be followed by a procedure identifier\n");
		break;
	case 8:
		printf("Parser Error: if must be followed by then\n");
		break;
	case 9:
		printf("Parser Error: while must be followed by do\n");
		break;
	case 10:
		printf("Parser Error: Relational operator missing from condition\n");
		break;
	case 11:
		printf("Parser Error: Arithmetic expressions may only contain arithmetic operators, numbers, parentheses, constants, and variables\n");
		break;
	case 12:
		printf("Parser Error: ( must be followed by )\n");
		break;
	case 13:
		printf("Parser Error: Multiple symbols in variable and constant declarations must be separated by commas\n");
		break;
	case 14:
		printf("Parser Error: Symbol declarations should close with a semicolon\n");
		break;
	case 15:
		printf("Parser Error: Statements within begin-end must be separated by a semicolon\n");
		break;
	case 16:
		printf("Parser Error: begin must be followed by end\n");
		break;
	case 17:
		printf("Parser Error: Bad arithmetic\n");
		break;
	case 18:
		printf("Parser Error: Confliciting symbol declarations\n");
		break;
	case 19:
		printf("Parser Error: Undeclared identifier\n");
		break;
	default:
		printf("Implementation Error: unrecognized error code\n");
		break;
	}

	free(code);
	free(table);
	exit(0);
}

void printsymboltable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < tIndex; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr, table[i].mark);

	free(table);
	table = NULL;


}

void printassemblycode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < cIndex; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
		case 1:
			printf("LIT\t");
			break;
		case 2:
			switch (code[i].m)
			{
			case 0:
				printf("RTN\t");
				break;
			case 1:
				printf("NEG\t");
				break;
			case 2:
				printf("ADD\t");
				break;
			case 3:
				printf("SUB\t");
				break;
			case 4:
				printf("MUL\t");
				break;
			case 5:
				printf("DIV\t");
				break;
			case 6:
				printf("ODD\t");
				break;
			case 7:
				printf("MOD\t");
				break;
			case 8:
				printf("EQL\t");
				break;
			case 9:
				printf("NEQ\t");
				break;
			case 10:
				printf("LSS\t");
				break;
			case 11:
				printf("LEQ\t");
				break;
			case 12:
				printf("GTR\t");
				break;
			case 13:
				printf("GEQ\t");
				break;
			default:
				printf("err\t");
				break;
			}
			break;
		case 3:
			printf("LOD\t");
			break;
		case 4:
			printf("STO\t");
			break;
		case 5:
			printf("CAL\t");
			break;
		case 6:
			printf("INC\t");
			break;
		case 7:
			printf("JMP\t");
			break;
		case 8:
			printf("JPC\t");
			break;
		case 9:
			switch (code[i].m)
			{
			case 1:
				printf("WRT\t");
				break;
			case 2:
				printf("RED\t");
				break;
			case 3:
				printf("HAL\t");
				break;
			default:
				printf("err\t");
				break;
			}
			break;
		default:
			printf("err\t");
			break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	if (table != NULL)
		free(table);

}