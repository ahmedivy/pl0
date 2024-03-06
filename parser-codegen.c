#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SYMBOL_TABLE_SIZE 500

// prototypes
void readFile(char *filename);
void tokenize();
void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void condition();
void expression();
void term();
void factor();
void printError(int i);
int symbolTableCheck(char *name);
void addSymbol(int kind, char *name, int val, int level, int addr);
void emit(int OP, int L, int M);

typedef enum
{
    skipsym = 1,
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    fisym,
    eqsym,
    neqsym,
    lessym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym,
    oddsym, // modified
} token_type;

char *reserved_words[] = {
    "const",
    "var",
    "procedure",
    "call",
    "begin",
    "end",
    "if",
    "fi",
    "then",
    "else",
    "while",
    "do",
    "read",
    "write"};

char *symbols[] = {
    "+",
    "-",
    "*",
    "/",
    "(",
    ")",
    ":=",
    "<=",
    ">=",
    ",",
    ".",
    "<",
    ">",
    ";",
    ":",
    "!=",
    "="};

char source[5012];
int tokenCount = 0;

typedef struct
{
    int kind;      // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    int mark;      // to indicate unavailable or deleted
} symbol;

symbol symbol_table[SYMBOL_TABLE_SIZE];

typedef enum
{
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    SYMBOL
} TokenType;

typedef struct
{
    TokenType type;
    char value[100];
} Token;

Token tokens[1024];

typedef struct
{
    int OP;
    int L;
    int M;
} INS;

INS code[1024];

char *opcodes[10] = {"LIT", "OPR", "LOD", "STO", "CAL",
                     "INC", "JMP", "JPC", "SYS", "ERR"};
