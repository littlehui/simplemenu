#include <definitions.h>
#include <globals.h>
#include <graphics.h>
#include <logic.h>
#include <screen.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL_events.h>

void performAction(SDL_Event event) {
	if (keys[BTN_SELECT] && keys[BTN_START]) {
		running=0;
		return;
	}
	if (keys[BTN_A] && keys[BTN_START]) {
		freeResources();
		exit(0);
	}
	if(keys[BTN_TB]) {
		if(currentCPU==NO_OC) {
			setCPU(MED_OC);
			drawHeader();
		} else if (currentCPU==MED_OC) {
			setCPU(MAX_OC);
			drawHeader();
		} else {
			setCPU(NO_OC);
			drawHeader();
		}
		return;
	}
	if(keys[BTN_TA]) {
		if(strcmp(menuSections[currentSection+1].sectionName,"END")!=0) {
			currentSection++;
			setupDecorations();
			totalPages=0;
			loadGameList();
			currentGame=0;
			currentPage=0;
			return;
		}
	}
	if(keys[BTN_B]) {
		if(currentSection>0) {
			currentSection--;
			setupDecorations();
			totalPages=0;
			loadGameList();
			currentGame=0;
			currentPage=0;
			return;
		}
	}
	if (keys[BTN_A]) {
		char fileToBeExecutedwithFullPath[200];
		if (gameList[currentPage][currentGame]!=NULL) {
			strcpy(fileToBeExecutedwithFullPath,menuSections[currentSection].filesDirectory);
			strcat(fileToBeExecutedwithFullPath,gameList[currentPage][currentGame]);
			if (currentSection>0) {
				executeCommand(menuSections[currentSection].executable,fileToBeExecutedwithFullPath);
			} else {
				executeFavorite(fileToBeExecutedwithFullPath);
			}
		}
		return;
	}
	if (keys[BTN_DOWN]) {
		if(currentGame == gamesInPage-1) {
			if (currentPage < totalPages) {
				currentGame=0;
				currentPage++;
				return;
			}
		}
		if (currentGame < gamesInPage-1) {
			currentGame++;
			return;
		}
	}
	if(keys[BTN_UP]) {
		if(currentGame == 0) {
			if (currentPage>0) {
				currentGame=gamesInPage-1;
				currentPage--;
				return;
			}
		}
		if (currentGame > 0) {
			currentGame--;
			return;
		}
	}
	if(keys[BTN_RIGHT]) {
		if (currentPage < totalPages) {
			currentGame=0;
			currentPage++;
		}
	}
	if(keys[BTN_LEFT]) {
		if (currentPage > 0) {
			currentGame=0;
			currentPage--;
		}
		return;
	}

}