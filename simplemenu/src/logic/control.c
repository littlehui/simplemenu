#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined TARGET_RG350 || defined TARGET_RG350_BETA
#include <shake.h>
#endif

#include "../headers/config.h"
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/graphics.h"
#include "../headers/logic.h"
#include "../headers/screen.h"
#include "../headers/string_utils.h"
#include "../headers/system_logic.h"
#include "../headers/doubly_linked_rom_list.h"
#include "../headers/utils.h"

int switchToGroup;

void scrollUp() {
	CURRENT_SECTION.currentGameNode=CURRENT_SECTION.currentGameNode->prev;
	//if at the first item of the first page
	if (CURRENT_SECTION.currentGameNode==NULL) {
		//go to the last page
		CURRENT_SECTION.currentPage=CURRENT_SECTION.totalPages;
		//go to the last game whether on the same page or not
		if (CURRENT_SECTION.totalPages==0) {
			CURRENT_SECTION.currentGameInPage=CURRENT_SECTION.gameCount-1;
		} else {
			if (CURRENT_SECTION.gameCount%(ITEMS_PER_PAGE)!=0) {
				CURRENT_SECTION.currentGameInPage=(CURRENT_SECTION.gameCount%ITEMS_PER_PAGE)-1;
			} else {
				CURRENT_SECTION.currentGameInPage=ITEMS_PER_PAGE-1;
			}
		}
		//point to the last node
		CURRENT_SECTION.currentGameNode=CURRENT_SECTION.tail;
	} else {
		//it's a full page and not the first item
		if (CURRENT_SECTION.currentGameInPage > 0) {
			//decrease selected game
			CURRENT_SECTION.currentGameInPage--;
		} else {
			//decrease page
			CURRENT_SECTION.currentPage--;
			//go to the last ekement of the page
			CURRENT_SECTION.currentGameInPage=ITEMS_PER_PAGE-1;
		}
	}
	//establish real game number
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void scrollDown() {
	CURRENT_SECTION.currentGameNode=CURRENT_SECTION.currentGameNode->next;
	// if at the end end of the list
	if (CURRENT_SECTION.currentGameNode==NULL) {
		//restart pages
		CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
		CURRENT_SECTION.currentPage=0;
		CURRENT_SECTION.currentGameInPage=0;
	} else {
		// it's a filled page and not the last item
		if (CURRENT_SECTION.currentGameInPage < ITEMS_PER_PAGE-1) {
			// increase selected game
			CURRENT_SECTION.currentGameInPage++;
		} else {
			CURRENT_SECTION.currentPage++;
			CURRENT_SECTION.currentGameInPage=0;
		}
	}
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void scrollToGame(int gameNumber) {
	if (CURRENT_SECTION.gameCount < gameNumber) {
		CURRENT_SECTION.currentGameInPage=0;
		CURRENT_SECTION.currentPage=0;
		CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
		return;
	}
	int pages = CURRENT_SECTION.gameCount / ITEMS_PER_PAGE;
	if (CURRENT_SECTION.gameCount%ITEMS_PER_PAGE==0) {
		pages--;
	}
	CURRENT_SECTION.totalPages=pages;
	CURRENT_SECTION.currentGameInPage=0;
	CURRENT_SECTION.currentPage=0;
	CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
	while (CURRENT_GAME_NUMBER<gameNumber) {
		scrollDown();
	}
}

int advanceSection(int showLogo) {
	logMessage("INFO","Advancing section");
	if(currentSectionNumber==favoritesSectionNumber) {
		return 0;
	}
	if(currentSectionNumber!=favoritesSectionNumber&&currentSectionNumber<favoritesSectionNumber-1) {
		currentSectionNumber++;
	} else if (currentSectionNumber!=favoritesSectionNumber) {
		currentSectionNumber=0;
	}
	if (CURRENT_SECTION.systemLogoSurface == NULL) {
		logMessage("INFO",CURRENT_SECTION.sectionName);
		logMessage("INFO","advanceSection - Loading system logo");
		CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
		resizeSectionSystemLogo(&CURRENT_SECTION);
	}
	#ifndef TARGET_BITTBOY
	if ((fullscreenMode||showLogo)&&currentSectionNumber!=favoritesSectionNumber) {
		currentlySectionSwitching=1;
		displayBackgroundPicture();
		logMessage("INFO","Displaying system logo");
		showConsole();
		refreshScreen();
	}
	#else
	if ((fullscreenMode||showLogo)&&currentSectionNumber!=favoritesSectionNumber) {
		displayBackgroundPicture();
		showConsole();
		refreshScreen();
	}
	#endif
	return 1;
}

int rewindSection(int showLogo) {
	logMessage("INFO","Rewinding section");
	if(currentSectionNumber!=favoritesSectionNumber&&currentSectionNumber>0) {
		currentSectionNumber--;
	} else if (currentSectionNumber!=favoritesSectionNumber) {
		currentSectionNumber=menuSectionCounter-2;
	}
	if (CURRENT_SECTION.systemLogoSurface == NULL) {
		logMessage("INFO",CURRENT_SECTION.sectionName);
		logMessage("INFO","rewindSection - Loading system logo");
		CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
		resizeSectionSystemLogo(&CURRENT_SECTION);
	}
	#ifndef TARGET_BITTBOY
	if ((fullscreenMode||showLogo)&&currentSectionNumber!=favoritesSectionNumber) {
		currentlySectionSwitching=1;
		displayBackgroundPicture();
		logMessage("INFO","Displaying system logo");
		showConsole();
		refreshScreen();
	}
	#else
	if ((fullscreenMode||showLogo)&&currentSectionNumber!=favoritesSectionNumber) {
		displayBackgroundPicture();
		showConsole();
		refreshScreen();
	}
	#endif
	if(currentSectionNumber!=favoritesSectionNumber) {
		return 1;
	}
	return 0;
}

void launchGame(struct Rom *rom) {
	FILE *file=NULL;
	char *error=malloc(3000);
	char tempExec[3000];

	char tempExecDirPlusFileName[3000];
	char tempExecFile[3000];

	if (favoritesSectionSelected && favoritesSize > 0) {
		struct Favorite favorite = favorites[CURRENT_GAME_NUMBER];
		strcpy(tempExec,favorite.emulatorFolder);
		strcat(tempExec,favorite.executable);
		file = fopen(tempExec, "r");
		if (!file&&strstr(tempExec,"#")==NULL) {
			strcpy(error,favorite.executable);
			strcat(error,"-NOT FOUND");
			generateError(error,0);
			return;
		}
		#ifndef TARGET_PC
		executeCommand(favorite.emulatorFolder,favorite.executable,favorite.name);
		#else
		executeCommandPC(favorite.executable,favorite.name);
		#endif
	} else if (rom->name!=NULL) {
		strcpy(tempExec,CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.activeEmulatorDirectory]);
		strcpy(tempExecFile,CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable]);
		char *ptr = strtok(tempExec, " ");
		strcpy(tempExecDirPlusFileName,CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.activeEmulatorDirectory]);
		strcat(tempExecDirPlusFileName,tempExecFile);
		file = fopen(ptr, "r");
		strcat(tempExec,CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable]);
		if (!file&&strstr(tempExec,"#")==NULL) {
			strcpy(error,tempExecDirPlusFileName);
			strcat(error,"-NOT FOUND");
			generateError(error,0);
			return;
		}
		if (CURRENT_SECTION.onlyFileNamesNoExtension) {
			#ifndef TARGET_PC
			executeCommand(CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.activeEmulatorDirectory], CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable],getGameName(rom->name));
			#else
			executeCommandPC(CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable],getGameName(rom->name));
			#endif
		} else {
			#ifdef TARGET_PC
			executeCommandPC(CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable],rom->name);
			#else
			executeCommand(CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.activeEmulatorDirectory], CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable],rom->name);
			#endif
		}
	}
}

