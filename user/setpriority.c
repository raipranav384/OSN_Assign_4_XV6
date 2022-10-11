#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stdbool.h>

bool isValidPID(int PID)
{
    if (PID >= 0)
        return true;

    return false;
}
bool isValidPriority(int newPriority)
{
    if (newPriority >= 0 && newPriority <= 100)
        return true;

    return false;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Error: Not Sufficient Arguments Provided\n");
    }

    int newPriority = atoi(argv[1]);
    int processPID = atoi(argv[2]);

    if (isValidPriority(newPriority) && isValidPID(processPID))
    {
        setpriority(newPriority, processPID);
    }

    return -1;
}