/*
	This is the lex.c file for the UCF Fall 2021 Systems Software Project.
	For HW2, you must implement the function lexeme *lexanalyzer(char *input).
	You may add as many constants, global variables, and support functions
	as you desire.

	If you choose to alter the printing functions or delete list or lex_index,
	you MUST make a note of that in you readme file, otherwise you will lose
	5 points.
*/

/*
Team Members:
Jake Goldberg
Dylan Piper
*/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#define MAX_NUMBER_TOKENS 500
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5

lexeme* list;
int lex_index;



void printlexerror(int type);
void printtokens();


void inputcheck(char* input)		//Compares the strings to known tokens to set the token type
{

	if (strcmp(input, "const") == 0)
	{
		list[lex_index].type = constsym;
		lex_index++;
	}
	else if (strcmp(input, "var") == 0)
	{
		list[lex_index].type = varsym;
		lex_index++;
	}
	else if (strcmp(input, "procedure") == 0)
	{
		list[lex_index].type = procsym;
		lex_index++;
	}
	else if (strcmp(input, "call") == 0)
	{
		list[lex_index].type = callsym;
		lex_index++;
	}
	else if (strcmp(input, "if") == 0)
	{
		list[lex_index].type = ifsym;
		lex_index++;
	}
	else if (strcmp(input, "then") == 0)
	{
		list[lex_index].type = thensym;
		lex_index++;
	}
	else if (strcmp(input, "else") == 0)
	{
		list[lex_index].type = elsesym;
		lex_index++;
	}
	else if (strcmp(input, "while") == 0)
	{
		list[lex_index].type = whilesym;
		lex_index++;
	}
	else if (strcmp(input, "do") == 0)
	{
		list[lex_index].type = dosym;
		lex_index++;
	}
	else if (strcmp(input, "begin") == 0)
	{
		list[lex_index].type = beginsym;
		lex_index++;
	}
	else if (strcmp(input, "end") == 0)
	{
		list[lex_index].type = endsym;
		lex_index++;
	}
	else if (strcmp(input, "read") == 0)
	{
		list[lex_index].type = readsym;
		lex_index++;
	}
	else if (strcmp(input, "write") == 0)
	{
		list[lex_index].type = writesym;
		lex_index++;
	}
	else if (strcmp(input, "odd") == 0)
	{
		list[lex_index].type = oddsym;
		lex_index++;
	}
	else if (strcmp(input, "!=") == 0)
	{
		list[lex_index].type = neqsym;
		lex_index++;
	}
	else if (strcmp(input, ":=") == 0) {
		list[lex_index].type = assignsym;
		lex_index++;
	}
	else if (strcmp(input, "<") == 0)
	{
		list[lex_index].type = lsssym;
		lex_index++;
	}
	else if (strcmp(input, "<=") == 0)
	{
		list[lex_index].type = leqsym;
		lex_index++;
	}
	else if (strcmp(input, ">") == 0)
	{
		list[lex_index].type = gtrsym;
		lex_index++;
	}
	else if (strcmp(input, ">=") == 0)
	{
		list[lex_index].type = geqsym;
		lex_index++;
	}
	else if (input[0] == '%')
	{
		list[lex_index].type = modsym;
		lex_index++;
	}
	else if (input[0] == '*')
	{
		list[lex_index].type = multsym;
		lex_index++;
	}
	else if (input[0] == '/')
	{
		list[lex_index].type = divsym;
		lex_index++;
	}
	else if (input[0] == '+')
	{
		list[lex_index].type = addsym;
		lex_index++;
	}
	else if (input[0] == '-')
	{
		list[lex_index].type = subsym;
		lex_index++;
	}
	else if (input[0] == '(')
	{
		list[lex_index].type = lparensym;
		lex_index++;
	}
	else if (input[0] == ')')
	{
		list[lex_index].type = rparensym;
		lex_index++;
	}
	else if (input[0] == ',')
	{
		list[lex_index].type = commasym;
		lex_index++;
	}
	else if (input[0] == '.')
	{
		list[lex_index].type = periodsym;
		lex_index++;
	}
	else if (input[0] == ';')
	{
		list[lex_index].type = semicolonsym;
		lex_index++;
	}
	else if (isalpha(input[0]) != 0)
	{
		list[lex_index].type = identsym;
		lex_index++;
	}
	else if (isdigit(input[0]) != 0)
	{
		list[lex_index].type = numbersym;
		lex_index++;
	}
	else {
		printlexerror(1);
		exit(0);
	}


}