void launchEmulator(struct Rom *rom) {
	if (favoritesSectionSelected && favoritesSize > 0) {
		struct Favorite favorite = favorites[CURRENT_GAME_NUMBER];
		#ifndef TARGET_PC
		executeCommand(favorite.emulatorFolder,favorite.executable,"*");
		#else
		executeCommandPC(favorite.executable,"*");
		#endif
	} else if (rom->name!=NULL) {
		#ifndef TARGET_PC
		executeCommand(CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.activeEmulatorDirectory], CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable],"*");
		#else
		executeCommandPC(CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable],"*");
		#endif
	}
}

void advancePage(struct Rom *rom) {
	if (rom==NULL||rom->name==NULL) {
		return;
	}
	if(CURRENT_SECTION.currentPage<=CURRENT_SECTION.totalPages) {
		if (CURRENT_SECTION.alphabeticalPaging) {
			char *currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
			char currentLetter=tolower(currentGame[0]);
			while((tolower(currentGame[0])==currentLetter||(isdigit(currentLetter)&&isdigit(currentGame[0])))) {
				scrollDown();
				free(currentGame);
				currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
				if(CURRENT_SECTION.currentGameNode==CURRENT_SECTION.head) {
					break;
				}
			}
			free(currentGame);
		} else {
			if(CURRENT_SECTION.currentPage!=CURRENT_SECTION.totalPages) {
				CURRENT_SECTION.currentPage++;
				for (int i=CURRENT_SECTION.currentGameInPage;i<ITEMS_PER_PAGE;i++) {
					CURRENT_SECTION.currentGameNode=CURRENT_SECTION.currentGameNode->next;
				}
			} else {
				CURRENT_SECTION.currentPage=0;
				CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
			}
			CURRENT_SECTION.currentGameInPage=0;
		}
	}
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void rewindPage(struct Rom *rom) {
	if (rom==NULL||rom->name==NULL) {
		return;
	}
	if (CURRENT_SECTION.alphabeticalPaging) {
		char *currentGame = getFileNameOrAlias(rom);
		char *previousGame;
		int hitStart = 0;
		while(!(CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0)) {
			previousGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->prev->data);
			if(tolower(currentGame[0])==tolower(previousGame[0])) {
				if (CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0) {
					hitStart = 1;
					break;
				} else {
					scrollUp();
				}
				free(currentGame);
				free(previousGame);
				currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
			} else {
				break;
			}

		}
		if (!hitStart) {
			scrollUp();
		}
		hitStart=0;
		free(currentGame);
		currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
		while(!(CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0)) {
			previousGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->prev->data);
			if ( (tolower(currentGame[0])==tolower(previousGame[0])) ||
					(isdigit(currentGame[0])&&isdigit(previousGame[0]))) {
				if (CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0) {
					hitStart = 1;
					break;
				} else {
					scrollUp();
				}
				free(currentGame);
				free(previousGame);
				currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
			} else {
				break;
			}
		}
		free(currentGame);
	} else if (CURRENT_SECTION.currentPage > 0) {
		int tempCurrentPage = CURRENT_SECTION.currentPage;
		while (!(CURRENT_SECTION.currentPage==(tempCurrentPage-1)&&CURRENT_SECTION.currentGameInPage==0)) {
			scrollUp();
		}
	} else {
		while (!(CURRENT_SECTION.currentPage==CURRENT_SECTION.totalPages&&CURRENT_SECTION.currentGameInPage==0)) {
			scrollUp();
		}
	}
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void showOrHideFavorites() {
	if (favoritesSectionSelected) {
		favoritesSectionSelected=0;
		currentSectionNumber=returnTo;
		if (CURRENT_SECTION.systemLogoSurface == NULL) {
			CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
			logMessage("INFO","showOrHideFavorites - Loading system logo");
			resizeSectionSystemLogo(&CURRENT_SECTION);
		}
		if (CURRENT_SECTION.backgroundSurface == NULL) {
			logMessage("INFO","Loading system background");
			CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
			resizeSectionBackground(&CURRENT_SECTION);
			CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
			resizeSectionSystemPicture(&CURRENT_SECTION);
		}
		if (returnTo==0) {
			if (fullscreenMode) {
				currentlySectionSwitching=1;
				resetPicModeHideLogoTimer();
				currentlySectionSwitching=1;
				displayBackgroundPicture();
				logMessage("INFO","Displaying system logo");
				showConsole();
			}
			determineStartingScreen(menuSectionCounter);
		} else {
			if (fullscreenMode) {
				currentlySectionSwitching=1;
				resetPicModeHideLogoTimer();
				displayBackgroundPicture();
				logMessage("INFO","Displaying system logo");
				showConsole();
			}
			loadGameList(0);
		}
		hideFullScreenModeMenu();
		scrollToGame(CURRENT_SECTION.realCurrentGameNumber);
		return;
	}
	if(strlen(favorites[0].name)<1) {
		return;
	}
	favoritesSectionSelected=1;
	returnTo=currentSectionNumber;
	currentSectionNumber=favoritesSectionNumber;
	if (CURRENT_SECTION.systemLogoSurface == NULL) {
		CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
		logMessage("INFO","showOrHideFavorites - Loading system logo");
		resizeSectionSystemLogo(&CURRENT_SECTION);
	}
	if (CURRENT_SECTION.backgroundSurface == NULL) {
		logMessage("INFO","Loading system background");
		CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
		resizeSectionBackground(&CURRENT_SECTION);
		CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
		resizeSectionSystemPicture(&CURRENT_SECTION);
	}
	if (fullscreenMode) {
		resetPicModeHideLogoTimer();
		currentlySectionSwitching=1;
		displayBackgroundPicture();
		logMessage("INFO","Displaying system logo");
		showConsole();
		refreshScreen();
	}
	loadFavoritesSectionGameList();
}

