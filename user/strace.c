#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int i;
    char *commandToExecute[MAXARG];

    if (argc < 3)
    {
        printf("Error!");
        exit(1);
    }

    int procListMask = atoi(argv[1]);
    strace(procListMask);

    //Copying commands after mask
    for (i = 2; i < argc && i < MAXARG; i++)
    {
        commandToExecute[i - 2] = argv[i];
    }
    //Execute commands after mask
    exec(commandToExecute[0], commandToExecute);
    exit(0);
}