#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 150
#define MAX_SYMBOL_COUNT 50
#define MAX_REG_HEIGHT 10

instruction *code;
int cIndex;
symbol *table;
int tIndex;
int level;
int *num_of_variables;
int lindex = 0;
int procedure_table_index;
int procedure_code_index;

void emit(int opname, int level, int mvalue);
void addToSymbolTable(int k, char n[], int v, int l, int a, int m);
void mark();
int multipledeclarationcheck(char name[]);
int findsymbol(char name[], int kind);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();
void block(lexeme *list1, int lindex1);
void CONST_DECLARATION(lexeme *list1, int lindex1);
int VAR_DECLARATION(lexeme *list1,int lindex1);
void PROCEDURE_DECLARATION(lexeme *list1,int lindex1);
void STATEMENT(lexeme *list1);
void expression(lexeme *list1);
void condition(lexeme *list1);
void term(lexeme *list1);
void factor(lexeme *list1);




instruction *parse(lexeme *list, int printTable, int printCode)
{

	code = malloc(sizeof(instruction) * MAX_CODE_LENGTH);
	table = malloc(sizeof(symbol) * MAX_SYMBOL_COUNT);
	num_of_variables = calloc(50,sizeof(int));
	tIndex = 0;
	cIndex = 0;
	// Program
	{
		emit(7,0,1);
		addToSymbolTable(3,"main",0,0,0,0);
		level = -1;
		block(list,lindex);
		emit(9,0,3);
		for(int i =0;i<cIndex;i++)
		{
			if(code[i].opcode == 5)
				code[i].m = table[code[i].m].addr;
		}
    	code[0].m = table[0].addr;

	}

	if (printTable)
		printsymboltable();
	if (printCode)
		printassemblycode();

	code[cIndex].opcode = -1;
	return code;
}

// adds a line of code to the program
void emit(int opname, int level, int mvalue)
{
	code[cIndex].opcode = opname;
	code[cIndex].l = level;
	code[cIndex].m = mvalue;
	cIndex++;
}

// add a symbol to the symbol table
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