void removeFavorite() {
	favoritesChanged=1;
	if (favoritesSize>0) {
		#if defined TARGET_RG350 || defined TARGET_RG350_BETA
		Shake_Play(device, effect_id1);
		#endif	
		for (int i=CURRENT_GAME_NUMBER;i<favoritesSize;i++) {
			strcpy(favorites[i].emulatorFolder,favorites[i+1].emulatorFolder);
			strcpy(favorites[i].section,favorites[i+1].section);
			strcpy(favorites[i].executable,favorites[i+1].executable);
			strcpy(favorites[i].filesDirectory,favorites[i+1].filesDirectory);
			strcpy(favorites[i].name,favorites[i+1].name);
			strcpy(favorites[i].alias,favorites[i+1].alias);
		}
		strcpy(favorites[favoritesSize-1].section,"\0");
		strcpy(favorites[favoritesSize-1].emulatorFolder,"\0");
		strcpy(favorites[favoritesSize-1].executable,"\0");
		strcpy(favorites[favoritesSize-1].filesDirectory,"\0");
		strcpy(favorites[favoritesSize-1].name,"\0");
		strcpy(favorites[favoritesSize-1].alias,"\0");
		favoritesSize--;
		if (CURRENT_GAME_NUMBER==favoritesSize) {
			FAVORITES_SECTION.realCurrentGameNumber--;
		}
		loadFavoritesSectionGameList();
		if(!isPicModeMenuHidden) {
			resetPicModeHideMenuTimer();
		}
	}
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void markAsFavorite(struct Rom *rom) {
	favoritesChanged=1;
	if (favoritesSize<FAVORITES_SIZE) {
		if (!doesFavoriteExist(rom->name)) {
			resetHideHeartTimer();
			#if defined TARGET_RG350 || defined TARGET_RG350_BETA
			Shake_Play(device, effect_id);
			msleep(200);
			Shake_Play(device, effect_id);
			#endif		
			if (CURRENT_SECTION.onlyFileNamesNoExtension) {
				strcpy(favorites[favoritesSize].name, getGameName(rom->name));
			} else {
				strcpy(favorites[favoritesSize].name, rom->name);
//				printf("%s\n", favorites[favoritesSize].name);
			}
			if(rom->alias!=NULL&&strlen(rom->alias)>2) {
				strcpy(favorites[favoritesSize].alias, rom->alias);
			} else {
				favorites[favoritesSize].alias[0]=' ';
			}
			strcpy(favorites[favoritesSize].section,CURRENT_SECTION.sectionName);
			strcpy(favorites[favoritesSize].emulatorFolder,CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.activeEmulatorDirectory]);
			strcpy(favorites[favoritesSize].executable,CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable]);
			strcpy(favorites[favoritesSize].filesDirectory,rom->directory);
			favoritesSize++;
			qsort(favorites, favoritesSize, sizeof(struct Favorite), compareFavorites);
		}
	}
}

