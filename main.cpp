#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <thread>
#include <chrono>

#include <ncurses.h>

#define BACKGROUND_COLOR 0
#define SNAKE_COLOR 1
#define BORDER_COLOR 2
#define NORMAL_POINT_COLOR 3
#define SPECIAL_POINT_COLOR 4


const int gamefieldWidth = 40;
const int gamefieldHeight = 14;
const int specialPointMaxTime = 30;
const int specialPointMinTime = 6;
const int specialPointMaxLifeTime = 20;

const char gameborderChar = '#';

struct snakepart{
	int x, y;
	snakepart *next;

	void addSnakepart();
	void updateSnakepart(int, int);
};

void snakepart::addSnakepart(){
	if(next != NULL){
		next->addSnakepart();
	}else{
		snakepart *newPart = new snakepart;
		newPart->x = x;
		newPart->y = y;
		newPart->next = NULL;
		next = newPart;
	}	
}

void snakepart::updateSnakepart(int prevX, int prevY){
	if(next != NULL){
		next->updateSnakepart(x, y);
	}
	x = prevX;
	y = prevY;
}

struct point{
	unsigned int x, y;

	void setPointCoordinatesRandomly();
};

void point::setPointCoordinatesRandomly(){
	x = rand() % gamefieldWidth;
	y = rand() % gamefieldHeight;
}

struct specialpoint : point{
	bool active;
	std::chrono::high_resolution_clock::time_point startTime;
	int lifeTime = 12;

	bool isAlive();
};

bool specialpoint::isAlive(){
	auto currentTimer = std::chrono::high_resolution_clock::now();

	if(std::chrono::duration_cast<std::chrono::duration<float>>(currentTimer - startTime).count() > lifeTime)
		return false;
	return true;
}

char gamefield[gamefieldHeight][gamefieldWidth];

int headDirection = 1;

snakepart head;

long long frameNr = 0;	//TODO: Still used?

point normalPoint;
specialpoint specialPoint;

int specialpointSpawnTime;
std::chrono::high_resolution_clock::time_point startTimer;

void resetSpecialPoint(){
	specialpointSpawnTime = rand() % specialPointMaxTime + specialPointMinTime;
	startTimer = std::chrono::high_resolution_clock::now();
	specialPoint.active = false;
	specialPoint.setPointCoordinatesRandomly();
	specialPoint.lifeTime = rand() % specialPointMaxLifeTime;
}

