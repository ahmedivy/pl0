//Jadyn Coleman

#include <stdio.h>

#define MAX_PAS_SIZE 500

typedef struct
{
    int OP;
    int L;
    int M;
} INS;

int BP, SP, PC;
INS IR;
int PAS[MAX_PAS_SIZE] = {0};

// function to load program into PAS
void loadProgram(const char *filename)
{
    FILE *fp;
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file\n");
        return;
    }

    int i = 0;
    while (fscanf(fp, "%d %d %d", &PAS[i], &PAS[i + 1], &PAS[i + 2]) != EOF && i < MAX_PAS_SIZE)
    {
        i += 3;
    }

    fclose(fp);
}

int base(int BP, int L)
{
    int arb = BP; // arb = activation record base
    while (L > 0) // find base L levels down
    {
        arb = PAS[arb];
        L--;
    }
    return arb;
}

// Array for printing opcodes
char *opcodes[10] = {"LIT", "OPR", "LOD", "STO", "CAL",
                     "INC", "JMP", "JPC", "SYS", "ERR"};
char *syscodes[3] = {"SOU", "SIN", "EOP"};
char *operations[12] = {"RTN", "ADD", "SUB", "MUL", "DIV", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ", "ODD"};

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    // load program into PAS
    loadProgram(argv[1]);

    // initialize registers
    SP = MAX_PAS_SIZE;
    BP = SP - 1;
    PC = 0;
    IR.OP = 0;
    IR.L = 0;
    IR.M = 0;

    // print header and initial values
    printf("                PC      BP      SP      Stack\n");
    printf("Initial values: %-3d     %-3d     %-3d\n\n", PC, BP, SP);

    int EOP = 0;
    while (!EOP)
    {

        // fetch
        IR.OP = PAS[PC];
        IR.L = PAS[PC + 1];
        IR.M = PAS[PC + 2];
        PC += 3;

        // execute
        switch (IR.OP)
        {
        case 1: // LIT
            PAS[--SP] = IR.M;
            break;

        case 2: // OPR
            switch (IR.M)
            {
            case 0: // RTN
                SP = BP + 1;
                BP = PAS[SP - 2];
                PC = PAS[SP - 3];
                break;

            case 1: // ADD
                PAS[SP + 1] = PAS[SP + 1] + PAS[SP];
                SP++;
                break;

            case 2: // SUB
                PAS[SP + 1] = PAS[SP + 1] - PAS[SP];
                SP++;
                break;

            case 3: // MUL
                PAS[SP + 1] = PAS[SP + 1] * PAS[SP];
                SP++;
                break;

            case 4: // DIV
                PAS[SP + 1] = PAS[SP + 1] / PAS[SP];
                SP++;
                break;

            case 5: // EQL
                PAS[SP + 1] = PAS[SP + 1] == PAS[SP];
                SP++;
                break;

            case 6: // NEQ
                PAS[SP + 1] = PAS[SP + 1] != PAS[SP];
                SP++;
                break;

            case 7: // LSS
                PAS[SP + 1] = PAS[SP + 1] < PAS[SP];
                SP++;
                break;

            case 8: // LEQ
                PAS[SP + 1] = PAS[SP + 1] <= PAS[SP];
                SP++;
                break;

            case 9: // GTR
                PAS[SP + 1] = PAS[SP + 1] > PAS[SP];
                SP++;
                break;

            case 10: // GEQ
                PAS[SP + 1] = PAS[SP + 1] >= PAS[SP];
                SP++;
                break;

            case 11: // ODD
                PAS[SP] = PAS[SP] % 2;
                break;

            default:
                break;
            }
            break;

        case 3: // LOD
            PAS[--SP] = PAS[base(BP, IR.L) - IR.M];
            break;

        case 4: // STO
            PAS[base(BP, IR.L) - IR.M] = PAS[SP];
            SP++;
            break;

        case 5: // CAL
            PAS[SP - 1] = base(BP, IR.L);
            PAS[SP - 2] = BP;
            PAS[SP - 3] = PC;
            BP = SP - 1;
            PC = IR.M;
            break;

        case 6: // INC
            SP -= IR.M;
            break;

        case 7: // JMP
            PC = IR.M;
            break;

        case 8: // JPC
            if (PAS[SP++] == 0)
                PC = IR.M;
            break;

        case 9:
            switch (IR.M)
            {
            case 1:
                printf("Output result is: %d\n", PAS[SP++]);
                break;

            case 2:
                printf("Please Enter an integer: ");
                scanf("%d", &PAS[--SP]);
                break;

            case 3:
                EOP = 1;
                break;

            default:
                break;
            }

        default:
            break;
        }

        // print instruction
        char *opCode;
        if (IR.OP == 9)
            opCode = syscodes[IR.M - 1];
        else if (IR.OP == 2)
            opCode = operations[IR.M];
        else
            opCode = opcodes[IR.OP - 1];

        printf("  %s %d %-8d", opCode, IR.L, IR.M);

        // print registers
        printf("%-3d     %-3d     %-3d     ", PC, BP, SP);

        // print stack
        for (int i = MAX_PAS_SIZE - 1; i >= SP; i--)
        {
            if (PAS[i] == 499 && PAS[i + 1] != 499)
                printf("| ");
            printf("%d ", PAS[i]);
        }
        printf("\n");
    }
    return 0;
}