int isSelectPressed() {
	return keys[BTN_SELECT];
}

void performGroupChoosingAction() {
	int existed = 0;
	if (keys[BTN_START]) {
		currentlyChoosing=3;
//		pthread_create(&clockThread, NULL, updateClock,NULL);
		return;
	}
	if (keys[BTN_UP]||keys[BTN_L1]) {
		if(activeGroup>0) {
			activeGroup--;
		} else {
			activeGroup=sectionGroupCounter-1;
		}
		return;
	}
	if (keys[BTN_DOWN]||keys[BTN_R1]) {
		if(activeGroup<sectionGroupCounter-1) {
			activeGroup++;
		} else {
			activeGroup=0;
		}
		return;
	}
	if (keys[BTN_A]) {
		int preFavs = favoritesSectionNumber;
		if (beforeTryingToSwitchGroup!=activeGroup) {
			for (int sectionCount=0;sectionCount<menuSectionCounter;sectionCount++) {
				if(sectionCount==currentSectionNumber) {
					sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][0]=1;
				} else {
					sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][0]=0;
				}
				sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][1]=menuSections[sectionCount].currentPage;
				sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][2]=menuSections[sectionCount].currentGameInPage;
				sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][3]=menuSections[sectionCount].realCurrentGameNumber;

				cleanListForSection(&menuSections[sectionCount]);
				if (menuSections[sectionCount].backgroundSurface!=NULL) {
					SDL_FreeSurface(menuSections[sectionCount].backgroundSurface);
					menuSections[sectionCount].backgroundSurface = NULL;
				}
				if (menuSections[sectionCount].systemLogoSurface!=NULL) {
					SDL_FreeSurface(menuSections[sectionCount].systemLogoSurface);
					menuSections[sectionCount].systemLogoSurface = NULL;
				}
				if (menuSections[sectionCount].systemPictureSurface!=NULL) {
					SDL_FreeSurface(menuSections[sectionCount].systemPictureSurface);
					menuSections[sectionCount].systemPictureSurface = NULL;
				}

				int execsNum = sizeof(menuSections[sectionCount].executables) / sizeof(menuSections[sectionCount].executables[0]);
				for (int j=0;j<execsNum-1;j++) {
					if (sectionCount!=favoritesSectionNumber) {
						if (menuSections[sectionCount].executables[j]!=NULL&&strlen(menuSections[sectionCount].executables[j])>0) {
							free(menuSections[sectionCount].executables[j]);
						}
						if (menuSections[sectionCount].emulatorDirectories[j]!=NULL&&strlen(menuSections[sectionCount].emulatorDirectories[j])>0) {
							free(menuSections[sectionCount].emulatorDirectories[j]);
						}
					}
				}
			}
			if (currentlyChoosing) {
				loadSections(sectionGroups[activeGroup].groupPath);
				currentlyChoosing=0;
				currentSectionNumber=0;
				for(int i=0;i<menuSectionCounter;i++) {
					menuSections[i].initialized=0;
					menuSections[i].currentPage=0;
					menuSections[i].realCurrentGameNumber=0;
					menuSections[i].currentGameInPage=0;
					menuSections[i].currentGameNode=CURRENT_SECTION.head;
				}
				for (int sectionCount=0;sectionCount<menuSectionCounter;sectionCount++) {
					if(sectionCount!=favoritesSectionNumber) {
						if (sectionGroupStates[activeGroup][sectionCount][0]==1) {
							currentSectionNumber=sectionCount;
							logMessage("INFO","performGroupChoosingAction - Loading system logo");
							CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
							resizeSectionSystemLogo(&CURRENT_SECTION);
							logMessage("INFO","Loading system background");
							CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
							resizeSectionBackground(&CURRENT_SECTION);
							CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
							resizeSectionSystemPicture(&CURRENT_SECTION);
							existed = 1;

						}
						menuSections[sectionCount].currentPage=sectionGroupStates[activeGroup][sectionCount][1];
						menuSections[sectionCount].currentGameInPage=sectionGroupStates[activeGroup][sectionCount][2];
						menuSections[sectionCount].realCurrentGameNumber=sectionGroupStates[activeGroup][sectionCount][3];
					} else {
						menuSections[sectionCount].currentPage=sectionGroupStates[beforeTryingToSwitchGroup][preFavs][1];
						menuSections[sectionCount].currentGameInPage=sectionGroupStates[beforeTryingToSwitchGroup][preFavs][2];
						menuSections[sectionCount].realCurrentGameNumber=sectionGroupStates[beforeTryingToSwitchGroup][preFavs][3];
					}
				}
				loadGameList(0);
			}
			if (!existed) {
				logMessage("INFO","performGroupChoosingAction !existed - Loading system logo");
				CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
				resizeSectionSystemLogo(&CURRENT_SECTION);
				logMessage("INFO","Loading system background");
				CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
				resizeSectionBackground(&CURRENT_SECTION);
				CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
				resizeSectionSystemPicture(&CURRENT_SECTION);
			}
		} else {
			activeGroup = beforeTryingToSwitchGroup;
			currentlyChoosing=0;
		}
	}
}

