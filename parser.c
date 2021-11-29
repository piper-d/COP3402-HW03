#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 1000
#define MAX_SYMBOL_COUNT 100

instruction *code;
int cIndex=0;
symbol *table;
int tIndex=0;
int level=0;

//Token data structure
typedef struct {
	char type[3]; // Token type (1-33)
	char value[12]; //Token value (identifier or number)
} Token;

//current token
Token* tokenList[MAX_SYMBOL_COUNT];
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
void term(lexeme* list);
void factor(lexeme* list);
void mark();
int findSymbol(lexeme list, int x);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();

instruction *parse(lexeme *list, int printTable, int printCode)
{
	code = NULL;
	/* this line is EXTREMELY IMPORTANT, you MUST uncomment it
		when you test your code otherwise IT WILL SEGFAULT in 
		vm.o THIS LINE IS HOW THE VM KNOWS WHERE THE CODE ENDS
		WHEN COPYING IT TO THE PAS
	code[cIndex].opcode = -1;
	*/
	return code;
}

void program(lexeme* list) {
	emit(7, 0, 0);
	addToSymbolTable(3, "main", 0, 0, 0, 0);
	level=-1;
	
	block(list);

	if(list[tIndex].type != 32){
		error(1);
	}
	
	emit(9, 0, 3);
	for(int i=0; i < cIndex; i++) {
		if(code[cIndex].opcode == 5) {
			code[cIndex].m = table[code[cIndex].m].addr;
		}
	}
	code[0].m=table[0].addr;
}

void block(lexeme* list) {
	level++;
	int procedure_idx = tIndex-1;
	constant(list);
	int x = variable(list);
	procedure(list);
	table[procedure_idx].addr = cIndex*3;

	if(level==0) {
		emit(6, 0, x);
	} else {
		emit(6, 0, x+3);
	}

	statement(list);
	mark(list);
	level--;
}

void constant(lexeme* list) {
	if(list[tIndex].type == 0) {
		tIndex++;
		if(list[tIndex].type != identsym) {
			error(2);
		}
		int symidx = mdc(list[tIndex]);
		if(symidx != -1) {
			error(13);
		}
		char identName[12];
		strcpy(identName, list[tIndex].name);

		tIndex++;

		if(list[tIndex].type != assignsym) {
			error(2);
		}

		tIndex++;

		if(list[tIndex].type != numbersym) {
			error(2);
		}

		addToSymbolTable(1, identName, list[tIndex].value, level, 0, 0);

		tIndex++;

		while(list[tIndex].type == commasym) {
			if(list[tIndex].type != semicolonsym) {
				if(list[tIndex++].type == identsym) {
					error(2);
				} else {
					error(2);
				}
			}
			tIndex++;
		}
	}
}

int variable(lexeme* list) {
	int numVars = 0;
	if(list[tIndex].type == varsym) {
		do {
			numVars++;
			tIndex++;
			if(list[tIndex].type != identsym) {
				error(20);
			}
			int symidx = mdc(list[tIndex]);
			if(symidx != -1) {
				error(13);
			}
			if(level == 0) {
				addToSymbolTable(2, list[tIndex].name, 0, level, numVars-1, 0);
			} else {
				addToSymbolTable(2, list[tIndex].name, 0, level, numVars+2, 0);
			}

			tIndex++;

		} while(list[tIndex].type == commasym);

		if(list[tIndex].type != semicolonsym) {
			if(list[tIndex].type == identsym) {
				error(14);
			} else {
				error(20);
			}
		}
		tIndex++;
		return numVars;
	}
}

void procedure(lexeme* list) {
	while(list[tIndex].type == procsym) {
		tIndex++;
		if(list[tIndex].type != identsym) {
			error();
		}
		int symidx = mdc(list[tIndex]);
		if(symidx != -1) {
			error();
		}
		addToSymbolTable(3, list[tIndex].name, 0, level, 0, 0);

		tIndex++;

		if(list[tIndex].type != semicolonsym) {
			error();
		}

		tIndex++;

		block(list);
		if(list[tIndex].type != semicolonsym) {
			error();
		}

		tIndex++;

		emit(2, 0, 0);
	}
}