lexeme* lexanalyzer(char* input, int flag)
{
	list = calloc(MAX_NUMBER_TOKENS, sizeof(lexeme)); //Allocates list to size 500
	int count = 0;
	lex_index = 0;

	while (input[count] != '\0') {			//Runs through the string until it reaches the EOF
		int start = 0;
		if (iscntrl(input[count]) != 0 || isspace(input[count]) != 0) {	//Skips spaces and control inputs
			count++;
			continue;
		}
		else if (isalpha(input[count]) != 0) {	//If the first character is alphabetical, it checks to see if it is a reserved word or an identifier
			start = count;
			for (count <= start + 11; count++;) {
				if (count - start > 11) {
					printlexerror(4);
					return NULL;
				}
				else if (isdigit(input[count]) != 0) {
					continue;
				}
				else if (isalpha(input[count]) == 0) {
					strncpy(list[lex_index].name, &input[start], count - start);
					inputcheck(list[lex_index].name);
					break;
				}
			}
		}
		else if (isdigit(input[count]) != 0) {	//If the first character is a digit, it makes sure it is no longer than 5 characters and contains no numbers.
			start = count;
			char temp[5];
			for (count <= start + 5; count++;) {
				if (count - start > 5) {
					printlexerror(3);
					return NULL;
				}
				else if (isalpha(input[count]) != 0) {
					printlexerror(2);
					return NULL;
				}
				else if (isdigit(input[count]) == 0) {
					strncpy(list[lex_index].name, &input[start], count - start);
					break;
				}
			}
			list[lex_index].value = atoi(list[lex_index].name);
			list[lex_index].type = numbersym;
			lex_index++;
		}
		else if (input[count] == '/') {		//Comment trap
			start = count;
			if (input[count + 1] == '/') {
				while (input[count] != '\n') {
					count++;
				}
			}
			else if (input[count + 1] == '*') {
				while (input[count] != '*') {
					if (input[count + 1] == '/')
						break;
					else if (input[count] == '\0') {
						printlexerror(5);
						return NULL;
					}
					count++;
				}
			}
		}
		else if (input[count] == '<') {	//Less than trap
			if (input[count + 1] == '>') {
				list[lex_index].type = neqsym;
				lex_index++;
				count += 2;
			}
			else if (input[count + 1] == '=') {
				list[lex_index].type = leqsym;
				lex_index++;
				count += 2;
			}
			else {
				list[lex_index].type = lsssym;
				lex_index++;
				count++;
			}
		}
		else if (input[count] == '>') {	//Greater than trap
			if (input[count + 1] == '=') {
				list[lex_index].type = geqsym;
				lex_index++;
				count += 2;
			}
			else {
				list[lex_index].type = gtrsym;
				lex_index++;
				count++;
			}

		}
		else if (input[count] == ':') {	//Assignment trap
			if (input[count + 1] == '=') {
				list[lex_index].type = assignsym;
				lex_index++;
				count += 2;
			}
			else {
				printlexerror(1);
				return NULL;
			}
		}
		else if (input[count] == '=') {	//Comparison trap
			if (input[count] == '=') {
				list[lex_index].type = eqlsym;
				lex_index++;
				count += 2;
			}
			else {
				printlexerror(1);
				return NULL;
			}
		}
		else {
			inputcheck(&input[count]);	//Symbol trap
			count++;
		}
	}
	printtokens();	//Prints all indexes in the lexeme
	return list;
}


