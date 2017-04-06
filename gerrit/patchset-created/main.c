#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    char *commit=0;
    char cmd[1024];
    int i;

    printf("patchset-created\n");
#if 0
    for(i=0; i<argc;i++) {
        printf("(%d)%s ", i, argv[i]);
    }
#endif
    if(argc >= 18) {
        commit = argv[18];
    }
    else {
        return 0;
    }

    sprintf(cmd,
        "ssh -p 29418 gerrit@127.0.0.1 gerrit review"
        " -m '\"You must review first, buddy!\"' --code-review -1 %s", commit);

        system(cmd);


    return 0;
}