// mark the symbols belonging to the current procedure, should be called at the end of block
void mark()
{
	printf("NOW%d\n",lindex);
	int i;
	for (i = tIndex - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

// checks if a new symbol has a valid name, by checking if there's an existing symbol
// with the same name in the procedure
int multipledeclarationcheck(char name[])
{
	int i;
	for (i = 0; i < tIndex; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}

// returns the index of a symbol with a given name and kind in the symbol table
// returns -1 if not found
// prioritizes lower lex levels
int findsymbol(char name[], int kind)
{
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < tIndex; i++)
	{
		//printf("%s\n%d\n%d\n",table[i].name,table[i].kind,table[i].mark);
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
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
		case 20:
			printf("Parser Error: Register Overflow Error\n");
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

	//free(table);
	//table = NULL;
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
						printf("RET\t");
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
						printf("EQL\t");
						break;
					case 7:
						printf("NEQ\t");
						break;
					case 8:
						printf("LSS\t");
						break;
					case 9:
						printf("LEQ\t");
						break;
					case 10:
						printf("GTR\t");
						break;
					case 11:
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
void block(lexeme *list1,int lindex1)
{
	level++;
	procedure_table_index = tIndex - 1;
	procedure_code_index = cIndex;
	CONST_DECLARATION(list1,lindex1);
	int x;
	x = VAR_DECLARATION(list1,lindex1);
	PROCEDURE_DECLARATION(list1,lindex1);
	// printf("%d\n",procedure_index);
	table[procedure_table_index].addr = procedure_code_index;
	emit(6,0,x + 3);
	STATEMENT(list1);


	mark();
	level--;

}
void CONST_DECLARATION(lexeme *list1,int lindex1)
{
	if(list1[lindex].type == constsym)
		{
			lindex++;
			do {
				// if it is a identifier
				if (list1[lindex].type == identsym)
				{
					if (list1[lindex+1].type == assignsym)
					{
						if (list1[lindex+2].type == numbersym)
						{
							addToSymbolTable(1,list1[lindex].name,list1[lindex+2].value,level,0,0);
							lindex = lindex + 4;
						}
					}
					}
			} while(list1[lindex].type == identsym);
		}

}
int VAR_DECLARATION(lexeme *list1,int lindex1)
{
	int retval = 0;
	if (list1[lindex].type == varsym)
	{
		lindex++;
		do {
			if(list1[lindex].type == identsym)
			{
				addToSymbolTable(2,list1[lindex].name,0,level,num_of_variables[level]+3,0);
				num_of_variables[level]++;
				lindex = lindex + 2;
				retval++;
			}
		} while(list1[lindex].type == identsym);
	}
	return retval;
}
void PROCEDURE_DECLARATION(lexeme *list1,int lindex1)
{
	while(list1[lindex].type == procsym)
	{
		lindex = lindex + 1;
		if(list1[lindex].type == identsym)
		{
			lindex = lindex + 1;
			
			if(list1[lindex].type == semicolonsym)
			{
				addToSymbolTable(3,list1[lindex-1].name,0,level,0,0);
				lindex++;
				block(list1,lindex);
				//emit(2,0,0);
				//lindex = lindex + 1;
			}
		}
	}
}
void STATEMENT(lexeme *list1)
{
	//printf("statement\n");
	// printf("%d",lindex);
	if(list1[lindex].type == identsym)
	{
		//printsymboltable();
		int symIdx = findsymbol(list1[lindex].name,2);
		printf("level=%d\n",level);
		lindex++;
		if (list1[lindex].type == assignsym)
		{
			lindex++;
			expression(list1);
			emit(4,level-table[symIdx].level,table[symIdx].addr);
		}
	}

	else if(list1[lindex].type == beginsym)
	{
		do
		{
			lindex++;
			STATEMENT(list1);
		} while (list1[lindex].type==semicolonsym);

		lindex++;
		//return;
	}

	else if(list1[lindex].type == ifsym)
	{
		lindex++;
		condition(list1);
		int jpcIdx = cIndex;
		emit(8,0,0);

		lindex++;
		STATEMENT(list1);
		if(list1[lindex].type == elsesym)
		{
			int jmpIdx = cIndex;
			emit(7,0,1);
			code[jpcIdx].m = cIndex;
			lindex++;
			STATEMENT(list1);
			code[jmpIdx].m = cIndex;
		}
		else
			code[jpcIdx].m = cIndex;

		//return;
	}
	else if(list1[lindex].type == whilesym)
	{
		lindex++;
		int loopIdx = cIndex;

		condition(list1);

		lindex++;
		int jpcIdx = cIndex;

		emit(8,0,0);
		STATEMENT(list1);
		emit(7,0,loopIdx);
		code[jpcIdx].m = cIndex;

		lindex++;
		//return;
	}
	else if(list1[lindex].type == readsym)
	{
		lindex++;
		int symIdx = findsymbol(list1[lindex].name,2);

		emit(9,0,2);
		emit(4,level-table[symIdx].level,table[symIdx].addr);

		lindex++;
		//return;
	}
	else if(list1[lindex].type == writesym)
	{
		lindex++;
		expression(list1);

		emit(9,0,1);
		//return;
	}
	else if(list1[lindex].type == callsym)
	{
		int symIdx = findsymbol(list1[lindex].name,3);

		lindex++;
		emit(5,level-table[symIdx].level,symIdx);
	}
}

void condition(lexeme *list1)
{
	expression(list1);

	if(list1[lindex].type == eqlsym)
	{
		lindex++;
		expression(list1);
		emit(2,0,6);
	}
	else if(list1[lindex].type == neqsym)
	{
		lindex++;
		expression(list1);
		emit(2,0,7);
	}
	else if(list1[lindex].type == lsssym)
	{
		lindex++;
		expression(list1);
		emit(2,0,8);
	}
	else if(list1[lindex].type == leqsym)
	{
		lindex++;
		expression(list1);
		emit(2,0,9);
	}
	else if(list1[lindex].type ==  gtrsym)
	{
		lindex++;
		expression(list1);
		emit(2,0,10);
	}
	else if(list1[lindex].type == geqsym)
	{
		lindex++;
		expression(list1);
		emit(2,0,11);
	}
}

void expression(lexeme *list1)
{
	if(list1[lindex].type == minussym)
	{
		lindex++;
		term(list1);
		emit(2,0,1);
		while (list1[lindex].type == plussym || list1[lindex].type == minussym)
		{

			if(list1[lindex].type == plussym)
			{
				lindex++;
				term(list1);
				emit(2,0,2);
			}
			else
			{
				lindex++;
				term(list1);
				emit(2,0,3);
			}
		}

	}
	else
	{
		if(list1[lindex].type == plussym)
		lindex++;
		term(list1);
		while(list1[lindex].type == plussym || list1[lindex].type == minussym)
		{
			if(list1[lindex].type == plussym)
			{
				lindex++;
				term(list1);
				emit(2,0,2);
			}
			else
			{
				lindex++;
				term(list1);
				emit(2,0,3);
			}
		}

	}
}

void term(lexeme *list1)
{
	factor(list1);
	while(list1[lindex].type == multsym || list1[lindex].type == divsym)
	{
		if(list1[lindex].type == multsym)
		{
			lindex++;
			factor(list1);
			emit(2,0,4);
		}
		else
		{
			lindex++;
			factor(list1);
			emit(2,0,5);
		}
	}
}

void factor(lexeme *list1)
{
	if(list1[lindex].type == identsym)
	{
		int symIdx_var = findsymbol(list1[lindex].name,2);
		int symIdx_const = findsymbol(list1[lindex].name,1);

		if(symIdx_const == 0 || table[symIdx_var].level < table[symIdx_const].level)
			emit(1,0,table[symIdx_const].val);
		else 
			emit(3,level-table[symIdx_var].level,table[symIdx_var].addr);
		lindex++;
	}
	else if(list1[lindex].type == numbersym)
	{
		emit(1,0,list1[lindex].value);
		lindex++;
	}
	else if(list1[lindex].type == lparensym)
	{
		lindex++;
		expression(list1);
		lindex++;
	}

}
