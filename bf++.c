#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __has_include(<conio.h>)
    #include <conio.h>
    #define get_char _getch
#else
    #define get_char getchar
#endif

#define RTS_SIZE 100
#define TAPE_SIZE 10000
#define MAX_NAME_LENGTH 300
#define MAX_CODE_LENGTH 3000

typedef struct
{
    unsigned int ret[RTS_SIZE];
    int retp;
    int tape[TAPE_SIZE];
    int tape_ptr;
    int acc;
} State;

void push(State *state, int value)
{
    state->ret[state->retp++] = value;
}

int pop(State *state)
{
    return state->ret[state->retp--];
}

void writeStruct(const char *path, const void *data, size_t size, size_t count)
{
    FILE *file = fopen(path, "wb");
    if (file)
    {
        fwrite(data, size, count, file);
        fclose(file);
    }
}

void readStruct(const char *path, void *data, size_t size, size_t count)
{
    FILE *file = fopen(path, "rb");
    if (file)
    {
        fread(data, size, count, file);
        fclose(file);
    }
}

void error(int code, int fin)
{
    printf("\nError: %i\n Type: ", code);
    switch (code)
    {
    case 1:
        printf("Exit Signal\n");
        code = 0;
        break;
    case 2:
        printf("Invalid Instruction\n");
        break;
    default:
        printf("???\n");
    }
    if (fin)
        exit(code);
}

char *readFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (content)
    {
        fread(content, 1, length, file);
        content[length] = '\0';
    }

    fclose(file);
    return content;
}

int xcommand(State *state, char *command)
{
    int p = 0;
    while (p < strlen(command))
    {
        switch (command[p])
        {
        case '!':
            return 1;
        case '.':
            putchar(state->tape[state->tape_ptr]);
            break;
        case 'C':
            state->tape[state->tape_ptr] = 0;
            break;
        case '+':
            state->tape[state->tape_ptr]++;
            break;
        case '-':
            state->tape[state->tape_ptr]--;
            break;
        case 'i':
            state->acc++;
            break;
        case 'd':
            state->acc--;
            break;
        case '>':
            if (state->tape_ptr < TAPE_SIZE - 1)
                state->tape_ptr++;
            else
                state->tape_ptr = 0;
            break;
        case '<':
            if (state->tape_ptr > 0)
                state->tape_ptr--;
            else
                state->tape_ptr = TAPE_SIZE - 1;
            break;
        case ';': // set jump loc (jiz)
        {
            if (state->tape[state->tape_ptr] != 0)
            {
                while (p < strlen(command) && command[++p] != 'j')
                    ;
            } else {
                push(state, p - 1);
            }
        }
        break;
        case ':': // set jump loc (jnz)
        {
            if (state->tape[state->tape_ptr] == 0)
            {
                while (p < strlen(command) && command[++p] != 'J')
                    ;
            } else {
                push(state, p - 1);
            }
        }
        break;
        case 'j': // jiz
        {
            if (state->tape[state->tape_ptr] == 0)
            {
                p = state->ret[state->retp - 1];
            }
            else
            {
                pop(state);
            }
        }
        break;
        case 'J': // jnz
        {
            if (state->tape[state->tape_ptr] != 0)
            {
                p = state->ret[state->retp - 1];
            }
            else
            {
                pop(state);
            }
        }
        break;
        case 'c':
            state->acc = 0;
            break;
        case ',':
            state->tape[state->tape_ptr] = get_char();
        case '^':
            state->acc *= 2;
            break;
        case 'v':
            state->acc /= 2;
            break;
        case 's':
            state->tape[state->tape_ptr] = state->acc;
            break;
        case 'l':
            state->acc = state->tape[state->tape_ptr];
            break;
        case '"':
            while (p < strlen(command) && command[++p] != '"')
                ;
            break;
        case '@':
            while (p < strlen(command) && command[++p] != '\n')
                ;
            break;
        case ' ':
        case '\n':
        case '\0':
        case '\t':
            break;
        default:
            printf("\nInvalid command: %c\n", command[p]);
            return 2;
        }
        p++;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    State state = {0};
    int err;
    char *file_content = {0};
    if (argc == 1)
    {
        printf("BF++ REPL [0.1]");
        char buffer[100];
        while (1)
        {
            printf(
                "\n"
                "Tape Pos: %04i\n"
                "     Acc: %04i\n"
                "Cur Cell: %04i\n"
                "] ",
                state.tape_ptr, state.acc, state.tape[state.tape_ptr]);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strcmp(buffer, ":reset") == 0)
            {
                memset(&state, 0, sizeof(state));
            }
            else if ((err = xcommand(&state, buffer)))
            {
                error(err, 0);
            }
        }
    }
    else if ((file_content = readFile(argv[1])) != NULL)
    {
        int err = 0;
        if ((err = xcommand(&state, file_content)))
            error(err, 0);
    }
    else if (file_content == NULL)
        printf("Invalid file path: %s\n", argv[1]);
    else if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        printf(
            "Brainf**k++\n\n"
            "Usage: %s <file>\n",
            argv[0]);
    }
    return 0;
}
