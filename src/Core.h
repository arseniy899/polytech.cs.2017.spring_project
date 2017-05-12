#pragma once
void initAddons();
void changeScene(int scene);
void Log_i(const char * tag, const char *str, ...);
void Log_e(const char * tag, const char *str, ...);
int initVars();
int openLevel(int num);
void setNewScreen();
void renderScreen();
void convertConstCopy(const char *source, char **toChr);
int openFolderDialog(int i);
extern yOffset;
int sliceFile(int i);
int nextPage(int i);
int prevPage(int i);
int openLevelSelect(int i);
int showDirectoryListing(int i);
int openLevelEditor(int i);
int ExitProg(int i);
extern char names [128][128];
extern char * pathCur;
extern char names [128][128];
extern char levelsPaths[256][128];
extern char levelsNames[64][BUTTONS_NAME_SIZE];
extern int levelSelectPage, yOffset;
