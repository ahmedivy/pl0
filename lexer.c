#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    elsesym
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <input>\n", argv[0]);
        return 1;
    }

    // read file
    readFile(argv[1]);

    // tokenize
    tokenize();

    // print source from tokens
    printf("Source Program:\n%s\n", source);

    // print lexeme table
    printf("\nLexeme Table:\n");
    printf("%-15s %s\n", "lexeme", "token type");
    for (int i = 0; i < tokenCount; i++)
    {
        printf("%-15s ", tokens[i].value);
        if (tokens[i].type == KEYWORD)
        {
            printf("%d\n", getKeywordValue(tokens[i].value));
        }
        else if (tokens[i].type == IDENTIFIER)
        {
            // check if length is greater than 11
            if (strlen(tokens[i].value) > 11)
                printf("Error: Name is too long\n");
            else
                printf("%d\n", identsym);
        }
        else if (tokens[i].type == NUMBER)
        {
            if (strlen(tokens[i].value) > 5)
                printf("Error: Number is too long\n");
            else
                printf("%d\n", numbersym);
        }
        else if (tokens[i].type == SYMBOL)
        {
            int value = getSymbolValue(tokens[i].value);
            if (value == -1)
                printf("Error: Invalid symbol\n");
            else
                printf("%d\n", value);
        }
    }

    // print lexeme list
    printf("\nToken List:\n");
    for (int i = 0; i < tokenCount; i++)
    {
        if (tokens[i].type == KEYWORD)
        {
            printf("%d ", getKeywordValue(tokens[i].value));
        }
        else if (tokens[i].type == IDENTIFIER)
        {
            if (strlen(tokens[i].value) > 11)
                continue;
            printf("%d %s ", identsym, tokens[i].value);
        }
        else if (tokens[i].type == NUMBER)
        {
            if (strlen(tokens[i].value) > 5)
                continue;
            printf("%d %s ", numbersym, tokens[i].value);
        }
        else if (tokens[i].type == SYMBOL)
        {
            int value = getSymbolValue(tokens[i].value);
            if (value != -1)
                printf("%d ", value);
        }
    }
    printf("\n");

    return 0;
}