void initializeGame(){
	srand(time(NULL));

	int x,y;
	x = round(gamefieldWidth / 2);
	y = round(gamefieldHeight / 2);

	head.x = x;
	head.y = y;
	head.next = NULL;

	head.addSnakepart();
	head.addSnakepart();
	head.addSnakepart();

	normalPoint.setPointCoordinatesRandomly();
	resetSpecialPoint();
	
    init_pair(BACKGROUND_COLOR, COLOR_BLACK, COLOR_BLACK);
    init_pair(SNAKE_COLOR, COLOR_BLACK, COLOR_GREEN);
    init_pair(BORDER_COLOR, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(NORMAL_POINT_COLOR, COLOR_CYAN, COLOR_CYAN);
    init_pair(SPECIAL_POINT_COLOR, COLOR_BLACK, COLOR_YELLOW);
}

void clearGamefield(){
  	for(int y = 0; y < gamefieldHeight; ++y){
    	for(int x = 0; x < gamefieldWidth; ++x){
	  	  	if(y == 0 || y == (gamefieldHeight - 1) || x == 0 || x == (gamefieldWidth - 1)){
				gamefield[y][x] = gameborderChar;
	      	}else{
	      		gamefield[y][x] = ' ';
	      	}
	   	}
  	}

  	snakepart *currentSnakepart = &head;
  	while(currentSnakepart != NULL){
  		if(currentSnakepart->x < 0 || currentSnakepart->x >= gamefieldHeight){
  			//TODO: Trap + Exception
  		}

  		if(currentSnakepart->y < 0 || currentSnakepart-> y >= gamefieldHeight){
  			//TODO: Trap + Exception
  		}

  		gamefield[currentSnakepart->y][currentSnakepart->x] = '+';
  		
  		currentSnakepart = currentSnakepart->next;
  	}
  	gamefield[normalPoint.y][normalPoint.x] = 'a';

  	if(specialPoint.active){
  		if(!specialPoint.isAlive()){
  			resetSpecialPoint();
  		}else{
  			gamefield[specialPoint.y][specialPoint.x] = 'A';
  		}
  	}else{
  		auto currentTimer = std::chrono::high_resolution_clock::now();

	  	if(std::chrono::duration_cast<std::chrono::duration<float>>(currentTimer - startTimer).count() >= specialpointSpawnTime){
	  		specialPoint.active = true;
			specialPoint.startTime = std::chrono::high_resolution_clock::now();
	  	}
  	}
}

void userInput(){
	while(1){
		switch(getch()){
			case 'a':
				headDirection = 1;
				break;
			case 'd':
				headDirection = 2;
				break;
			case 'w':
				headDirection = 3;
				break;
			case 's':
				headDirection = 4;
				break;
		}
	}
}

bool snakeIsCollidingWithItself(){
	if(head.next == NULL)
		return false;

	snakepart *currentSnakepart = &head;
	currentSnakepart = currentSnakepart->next;

  	while(currentSnakepart != NULL){
  		if(currentSnakepart->x == head.x && currentSnakepart->y == head.y)
  			return true;

  		currentSnakepart = currentSnakepart->next;
  	}
  	return false;
}

void updateSnake(){
	head.next->updateSnakepart(head.x, head.y);

	switch(headDirection){
		case 1:
			head.x -= 1;
			if(head.x < 0)
				head.x = (gamefieldWidth - 1);
		break;
		case 2:
			head.x += 1;
			if(head.x >= gamefieldWidth)
				head.x = 0;
		break;
		case 3:
			head.y -= 1;
			if(head.y < 0)
				head.y = (gamefieldHeight - 1);
		break;
		case 4:
			head.y += 1;
			if(head.y >= gamefieldHeight)
				head.y = 0;
		break;
	}

	if(head.x == normalPoint.x && head.y == normalPoint.y){
		normalPoint.setPointCoordinatesRandomly();
		head.addSnakepart();
	}

	if(head.x == specialPoint.x && head.y == specialPoint.y){
		resetSpecialPoint();
		head.addSnakepart();
		head.addSnakepart();
	}

	if(snakeIsCollidingWithItself()){
		head.next = NULL;	//TODO: Set Game Over
	}
}

void setPixel(char c, int color){
	attron(COLOR_PAIR(color));
	addch(c);
	attroff(COLOR_PAIR(color));
}

void drawScene(){
  	clearGamefield();
	for(int y = 0; y < (gamefieldHeight); ++y){
		for(int x = 0; x < (gamefieldWidth); ++x){
			int color; 
			char pixel = gamefield[y][x];

			switch(pixel){
				case '+':
					color = SNAKE_COLOR;
					break;
				case '#':
					color = BORDER_COLOR;
					break;
				case 'a':
					color = NORMAL_POINT_COLOR;
					break;
				case 'A':
					color = SPECIAL_POINT_COLOR;
					break;
				default:
					color = BACKGROUND_COLOR;
					break;
			}
   	 		setPixel(gamefield[y][x], color);
   	 	} 
	  	setPixel('\n', 0);
	}
}

void debugging(){
	std::string playerPosition = "Player X=" + std::to_string(head.x) + "  Y=" + std::to_string(head.y)+"\n";
	std::string pointPosition = "Point X=" + std::to_string(normalPoint.x) + "  Y=" + std::to_string(normalPoint.y)+"\n";
	std::string specialPointSpawn;
	if(specialPoint.active){
		specialPointSpawn = "SpecialPoint Spawned!";
	}else{
		specialPointSpawn = "SpecialPoint SpawnTime=" + std::to_string(specialpointSpawnTime);
	} 

	printw(playerPosition.c_str());
	printw(pointPosition.c_str());
	printw(specialPointSpawn.c_str());
}

int main()
{

	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set(false);

	initializeGame();
	std::thread inputThread(userInput);
  	
  	while(1){
  		clear();
   		updateSnake();
  		drawScene();
  		debugging();
 		refresh();
   		std::this_thread::sleep_for(std::chrono::milliseconds(600));
  	}

  	endwin();

  	return 0;
}