void statement(lexeme* list) {
	if(list[tIndex].type == identsym) {
		int symidx = findSymbol(list[tIndex], 2);
		if (symidx == -1) {
			if(findSymbol(list[tIndex], 1) != findSymbol(list[tIndex], 3)) {
				error();
			} else {
				error();
			}
		}

		tIndex++;

		if(list[tIndex].type != assignsym) {
			error();
		}

		tIndex++;

		expression(list);

		emit(4, level-table[symidx].level, table[symbol].addr);
		return;
	}
	if(list[tIndex].type == beginsym) {
		do {
			tIndex++;
			statement(list);
		} while(list[tIndex].type == semicolonsym);
		if(list[tIndex].type != endsym) {
			if(list[tIndex].type == identsym || list[tIndex].type == beginsym || list[tIndex].type == ifsym || list[tIndex].type == whilesym || list[tIndex].type == readsym || list[tIndex].type == writesym || list[tIndex].type == callsym) {
				error();
			} else {
				error();
			}
		}
		tIndex++;
		return;
	}
	if(list[tIndex].type == ifsym) {
		tIndex++;
		condition(list);

		int jpcidx = cIndex;
		emit(8, 0, 0);

		if(list[tIndex].type != thensym) {
			error();
		}

		tIndex++;

		statement(list);

		if(list[tIndex].type == elsesym) {
			int jmpidx = cIndex;
			emit(7, 0, 0);
			code[jpcidx].m = cIndex*3;
			statement(list);
			code[jmpidx].m = cIndex*3;
		} else {
			code[jpcidx].m = cIndex*3;
		}
		return;
	}
	if(list[tIndex].type == whilesym) {
		tIndex++;
		int loopidx = cIndex;
		condition(list);
		
		if(list[tIndex].type != dosym) {
			error();
		}

		tIndex++;

		int jpcidx = cIndex;
		emit(8, 0, 0);

		statement(list);
		emit(7, 0, loopidx*3);

		code[jpcidx].m = cIndex*3;
		return; 
	}
	if(list[tIndex].type == readsym) {
		tIndex++;
		if(list[tIndex].type != identsym){
			error();
		}
		int symIdx = findSymbol(list[tIndex], 2);
		if(symIdx == -1){
			if(findSymbol(list[tIndex], 1) != findSymbol(list[tIndex],3)) {
				error();
			} else	{
				error();
			}
		}
		tIndex++;
		emit(9,0,2);
		//  check psuedo for this, what is L= level etc
		
		emit(4,level-table[symIdx].level,table[symIdx].addr);
		
		return;
	}
	if(list[tIndex].type == writesym) {
		tIndex++;
		expression(list);
		emit(9,0,1);
		return;
	}
	if(list[tIndex].type == callsym) {
		tIndex++;
		int symIdx = findSymbol(list[tIndex],3);
		if(symIdx == -1) {
			if(findSymbol(list[tIndex],1) != findSymbol(list[tIndex],2)) {
				error();
			} else {
				error();
			}
		}
		
		tIndex++;

		emit(5,level-table[symIdx].level,symIdx);
	}
}

void condition(lexeme* list) {
	if(list[tIndex].type == oddsym) {
		tIndex++;
		expression(list);
		emit(2,0,6);
	} else {
		expression(list);
		if(list[tIndex].type == eqlsym) {
			tIndex++;
			expression(list);
			emit(2,0,8);
		}else if(list[tIndex].type == neqsym) {
			tIndex++;
			expression(list);
			emit(2,0,9);
		}else if(list[tIndex].type == lsssym) {
			tIndex++;
			expression(list);
			emit(2,0,10);
		}else if(list[tIndex].type == leqsym) {
			tIndex++;
			expression(list);
			emit(2,0,11);
		}else if(list[tIndex].type == gtrsym) {
			tIndex++;
			expression(list);
			emit(2,0,12);
		}else if(list[tIndex].type == geqsym) {
			tIndex++;
			expression(list);
			emit(2,0,13);
		}else {
			error();
		}
	}
}

void expression(lexeme* list) {
	if(list[tIndex].type == subsym) {
		tIndex++;
		term(list);
		emit(2,0,1);
		while ( list[tIndex].type == addsym || list[tIndex].type == subsym) {
			if(list[tIndex].type == addsym) {
				tIndex++;
				term(list);
				emit(2,0,2);
			}else {
				tIndex++;
				term(list);
				emit(2,0,3);
			}
		}
	}else {
		if(list[tIndex].type == addsym) {
			tIndex++;
		}
		term(list);
		while(list[tIndex].type == addsym || list[tIndex].type == subsym) {
			if(list[tIndex].type == addsym) {
				tIndex++;
				term(list);
				emit(2,0,2);
			}else {
				tIndex++;
				term(list);
				emit(2,0,3);
			}
		}
	}
	if(list[tIndex].value % 2 != 0) {
		error();
	}
}

void term(lexeme* list) {
	factor(list);
	while(list[tIndex].type == multsym || list[tIndex].type == divsym || list[tIndex].type == modsym) {
		if(list[tIndex].type == multsym) {
			tIndex++;
			factor(list);
			emit(2, 0, 4);
		} else if(list[tIndex].type == divsym) {
			tIndex++;
			factor(list);
			emit(2, 0, 5):
		} else {
			tIndex++;
			factor(list);
			emit(2, 0, 7);
		}
	}
}

void factor(lexeme* list) {
	if(list[tIndex].type == identsym) {
		int symIdx_var = findSymbol(list[tIndex], 2);
		int symIdx_const = findSymbol(list[tIndex], 1);

		if(symIdx_var == -1 && symIdx_const == -1) {
			if(findSymbol(list[tIndex], 3) != -1) {
				error();
			} else {
				error();
			}
		}

		if(symIdx_var == -1) {
			emit(1, 0, table[symIdx_const].val);
		} else if(symIdx_const == -1 || table[symIdx_var].level > table[symIdx_const].level) {
			emit(3, level-table[symIdx_var],level, table[symIdx_var].addr);
		} else {
			emit(1, 0, table[symIdx_const].val);
		}

		tIndex++;
	} else if(list[tIndex].type == numbersym) {
		emit(1, 0, 0);
		tIndex++;
	} else if(list[tIndex].type == lparensym) {
		tIndex++;
		expression(list);
		if(list[tIndex].type != rparensym) {
			error();
		}
		tIndex++;
	} else {
		error();
	}
}

void mark() {

}

int findSymbol(lexeme list, int x) {

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