char *syscodes[3] = {"SOU", "SIN", "EOP"};
char *operations[12] = {"RTN", "ADD", "SUB", "MUL", "DIV", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ", "ODD"};

int currentToken = 0;
int numVars = 0;
int symbolTableIndex = 0;
int currentCodeIndex = 0;

int level = 0;

void emit(int OP, int L, int M)
{
    code[currentCodeIndex].OP = OP;
    code[currentCodeIndex].L = L;
    code[currentCodeIndex].M = M;
    currentCodeIndex++;
}

int is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isSymbol(char c)
{
    return !isalnum((unsigned char)c) && !isspace((unsigned char)c) && c != '_' && c != '\0' && c != '\n' && c != '\r' && c != '\t';
}

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int starts_with(char *str, char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

void tokenize()
{
    int i = 0;

    while (source[i] != '\0')
    {
        // whitespace
        if (source[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r' || source[i] == '\0')
        {
            i++;
            continue;
        }

        // reserved words
        for (int j = 0; j < 14; j++)
        {
            if (starts_with(&source[i], reserved_words[j]))
            {
                strcpy(tokens[tokenCount].value, reserved_words[j]);
                tokens[tokenCount].type = KEYWORD;
                tokenCount++;
                i += strlen(reserved_words[j]);
                continue;
            }
        }

        // symbols
        for (int j = 0; j < 17; j++)
        {
            if (starts_with(&source[i], symbols[j]))
            {
                strcpy(tokens[tokenCount].value, symbols[j]);
                tokens[tokenCount].type = SYMBOL;
                tokenCount++;
                i += strlen(symbols[j]);
                continue;
            }
        }

        // identifier
        if (is_letter(source[i]))
        {
            int j = 0;
            while (is_letter(source[i]) || is_digit(source[i]))
            {
                tokens[tokenCount].value[j] = source[i];
                i++;
                j++;
            }
            tokens[tokenCount].value[j] = '\0';
            tokens[tokenCount].type = IDENTIFIER;
            tokenCount++;
            continue;
        }

        // number
        if (is_digit(source[i]))
        {
            int j = 0;
            while (is_digit(source[i]))
            {
                tokens[tokenCount].value[j] = source[i];
                i++;
                j++;
            }
            tokens[tokenCount].value[j] = '\0';
            tokens[tokenCount].type = NUMBER;
            tokenCount++;
            continue;
        }

        // comment
        if (source[i] == '/')
        {
            if (source[i + 1] == '*')
            {
                i += 2;
                while (source[i] != '*' && source[i + 1] != '/')
                    i++;
                i += 2;
                continue;
            }
        }

        // single character symbols
        if (isSymbol(source[i]))
        {
            tokens[tokenCount].value[0] = source[i];
            tokens[tokenCount].value[1] = '\0';
            tokens[tokenCount].type = SYMBOL;
            tokenCount++;
            i++;
            continue;
        }

        i++;
    }
}

void readFile(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error: Could not open file\n");
        exit(1);
    }
    char c;
    int i = 0;
    while ((c = fgetc(file)) != EOF)
        source[i++] = c;
    source[i] = '\0';
    fclose(file);
}

int getKeywordValue(char *keyword)
{
    if (strcmp(keyword, "const") == 0)
        return constsym;
    else if (strcmp(keyword, "var") == 0)
        return varsym;
    else if (strcmp(keyword, "procedure") == 0)
        return procsym;
    else if (strcmp(keyword, "call") == 0)
        return callsym;
    else if (strcmp(keyword, "begin") == 0)
        return beginsym;
    else if (strcmp(keyword, "end") == 0)
        return endsym;
    else if (strcmp(keyword, "if") == 0)
        return ifsym;
    else if (strcmp(keyword, "fi") == 0)
        return thensym;
    else if (strcmp(keyword, "then") == 0)
        return thensym;
    else if (strcmp(keyword, "else") == 0)
        return elsesym;
    else if (strcmp(keyword, "while") == 0)
        return whilesym;
    else if (strcmp(keyword, "do") == 0)
        return dosym;
    else if (strcmp(keyword, "read") == 0)
        return readsym;
    else if (strcmp(keyword, "write") == 0)
        return writesym;
    else
        return -1;
}

int getSymbolValue(char *symbol)
{
    if (strcmp(symbol, "+") == 0)
        return plussym;
    else if (strcmp(symbol, "-") == 0)
        return minussym;
    else if (strcmp(symbol, "*") == 0)
        return multsym;
    else if (strcmp(symbol, "/") == 0)
        return slashsym;
    else if (strcmp(symbol, "(") == 0)
        return lparentsym;
    else if (strcmp(symbol, ")") == 0)
        return rparentsym;
    else if (strcmp(symbol, "=") == 0)
        return eqsym;
    else if (strcmp(symbol, ",") == 0)
        return commasym;
    else if (strcmp(symbol, ".") == 0)
        return periodsym;
    else if (strcmp(symbol, "<") == 0)
        return lessym;
    else if (strcmp(symbol, ">") == 0)
        return gtrsym;
    else if (strcmp(symbol, ";") == 0)
        return semicolonsym;
    else if (strcmp(symbol, ":=") == 0)
        return becomessym;
    else if (strcmp(symbol, "<=") == 0)
        return leqsym;
    else if (strcmp(symbol, ">=") == 0)
        return geqsym;
    else if (strcmp(symbol, "!=") == 0)
        return neqsym;
    else
        return -1;
}

void printError(int i)
{
    switch (i)
    {
    case 0:
        printf("Error: Program must end with period\n");
        break;

    case 1:
        printf("Error: const, var, and read keywords must be followed by identifier\n");
        break;

    case 2:
        printf("Error: Symbol name has already been declared\n");
        break;

    case 3:
        printf("Error: Constants must be assigned with =\n");
        break;

    case 4:
        printf("Error: Constants must be assigned an integer value\n");
        break;

    case 5:
        printf("Error: Constant and variable declarations must be followed by a semicolon\n");
        break;

    case 6:
        printf("Error: Undeclared identifier\n");
        break;

    case 7:
        printf("Error: Only variable values may be altered\n");
        break;

    case 8:
        printf("Error: Assignment statements must use :=\n");
        break;

    case 9:
        printf("Error: Begin must be followed by end\n");
        break;

    case 10:
        printf("Error: If must be followed by then\n");
        break;

    case 11:
        printf("Error: While must be followed by do\n");
        break;

    case 12:
        printf("Error: Condition must contain comparison operator\n");
        break;

    case 13:
        printf("Error: Right parenthesis must follow left parenthesis\n");
        break;

    case 14:
        printf("Error: Arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
        break;

    default:
        break;
    }
}

int symbolTableCheck(char *name)
{
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
            return i;
    }
    return -1;
}

void addSymbol(int kind, char *name, int val, int level, int addr)
{
    symbol_table[symbolTableIndex].kind = kind;
    strcpy(symbol_table[symbolTableIndex].name, name);
    symbol_table[symbolTableIndex].val = val;
    symbol_table[symbolTableIndex].level = level;
    symbol_table[symbolTableIndex].addr = addr;
    symbol_table[symbolTableIndex].mark = 1;
    symbolTableIndex++;
}

void program()
{
    emit(7, 0, 3);
    block();
    if (strcmp(tokens[currentToken].value, ".") != 0)
        printError(0);
    emit(9, 0, 3);
}

void block()
{
    constDeclaration();
    numVars = varDeclaration();
    emit(6, 0, 3 + numVars);
    statement();
}

void constDeclaration()
{
    if (getKeywordValue(tokens[currentToken].value) == constsym)
    {
        do
        {
            currentToken++;
            if (tokens[currentToken].type != IDENTIFIER)
                printError(1);
            if (symbolTableCheck(tokens[currentToken].value) != -1)
                printError(2);
            char *name = tokens[currentToken].value;
            currentToken++;
            if (getSymbolValue(tokens[currentToken].value) != eqsym)
                printError(3);
            currentToken++;
            if (tokens[currentToken].type != NUMBER)
                printError(4);
            addSymbol(1, name, atoi(tokens[currentToken].value), 0, 0);
            currentToken++;
        } while (atoi(tokens[currentToken].value) == commasym);

        if (atoi(tokens[currentToken].value) != semicolonsym)
            printError(5);
        currentToken++;
    }
}

int varDeclaration()
{
    numVars = 0;
    if (getKeywordValue(tokens[currentToken].value) == varsym)
    {
        do
        {
            numVars++;
            currentToken++;
            if (tokens[currentToken].type != IDENTIFIER)
                printError(1);
            if (symbolTableCheck(tokens[currentToken].value) != -1)
                printError(2);
            addSymbol(2, tokens[currentToken].value, 0, 0, 2 + numVars);
            currentToken++;
        } while (getSymbolValue(tokens[currentToken].value) == commasym);
        if (getSymbolValue(tokens[currentToken].value) != semicolonsym)
            printError(5);
        currentToken++;
    }
    return numVars;
}

void statement()
{
    if (tokens[currentToken].type == IDENTIFIER)
    {
        int symIdx = symbolTableCheck(tokens[currentToken].value);
        if (symIdx == -1)
            printError(6);
        if (symbol_table[symIdx].kind != 2)
            printError(7);
        currentToken++;
        if (getSymbolValue(tokens[currentToken].value) != becomessym)
            printError(8);
        currentToken++;
        expression();
        // emit STO(M=table[symIdx].addr)
        emit(4, 0, symbol_table[symIdx].addr);
        return;
    }
    // if (atoi(tokens[currentToken].value) == beginsym)
    if (getKeywordValue(tokens[currentToken].value) == beginsym)
    {
        do
        {
            currentToken++;
            statement();
            // } while (atoi(tokens[currentToken].value) == semicolonsym);
        } while (getSymbolValue(tokens[currentToken].value) == semicolonsym);
        // if (atoi(tokens[currentToken].value) != endsym)
        if (getKeywordValue(tokens[currentToken].value) != endsym)
            printError(9);
        currentToken++;
        return;
    }
    if (atoi(tokens[currentToken].value) == ifsym)
    {
        currentToken++;
        condition();
        int jpcIdx = currentCodeIndex;
        // emit JPC
        emit(8, 0, 0);
        if (getKeywordValue(tokens[currentToken].value) != thensym)
            printError(10);
        currentToken++;
        statement();
        code[jpcIdx].M = currentCodeIndex;
        return;
    }
    if (getKeywordValue(tokens[currentToken].value) == whilesym)
    {
        currentToken++;
        int loopIdx = currentCodeIndex;
        condition();
        if (getKeywordValue(tokens[currentToken].value) != dosym)
            printError(11);
        currentToken++;
        int jpcIdx = currentCodeIndex;
        // emit JPC
        emit(8, 0, 0);
        statement();
        // emit JMP(M=loopIdx)
        emit(7, 0, loopIdx);
        code[jpcIdx].M = currentCodeIndex;
        return;
    }
    if (getKeywordValue(tokens[currentToken].value) == readsym)
    {
        currentToken++;
        if (tokens[currentToken].type != IDENTIFIER)
            printError(1);
        int symIdx = symbolTableCheck(tokens[currentToken].value);
        if (symIdx == -1)
            printError(6);
        if (symbol_table[symIdx].kind != 2)
            printError(4); // to be correct
        currentToken++;
        // emit READ
        emit(9, 0, 2);
        // emit STO(M=table[symIdx].addr)
        emit(4, 0, symbol_table[symIdx].addr);
        currentToken++;
        return;
    }
    if (getKeywordValue(tokens[currentToken].value) == writesym)
    {
        currentToken++;
        expression();
        // emit WRITE
        emit(9, 0, 1);
        return;
    }
}

void condition()
{
    expression();
    if (getSymbolValue(tokens[currentToken].value) == eqsym)
    {
        currentToken++;
        expression();
        // emit EQL
        emit(8, 0, 8);
    }
    else if (getSymbolValue(tokens[currentToken].value) == neqsym)
    {
        currentToken++;
        expression();
        // emit NEQ
        emit(8, 0, 9);
    }
    else if (getSymbolValue(tokens[currentToken].value) == lessym)
    {
        currentToken++;
        expression();
        // emit LSS
        emit(8, 0, 7);
    }
    else if (getSymbolValue(tokens[currentToken].value) == leqsym)
    {
        currentToken++;
        expression();
        // emit LEQ
        emit(8, 0, 8);
    }
    else if (getSymbolValue(tokens[currentToken].value) == gtrsym)
    {
        currentToken++;
        expression();
        // emit GTR
        emit(8, 0, 9);
    }
    else if (getSymbolValue(tokens[currentToken].value) == geqsym)
    {
        currentToken++;
        expression();
        // emit GEQ
        emit(8, 0, 10);
    }
    else
        printError(12);
}

void expression()
{
    if (getSymbolValue(tokens[currentToken].value) == minussym)
    {
        currentToken++;
        term();
        // emit NEG
        emit(2, 0, 1);
        while (getSymbolValue(tokens[currentToken].value) == plussym || getSymbolValue(tokens[currentToken].value) == minussym)
        {
            if (getSymbolValue(tokens[currentToken].value) == plussym)
            {
                currentToken++;
                term();
                // emit ADD
                emit(2, 0, 2);
            }
            else
            {
                currentToken++;
                term();
                // emit SUB
                emit(2, 0, 3);
            }
        }
    }
    else
    {
        if (getSymbolValue(tokens[currentToken].value) == plussym)
            currentToken++;
        term();
        while (getSymbolValue(tokens[currentToken].value) == plussym || getSymbolValue(tokens[currentToken].value) == minussym)
        {
            if (getSymbolValue(tokens[currentToken].value) == plussym)
            {
                currentToken++;
                term();
                // emit ADD
                emit(2, 0, 2);
            }
            else
            {
                currentToken++;
                term();
                // emit SUB
                emit(2, 0, 3);
            }
        }
    }
}

void term()
{
    factor();
    while (getSymbolValue(tokens[currentToken].value) == multsym || getSymbolValue(tokens[currentToken].value) == slashsym)
    {
        if (getSymbolValue(tokens[currentToken].value) == multsym)
        {
            currentToken++;
            factor();
            // emit MUL
            emit(2, 0, 3);
        }
        else
        {
            currentToken++;
            factor();
            // emit DIV
            emit(2, 0, 4);
        }
    }
}

void factor()
{
    if (tokens[currentToken].type == IDENTIFIER)
    {
        int symIdx = symbolTableCheck(tokens[currentToken].value);
        if (symIdx == -1)
            printError(6);
        if (symbol_table[symIdx].kind == 1)
        {
            // emit LIT(M=table[symIdx].val)
            emit(1, 0, symbol_table[symIdx].val);
        }
        else if (symbol_table[symIdx].kind == 2)
        {
            // emit LOD(M=table[symIdx].addr)
            emit(3, 0, symbol_table[symIdx].addr);
        }
        else
            printError(7);
        currentToken++;
    }
    // else if (atoi(tokens[currentToken].value) == numbersym)
    else if (tokens[currentToken].type == NUMBER)
    {
        // emit LIT
        emit(1, 0, atoi(tokens[currentToken].value));
        currentToken++;
    }
    else if (
        // atoi(tokens[currentToken].value) == lparentsym)
        getSymbolValue(tokens[currentToken].value) == lparentsym)
    {
        currentToken++;
        expression();
        if (atoi(tokens[currentToken].value) != rparentsym)
            printError(13);
        currentToken++;
    }
    else
        printError(14);
}

int main(int argc, char *argv[])
{
    // if (argc < 2)
    // {
    //     printf("Usage: %s <input>\n", argv[0]);
    //     return 1;
    // }

    // read file
    // readFile(argv[1]);
    readFile("inputs/input.txt");

    // tokenize
    tokenize();

    // checking error
    for (int i = 0; i < tokenCount; i++)
    {
        if (tokens[i].type == IDENTIFIER && strlen(tokens[i].value) > 11)
            printf("Error: Name is too long\n");
        else if (tokens[i].type == NUMBER && strlen(tokens[i].value) > 5)
            printf("Error: Number is too long\n");
        else if (tokens[i].type == SYMBOL && getSymbolValue(tokens[i].value) == -1)
            printf("Error: Invalid symbol\n");
    }

    program();

    // print assembly code
    printf("Assembly code:\n");
    printf("Line\tOP\tL\tM\n");
    for (int i = 0; i < currentCodeIndex; i++)
    {
        printf("  %d\t%s\t%d\t%d\n", i, opcodes[code[i].OP - 1], code[i].L, code[i].M);
    }

    // print symbol table
    printf("\nSymbol Table:\n");
    printf("Kind | Name           | Value | Level | Address | Mark\n");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < symbolTableIndex; i++)
    {
        printf("  %d  | %14s | %5d | %5d | %7d | %4d\n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].addr, symbol_table[i].mark);
    }
    return 0;
}