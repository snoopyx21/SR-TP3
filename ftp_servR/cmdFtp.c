#include "cmdFtp.h"

t_symcle tabcle[NBCLE] = 
{
    { "open",       CMD_OPEN,       4}, 
    { "dir",        CMD_DIR,        3}, 
    { "show",       CMD_SHOW,       4}, 
    { "ciao",       CMD_CIAO,       4}, 
    { "debugon",    CMD_DEBUGON,    7}, 
    { "debugoff",   CMD_DEBUGOFF,   8},
    { "get",        CMD_GET,        3}, 
    { "send",       CMD_SEND,       4}, 
    { "ren",        CMD_REN,        3}, 
    { "del",        CMD_DEL,        3}, 
    { "cd",         CMD_CD,         2}, 
    { "mkd",        CMD_MKD,        3},
    { "rmd",        CMD_RMD,        3}, 
    { "exit",       CMD_EXIT,       4}
};

int command(char * command_client)
{
    int i;

    for (i = 0 ; i < NBCLE ; i++) 
    {

        t_symcle *sym = &tabcle[i];
        if (strncmp(sym->tcommand, command_client, sym->size) == 0)
        {

            return sym->val;
        }
    }
    fprintf(stderr, "Unknown command\n");
    return ERROR;
}