void printtokens()
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (list[i].type)
		{
		case oddsym:
			printf("%11s\t%d", "odd", oddsym);
			break;
		case eqlsym:
			printf("%11s\t%d", "==", eqlsym);
			break;
		case neqsym:
			printf("%11s\t%d", "!=", neqsym);
			break;
		case lsssym:
			printf("%11s\t%d", "<", lsssym);
			break;
		case leqsym:
			printf("%11s\t%d", "<=", leqsym);
			break;
		case gtrsym:
			printf("%11s\t%d", ">", gtrsym);
			break;
		case geqsym:
			printf("%11s\t%d", ">=", geqsym);
			break;
		case modsym:
			printf("%11s\t%d", "%", modsym);
			break;
		case multsym:
			printf("%11s\t%d", "*", multsym);
			break;
		case divsym:
			printf("%11s\t%d", "/", divsym);
			break;
		case addsym:
			printf("%11s\t%d", "+", addsym);
			break;
		case subsym:
			printf("%11s\t%d", "-", subsym);
			break;
		case lparensym:
			printf("%11s\t%d", "(", lparensym);
			break;
		case rparensym:
			printf("%11s\t%d", ")", rparensym);
			break;
		case commasym:
			printf("%11s\t%d", ",", commasym);
			break;
		case periodsym:
			printf("%11s\t%d", ".", periodsym);
			break;
		case semicolonsym:
			printf("%11s\t%d", ";", semicolonsym);
			break;
		case assignsym:
			printf("%11s\t%d", ":=", assignsym);
			break;
		case beginsym:
			printf("%11s\t%d", "begin", beginsym);
			break;
		case endsym:
			printf("%11s\t%d", "end", endsym);
			break;
		case ifsym:
			printf("%11s\t%d", "if", ifsym);
			break;
		case thensym:
			printf("%11s\t%d", "then", thensym);
			break;
		case elsesym:
			printf("%11s\t%d", "else", elsesym);
			break;
		case whilesym:
			printf("%11s\t%d", "while", whilesym);
			break;
		case dosym:
			printf("%11s\t%d", "do", dosym);
			break;
		case callsym:
			printf("%11s\t%d", "call", callsym);
			break;
		case writesym:
			printf("%11s\t%d", "write", writesym);
			break;
		case readsym:
			printf("%11s\t%d", "read", readsym);
			break;
		case constsym:
			printf("%11s\t%d", "const", constsym);
			break;
		case varsym:
			printf("%11s\t%d", "var", varsym);
			break;
		case procsym:
			printf("%11s\t%d", "procedure", procsym);
			break;
		case identsym:
			printf("%11s\t%d", list[i].name, identsym);
			break;
		case numbersym:
			printf("%11d\t%d", list[i].value, numbersym);
			break;
		}
		printf("\n");
	}
	printf("\n");
	printf("Token List:\n");
	for (i = 0; i < lex_index; i++)
	{
		if (list[i].type == numbersym)
			printf("%d %d ", numbersym, list[i].value);
		else if (list[i].type == identsym)
			printf("%d %s ", identsym, list[i].name);
		else
			printf("%d ", list[i].type);
	}
	printf("\n");
	list[lex_index++].type = -1;
}

void printlexerror(int type)
{
	if (type == 1)
		printf("Lexical Analyzer Error: Invalid Symbol\n");
	else if (type == 2)
		printf("Lexical Analyzer Error: Invalid Identifier\n");
	else if (type == 3)
		printf("Lexical Analyzer Error: Excessive Number Length\n");
	else if (type == 4)
		printf("Lexical Analyzer Error: Excessive Identifier Length\n");
	else
		printf("Implementation Error: Unrecognized Error Type\n");

	free(list);
	return;
}