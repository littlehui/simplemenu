#ifndef LOGIC
#define LOGIC
void HW_Init();
void setCPU(uint32_t mhz);
void readConfig();
void quit();
char *determineExecutable(char *fileToBeExecutedWithFullPath);
void executeFavorite (char fileToBeExecutedWithFullPath[]);
void executeCommand (char executable[], char fileToBeExecutedWithFullPath[]);
void loadGameList();
#endif