void performSettingsChoosingAction() {
	if (keys[BTN_UP]) {
		if(chosenSetting>0) {
			chosenSetting--;
		} else {
			#if defined TARGET_RG300 || defined TARGET_RG350 || defined TARGET_RG350_BETA || defined TARGET_PC
			chosenSetting=9;
			#else
			chosenSetting=8;
			#endif
		}
	} else if (keys[BTN_DOWN]) {
		#if defined TARGET_RG300 || defined TARGET_RG350 || defined TARGET_RG350_BETA || defined TARGET_PC
		if(chosenSetting<9) {
		#else
		if(chosenSetting<8) {
		#endif
			chosenSetting++;
		} else {
			chosenSetting=0;
		}
	} else if (keys[BTN_LEFT]||keys[BTN_RIGHT]) {
		if (chosenSetting==TIDY_ROMS_OPTION) {
			stripGames=1+stripGames*-1;
		}
		#if defined TARGET_RG350 || defined TARGET_RG350_BETA || TARGET_PC
		else if (chosenSetting==USB_OPTION) {
			hdmiChanged=1+hdmiChanged*-1;
		}
		#endif
		else if (chosenSetting==SHUTDOWN_OPTION) {
			if (shutDownEnabled) {
				selectedShutDownOption=1+selectedShutDownOption*-1;
			} else {
				switch (selectedShutDownOption) {
				case 0:
					if(keys[BTN_RIGHT]) {
						selectedShutDownOption = 1;
					} else {
						selectedShutDownOption = 2;
					}
					break;
				case 1:
					if(keys[BTN_RIGHT]) {
						selectedShutDownOption = 2;
					} else {
						selectedShutDownOption = 0;
					}
					break;
				default:
					if(keys[BTN_RIGHT]) {
						selectedShutDownOption = 0;
					} else {
						selectedShutDownOption = 1;
					}
				}
			}
		} else if (chosenSetting==FULL_SCREEN_FOOTER_OPTION) {
			footerVisibleInFullscreenMode=1+footerVisibleInFullscreenMode*-1;
		} else if (chosenSetting==AUTO_HIDE_LOGOS_OPTION) {
			autoHideLogos=1+autoHideLogos*-1;
		} else if (chosenSetting==FULL_SCREEN_MENU_OPTION) {
			menuVisibleInFullscreenMode=1+menuVisibleInFullscreenMode*-1;
		} else if (chosenSetting==THEME_OPTION) {
			if (keys[BTN_LEFT]) {
				if (activeTheme>0) {
					activeTheme--;
				} else {
					activeTheme=themeCounter-1;
				}
			} else {
				if (activeTheme<themeCounter-1) {
					activeTheme++;
				} else {
					activeTheme=0;
				}
			}
			char *temp=malloc(8000);
			strcpy(temp,themes[activeTheme]);
			strcat(temp,"/theme.ini");
			loadTheme(temp);
			loadSectionGroups();
			free(temp);
			currentMode=3;
			MENU_ITEMS_PER_PAGE=itemsInCustom;
			FULLSCREEN_ITEMS_PER_PAGE=itemsInFullCustom;
		} else if (chosenSetting==ITEMS_PER_PAGE_OPTION) {
			if (keys[BTN_LEFT]) {
				int items = 0;
				while (items == 0) {
					if (currentMode==0) {
						items = itemsInCustom;
						MENU_ITEMS_PER_PAGE=itemsInCustom;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullCustom;
						currentMode=3;
					} else if (currentMode==3) {
						items = itemsInDrunkenMonkey;
						MENU_ITEMS_PER_PAGE=itemsInDrunkenMonkey;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullDrunkenMonkey;
						currentMode=2;
					} else if (currentMode==2) {
						items = itemsInTraditional;
						MENU_ITEMS_PER_PAGE=itemsInTraditional;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullTraditional;
						currentMode=1;
					} else {
						items = itemsInSimple;
						MENU_ITEMS_PER_PAGE=itemsInSimple;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullSimple;
						currentMode=0;
						break;
					}
				}
			}
			if (keys[BTN_RIGHT]) {
				int items = 0;
				while (items == 0) {
					if (currentMode==2) {
						items = itemsInCustom;
						MENU_ITEMS_PER_PAGE=itemsInCustom;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullCustom;
						currentMode=3;
					} else if (currentMode==1) {
						items = itemsInDrunkenMonkey;
						MENU_ITEMS_PER_PAGE=itemsInDrunkenMonkey;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullDrunkenMonkey;
						currentMode=2;
					} else if (currentMode==0) {
						items = itemsInTraditional;
						MENU_ITEMS_PER_PAGE=itemsInTraditional;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullTraditional;
						currentMode=1;
					} else {
						items = itemsInSimple;
						MENU_ITEMS_PER_PAGE=itemsInSimple;
						FULLSCREEN_ITEMS_PER_PAGE=itemsInFullSimple;
						currentMode=0;
						break;
					}
				}
			}
			char *temp=malloc(8000);
			strcpy(temp,themes[activeTheme]);
			strcat(temp,"/theme.ini");
			loadTheme(temp);
			loadSectionGroups();
			free(temp);
			switch (currentMode)
			{
		    	case 0:
		    		fontSize=baseFont;
		    		break;
			    case 1:
			    	fontSize=baseFont-2;
			    	break;
			    case 2:
			    	fontSize=baseFont-4;
			    	break;
			    default:
			    	fontSize=fontSizeCustom;
			}
			if(fullscreenMode==0) {
				ITEMS_PER_PAGE=MENU_ITEMS_PER_PAGE;
			} else {
				ITEMS_PER_PAGE=FULLSCREEN_ITEMS_PER_PAGE;
			}
		} else if (chosenSetting==SCREEN_TIMEOUT_OPTION) {
			if(!hdmiEnabled) {
				if (keys[BTN_LEFT]) {
					if (timeoutValue>0) {
						timeoutValue--;
					}
				} else {
					if (timeoutValue<60) {
						timeoutValue++;
					}
				}
			}
		} else if (chosenSetting==DEFAULT_OPTION) {
			char command [300];
			if (shutDownEnabled) {
				#ifdef TARGET_BITTBOY
				snprintf(command,sizeof(command),"rm /mnt/autoexec.sh;mv /mnt/autoexec.sh.bck /mnt/autoexec.sh");
				#endif
				#ifdef TARGET_RG300
				snprintf(command,sizeof(command),"rm /home/retrofw/autoexec.sh;mv /home/retrofw/autoexec.sh.bck /home/retrofw/autoexec.sh");
				#endif
				#if defined TARGET_RG350 || defined TARGET_NPG
				snprintf(command,sizeof(command),"rm /usr/local/sbin/frontend_start;mv /usr/local/sbin/frontend_start.bck /usr/local/sbin/frontend_start");
				#endif
				#if defined TARGET_RG350_BETA
				snprintf(command,sizeof(command),"rm /media/data/local/home/.autostart");
				#endif
			} else {
				#ifdef TARGET_BITTBOY
				snprintf(command,sizeof(command),"mv /mnt/autoexec.sh /mnt/autoexec.sh.bck;cp scripts/autoexec.sh /mnt");
				#endif
				#ifdef TARGET_RG300
				snprintf(command,sizeof(command),"mv /home/retrofw/autoexec.sh /home/retrofw/autoexec.sh.bck;cp scripts/autoexec.sh /home/retrofw");
				#endif
				#if defined TARGET_RG350 || defined TARGET_NPG
				snprintf(command,sizeof(command),"mv /usr/local/sbin/frontend_start /usr/local/sbin/frontend_start.bck;cp scripts/frontend_start /usr/local/sbin/");
				#endif
				#if defined TARGET_RG350_BETA
				snprintf(command,sizeof(command),"cp ./scripts/frontend_start /media/data/local/home/.autostart");
				#endif
			}
			int ret = system(command);
			if (ret==-1) {
				logMessage("ERROR","Error setting default");
			}
			if (selectedShutDownOption==2) {
				selectedShutDownOption = 0;
			}
			shutDownEnabled=1+shutDownEnabled*-1;
		}
	} else if (chosenSetting==SHUTDOWN_OPTION&&keys[BTN_A]) {
		running=0;
	}
	#if defined TARGET_RG300
	else if (chosenSetting==USB_OPTION&&keys[BTN_A]) {
		hotKeyPressed=0;
		int returnedValue = system("./scripts/usb_mode_on.sh>/home/retrofw/apps/test.txt");
		if (returnedValue==0) {
			isUSBMode = 1;
		} else {
			generateError("USB MODE  NOT AVAILABLE",0);
		}
		currentlyChoosing=0;
	}
	#endif
	else if (keys[BTN_START]) {
//		pthread_cancel(clockThread);
		#if defined TARGET_RG350 || defined TARGET_RG350_BETA
		if (hdmiChanged!=hdmiEnabled) {
			FILE *fp = fopen("/sys/class/hdmi/hdmi","w");
			if (fp!=NULL) {
				fprintf(fp, "%d", hdmiChanged);
				fclose(fp);
			}
			saveLastState();
			saveFavorites();
			clearTimer();
			clearPicModeHideLogoTimer();
			clearPicModeHideMenuTimer();
			freeResources();
			execlp("./simplemenu","invoker",NULL);
		}
		#endif
		currentlyChoosing=0;
//		char *temp=malloc(8000);
//		strcpy(temp,themes[activeTheme]);
//		strcat(temp,"/theme.ini");
//		loadTheme(temp);
//		free(temp);
		if(fullscreenMode==0) {
			ITEMS_PER_PAGE=MENU_ITEMS_PER_PAGE;
		} else {
			ITEMS_PER_PAGE=FULLSCREEN_ITEMS_PER_PAGE;
		}
		if (currentSectionNumber!=favoritesSectionNumber) {
			loadGameList(2);
		} else {
			loadFavoritesSectionGameList(1);
		}
	}
}



void performChoosingAction() {
	if (keys[BTN_UP]) {
		if(CURRENT_SECTION.activeExecutable>0) {
			CURRENT_SECTION.activeExecutable--;
			CURRENT_SECTION.activeEmulatorDirectory--;
		} else {
			CURRENT_SECTION.activeExecutable=sizeof(CURRENT_SECTION.executables)/sizeof(CURRENT_SECTION.executables[0])-1;
			while (CURRENT_SECTION.activeExecutable>0&&CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable]==NULL) {
				CURRENT_SECTION.activeExecutable--;
				CURRENT_SECTION.activeEmulatorDirectory--;
			}
		}
		return;
	}
	if (keys[BTN_DOWN]) {
		if(CURRENT_SECTION.executables[CURRENT_SECTION.activeExecutable+1]!=NULL) {
			CURRENT_SECTION.activeExecutable++;
			CURRENT_SECTION.activeEmulatorDirectory++;
		} else {
			CURRENT_SECTION.activeExecutable=0;
			CURRENT_SECTION.activeEmulatorDirectory=0;
		}
		return;
	}
	if (keys[BTN_A]) {
		if (currentlyChoosing) {
			currentlyChoosing=0;
			return;
		}
	}
}

void callDeleteGame(struct Rom *rom) {
	if (!favoritesSectionSelected) {
		deleteGame(rom);
		loadGameList(1);
		while(CURRENT_SECTION.hidden) {
			rewindSection(0);
			loadGameList(0);
			hideFullScreenModeMenu();
		}
		if(CURRENT_SECTION.currentGameInPage==countGamesInPage()) {
			scrollUp();
		}
		setupDecorations();
	} else {
		generateError("YOU CAN'T DELETE GAMES-WHILE IN FAVORITES",0);
	}
}
