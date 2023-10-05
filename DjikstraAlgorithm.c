#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <limits.h>
#include <stdarg.h>
#include <dirent.h>
// Name: Anthony Kevin Oktavius
// University Name: Binus University
// Semester: 2
enum Direction {up, down, left, right};
int begginingPositionPlayer = 0;
int begginingPositionOpponent = 0;
int numberOfPaths = 0;
int numberOfColumnPaths = 0;
int isGameOver = 0;
char mapPath[600] = {};
struct Path{
	int indexNumber;
	char value;
	int movementCost;
	int isEnemyPath:2;
	struct Path* lastPath;
	struct Path* up;
	struct Path* down;
	struct Path* left;
	struct Path* right;
} typedef Path;

struct Player{
	int score;
	char name[30];
	short int currentMovePoints;
	short int isHit;
	struct Path* currentPath;
} typedef Player;

struct Opponent{
	short int currentMovePoints;
	struct Path* currentPath;
	struct Path* nextPath;
} typedef Opponent;

struct Node{
	int totalCost;
	struct Path* pathInfo;
} typedef Node;

// Priority Queue Using Heap as insertion and deletion will take O(log n)
struct PriorityQueue{
	int capacity;
	struct Node* nodeCollections;
} typedef PriorityQueue;

int moveLeftNode(int position){
	return (2 * position) + 1;
}

int moveRightNode(int position){
	return (2 * position) + 2;
}

int moveParentNode(int position){
	return (position - 1) / 2;
}
PriorityQueue priorityQueue = {};
Opponent* opponent = NULL;
Player* player = NULL;
Path* topLeftPath = NULL;
Path* tempHeadPath = NULL;
Path* tempTailPath = NULL;
Path* currPath = NULL;
Path* previousPath = NULL;

Node oneNode(Path* path, int distance){
	static struct Node tempNode = {};
	tempNode.pathInfo = path;
	tempNode.totalCost = path->movementCost + distance;
	return tempNode;
}
void swapValue(Node node[], int indexOne, int indexTwo){
	int tempCost = 0;
	Path* tempPath = NULL;
	tempCost = node[indexOne].totalCost;
	tempPath = node[indexOne].pathInfo;
	node[indexOne].totalCost = node[indexTwo].totalCost;
	node[indexOne].pathInfo = node[indexTwo].pathInfo;
	node[indexTwo].totalCost = tempCost;
	node[indexTwo].pathInfo = tempPath;
}
void printLine(char message[]){
	printf("%s\n", message);
}
void traverseUp(int i){
	int index = i;
	while (index != 0 && priorityQueue.nodeCollections[moveParentNode(index)].totalCost > priorityQueue.nodeCollections[index].totalCost){
		swapValue(priorityQueue.nodeCollections, moveParentNode(index), index);
		index = moveParentNode(index);
	}
}
void fixHeapMin(int i, int maxIndex){
	int index = i;
	int valueLeftNode = 0;
	int valueRightNode = 0;
	do{
		// Traverse Down if there's node which is smaller than the parent and if there's any
		if (moveLeftNode(index) < maxIndex && priorityQueue.nodeCollections[index].totalCost > priorityQueue.nodeCollections[moveLeftNode(index)].totalCost){
			valueLeftNode = priorityQueue.nodeCollections[moveLeftNode(index)].totalCost;
		}
		else{
			valueLeftNode = INT_MAX;
		}
		if (moveRightNode(index) < maxIndex && priorityQueue.nodeCollections[index].totalCost > priorityQueue.nodeCollections[moveRightNode(index)].totalCost){
			valueRightNode = priorityQueue.nodeCollections[moveRightNode(index)].totalCost;
		}
		else{
			valueRightNode = INT_MAX;
		}
		if (valueLeftNode != INT_MAX || valueRightNode != INT_MAX){
			if (valueLeftNode <= valueRightNode){
				swapValue(priorityQueue.nodeCollections, index, moveLeftNode(index));
				index = moveLeftNode(index);
			}
			else if (valueRightNode < valueLeftNode){
				swapValue(priorityQueue.nodeCollections, index, moveRightNode(index));
				index = moveRightNode(index);
			}
		}
	}while(valueLeftNode != INT_MAX || valueRightNode != INT_MAX);
}
// Insert data for min priority queue
Node insertDataMin(Node newNode){
	priorityQueue.nodeCollections = (Node*)realloc(priorityQueue.nodeCollections, (priorityQueue.capacity + 1) * sizeof(Node));
	priorityQueue.nodeCollections[priorityQueue.capacity] = newNode;
	traverseUp(priorityQueue.capacity);
	priorityQueue.capacity += 1;
	return priorityQueue.nodeCollections[priorityQueue.capacity - 1];
}
// Delete data for min priority queue
Path* extractMin(){
	Path* path = NULL;
	path = priorityQueue.nodeCollections[0].pathInfo;
	if (priorityQueue.capacity > 1){
		swapValue(priorityQueue.nodeCollections, 0, priorityQueue.capacity - 1);
		fixHeapMin(0, priorityQueue.capacity - 1);
	}
	priorityQueue.nodeCollections = (Node*)realloc(priorityQueue.nodeCollections, (priorityQueue.capacity - 1) * sizeof(Node));
	priorityQueue.capacity -= 1;
	return path;
}
// I love designing! That's why I provide some colors to color the cmd!
void boldCyan(){
	printf("\033[1;36m");
}
void magentaColor(){
	printf("\033[1;35m");
}
void yellowColor(){
	printf("\033[01;33m");
}
void greenColor(){
	printf("\033[1;32m");
}
void resetColor(){
	printf("\033[0m");
}
void redColor(){
	printf("\033[1;31m");
}
// Debugging option to see the values inside min heap
void printContents(){
	int i = 0;
	greenColor();
	for (i = 0; i < priorityQueue.capacity; i++){
		printf("%i ", priorityQueue.nodeCollections[i].totalCost);
	}
	resetColor();
	printf("\n");
}
// Method to get the data from .txt file and some tags to produce desired color
void printFromFile(char path[]){
	char tempText[600] = {};
	FILE* file = fopen(path, "r");
	while (fscanf(file, "%[^\n]\n", tempText) != EOF){
		if (strcmp("<red>", tempText) == 0){
			redColor();
		}
		else if (strcmp ("<blue>", tempText) == 0){
			magentaColor();
		}
		else if (strcmp("<yellow>", tempText) == 0){
			yellowColor();
		}
		else if (strcmp("<green>", tempText) == 0){
			greenColor();
		}
		else if (strcmp("<reset>", tempText) == 0){
			resetColor();
		}
		else{
			printf("%s\n", tempText);
		}
	}
	fclose(file);
}
// Easy way to prevent overflow values and return the value
int giveRangedValues(int min, int max, char message[]){
	static int temp = 0;
	do{
		yellowColor();
		printf("%s", message);
		scanf("%i", &temp);
		getchar();
		resetColor();
	}while(temp < min || temp > max);
	return temp;
}
// Make a new path
Path* onePath(char value, int pathNumber){
	currPath = (Path*) malloc(sizeof(Path));
	if (value == '^'){
		currPath->movementCost = 50;
	}
	else if (value == '%'){
		currPath->movementCost = 25;
	}
	else if (value == '#'){
		currPath->movementCost = 10;
	}
	else if (value == ' '){
		currPath->movementCost = 1;
	}
	else{
		currPath->movementCost = 0;
	}
	currPath->indexNumber = pathNumber;
	currPath->value = value;
	currPath->right = NULL;
	currPath->up = NULL;
	currPath->down = NULL;
	currPath->left = NULL;
	return currPath;
}
// Load map data from a .txt file
void getMapDataFromFile(char path[]){
	FILE* file = fopen(path, "r");
	int i = 0;
	int wasNewline = 0;
	numberOfColumnPaths = 0;
	Path* newNode = NULL;
	while(fscanf(file, "%c", &mapPath[i]) != EOF){
		//printf("Reached here!\n");
		newNode = onePath(mapPath[i], i);
		if (topLeftPath == NULL){
			topLeftPath = newNode;
			tempHeadPath = topLeftPath;
			tempTailPath = tempHeadPath;
		}
		else if (mapPath[i] != '\n'){
			if (wasNewline == 1){
				newNode->up = previousPath;
				previousPath->down = newNode;
				tempHeadPath = newNode;
				tempTailPath = tempHeadPath;
				previousPath = previousPath->right;
				wasNewline = 0;
			}
			else if (previousPath == NULL){
				newNode->left = tempTailPath;
				tempTailPath->right = newNode;
				tempTailPath = tempTailPath->right;
			}
			else{
				newNode->up = previousPath;
				previousPath->down = newNode;
				tempTailPath->right = newNode;
				newNode->left = tempTailPath;
				tempTailPath = tempTailPath->right;
				previousPath = previousPath->right;
			}
		}
		else{
			if (tempHeadPath->up == NULL){
				//printf("!!!");
				numberOfColumnPaths = i;
			}
			wasNewline = 1;
			previousPath = tempHeadPath;
			tempHeadPath = tempHeadPath->down;
		}
		i+=1;
	}
	numberOfPaths=i-1;
	fclose(file);
}
// A useful method for freeing memory so it will prevent memory error
void freeMemory(){
	tempHeadPath = topLeftPath;
	while (tempHeadPath != NULL){
		tempTailPath = tempHeadPath;
		tempHeadPath = tempHeadPath->down;
		if (tempHeadPath != NULL){
			tempHeadPath->up = NULL;	
		}
		while (tempTailPath->right != NULL){
			tempTailPath = tempTailPath->right;
			tempTailPath->up = NULL;
			free(tempTailPath->left);
			tempTailPath->left = NULL;
		}
		tempTailPath->up = NULL;
		tempTailPath->left = NULL;
		free(tempTailPath);
	}
	free(tempHeadPath);
	topLeftPath = NULL;
	tempHeadPath = NULL;
	tempTailPath = NULL;
	currPath = NULL;
	player->currentPath = NULL;
	opponent->currentPath = NULL;
}
// Method to print the map from mapPath array that was scanned from .txt file
void printMap2D(){
	int i = 0;
	tempHeadPath = topLeftPath;
	tempTailPath = tempHeadPath;
	while (tempHeadPath != NULL){
		tempTailPath = tempHeadPath;
		while (tempTailPath != NULL){
			if (tempTailPath == player->currentPath){
				mapPath[i++] = 'P';
			}
			else if (tempTailPath == opponent->currentPath){
				mapPath[i++] = '*';
			}
			else{
				mapPath[i++] = tempTailPath->value;
			}
			tempTailPath = tempTailPath->right;
		}
		mapPath[i++] = '\n';
		tempHeadPath = tempHeadPath->down;
	}
	printf("%s\n", mapPath);
}
// Method to give delays to cmd
int myTimer(int duration){
	static DWORD finish_time = 0;
	if (GetTickCount() < finish_time){
		return -1;		
	}
	finish_time = GetTickCount() + duration;
	return 1;
}
// method to check whether a particular path contains wall or NULL value
int determineIfWall(Path* path){
	if (path == NULL || path->value == '-' || path->value == '|' || path->value == '='){
		return 1;
	}
	return -1;
}
// Method to insert data into the path pointer
void insertProcess(short int isVisited[], Path* path, Path* sourcePath, int distanceCounter, int isDebug){
	if (isDebug == 1){
		path->value = '0';
	}
	path->lastPath = sourcePath;
	isVisited[path->indexNumber] = 1;
	insertDataMin(oneNode(path, distanceCounter));
}
// These three methods are used to generate path for the opponent, one for debugging, the other is for the game
void generatePath(Path* sourcePath){
	Path* path = sourcePath;
	while (path != NULL){
		path->value = '@';
		path = path->lastPath;
	}
}
void generateOpponentPath(Path* sourcePath){
	Path* path = sourcePath;
	while (path != NULL){
		path->isEnemyPath = 1;
		path = path->lastPath;
	}
}
void pathGenerateMode(int isDebug, Path* path){
	if (isDebug == 1){
		generatePath(path);
	}
	else{
		generateOpponentPath(path);
	}
}
// This is where the logic for opponent lays to detect player's position
void doDjikstraSearchMap(int isDebug){
	short int isVisited[600] = {};
	// Similar to BFS Technique, but this time we use priority queue
	int distanceCounter = 1;
	Path* currentScanPath = NULL;
	insertProcess(isVisited, opponent->currentPath, NULL, distanceCounter, isDebug);
	while (priorityQueue.capacity > 0){
		currentScanPath = extractMin();
		if (determineIfWall(currentScanPath->up) == -1 && isVisited[currentScanPath->up->indexNumber] != 1){
			insertProcess(isVisited, currentScanPath->up, currentScanPath, distanceCounter, isDebug);
			if (currentScanPath->up == player->currentPath){
				pathGenerateMode(isDebug, currentScanPath->up);
				break;
			}
		}
		if (determineIfWall(currentScanPath->down) == -1 && isVisited[currentScanPath->down->indexNumber] != 1){
			insertProcess(isVisited, currentScanPath->down, currentScanPath, distanceCounter, isDebug);
			if (currentScanPath->down == player->currentPath){
				pathGenerateMode(isDebug, currentScanPath->down);
				break;
			}
		}
		if (determineIfWall(currentScanPath->left) == -1 && isVisited[currentScanPath->left->indexNumber] != 1){
			insertProcess(isVisited, currentScanPath->left, currentScanPath, distanceCounter, isDebug);
			if (currentScanPath->left == player->currentPath){
				pathGenerateMode(isDebug, currentScanPath->left);
				break;
			}
		}
		if (determineIfWall(currentScanPath->right) == -1 && isVisited[currentScanPath->right->indexNumber] != 1){
			insertProcess(isVisited, currentScanPath->right, currentScanPath, distanceCounter, isDebug);
			if (currentScanPath->right == player->currentPath){
				pathGenerateMode(isDebug, currentScanPath->right);
				break;
			}
		}
		distanceCounter+=1;
		if (isDebug == 1){
			printContents();
			printMap2D();
			while (myTimer(300) == -1){
				
			}
		}
	}
	while (priorityQueue.capacity > 0){
		currentScanPath = extractMin();
	}
}
// Generate random position for the enemy and the player
Path* findPosition(unsigned int randomizer){
	int currentValue = 0;
	int pathNumber = 0;
	Path* currPath = topLeftPath;
	do{
		randomizer -= 1357;
		pathNumber = randomizer % numberOfPaths;
	}while(mapPath[pathNumber] == '=' || mapPath[pathNumber] == '|' || mapPath[pathNumber] == '-'|| mapPath[pathNumber] == '\n');
	while (currentValue + numberOfColumnPaths < pathNumber){
		//printf("Current value: %u %u", currentValue, pathNumber);
	//printf("Reached here %c step 1!\n", currPath->value);
		currPath = currPath->down;
		currentValue += (numberOfColumnPaths + 1);
	}
	while (currentValue < pathNumber){
		//printf("Reached here %i %i step 2!\n", currentValue, pathNumber);
		currPath = currPath->right;
		currentValue += 1;
	}
	return currPath;
}

void giveFullPath(char source[], int numberOfArgs, ...){
	va_list args;
	va_start(args, numberOfArgs);
	int i = 0;
	for (i; i < numberOfArgs; i++){
		strcat(source, va_arg(args, char*));
	}
	va_end(args);
}

int getScoreData(char fullPath[], int lineNumber){
	int score = 0;
	FILE* file = fopen(fullPath, "r");
	if (file != NULL){
		char tempInfo[100] = {};
		while (lineNumber > 0){
			fscanf(file, "%[^\n]\n", tempInfo);
			lineNumber-=1;
		}
		fscanf(file, "%i", &score);
		printf("%i", score);
		fclose(file);
		return score;
	}
	return -1;
}

// Saves the data by either overwrite the whole data if the user doesn't exist or make a new file that's based on player's name
void printSeparateFiles(char fullPath[], char format[], int numberOfArgs,...){
	FILE* file = fopen(fullPath, "w");
	printf("Reached here!\n");
	va_list args;
	va_start(args, numberOfArgs);
	int i = 0;
	for (i = 0; i < numberOfArgs; i++){
		if (format[i] == 's'){
			fprintf(file, "%s\n", va_arg(args, char*));
		}
		else if (format[i] == 'i'){
			fprintf(file, "%i\n", va_arg(args, int));
		}
	}
	fclose(file);
}
void printAllFilesFromDirectory(char directory[]){
	DIR* dir = opendir(directory);
	struct dirent* dirInfo;
	if (dir){
		dirInfo = readdir(dir);
		while (dirInfo != NULL){
			if (strcmp(dirInfo->d_name, ".") != 0 && strcmp(dirInfo->d_name, "..") != 0){
				char fullPath[150] = {};
				printLine("==================================");
				giveFullPath(fullPath, 2, directory, dirInfo->d_name);
				printFromFile(fullPath);
				printLine("==================================");
			}
			dirInfo = readdir(dir);
		}
		closedir(dir);
	}
}
void printPlayerStatus(int score){
	printf("============================================\n");
	printf("Player Name: %s\n", player->name);
	printf("Score: %i\n", score);
	printf("============================================\n\n");
}
Path* moveToAnotherPath(Path* path){
	opponent->currentMovePoints = 0;
	return path;
}
void moveOpponent(){
	if (opponent->currentMovePoints < (opponent->currentPath->movementCost / 2)){
		opponent->currentMovePoints += 5;
		return;
	}
	opponent->currentPath->isEnemyPath = 0;
	if (opponent->currentPath->up->isEnemyPath == 1){
		opponent->currentPath = moveToAnotherPath(opponent->currentPath->up);
	}
	else if (opponent->currentPath->right->isEnemyPath == 1){
		opponent->currentPath = moveToAnotherPath(opponent->currentPath->right);
	}
	else if (opponent->currentPath->down->isEnemyPath == 1){
		opponent->currentPath = moveToAnotherPath(opponent->currentPath->down);
	}
	else if (opponent->currentPath->left->isEnemyPath == 1){
		opponent->currentPath = moveToAnotherPath(opponent->currentPath->left);
	}
	else{
		doDjikstraSearchMap(0);
		return moveOpponent();
	}
}
Path* stepMovementPlayer(Path* path){
	if (path == opponent->currentPath){
		isGameOver = 1;
		return NULL;
	}
	else{
		return path;
	}
}
void movePlayer(char key){
	printf("%i\n", player->currentMovePoints);
	player->currentMovePoints += 5;
	if (player->currentPath == opponent->currentPath){
		isGameOver = 1;
	}
	else if (player->currentMovePoints >= player->currentPath->movementCost){
		player->score += player->currentPath->movementCost;
		if ((key == 'A' || key == 'a') && determineIfWall(player->currentPath->left) == -1){
			player->currentPath = stepMovementPlayer(player->currentPath->left);
		}
		else if ((key == 'S' || key == 's') && determineIfWall(player->currentPath->down) == -1){
			player->currentPath = stepMovementPlayer(player->currentPath->down);
		}
		else if ((key == 'D' || key == 'd') && determineIfWall(player->currentPath->right) == -1){
			player->currentPath = stepMovementPlayer(player->currentPath->right);
		}
		else if ((key == 'W' || key == 'w') && determineIfWall(player->currentPath->up) == -1){
			player->currentPath = stepMovementPlayer(player->currentPath->up);
		}
		player->currentMovePoints = 0;
	}
}
void MainGame(){
	player->score = 0;
	printf("Please enter your name: ");
	scanf("%[^\n]", player->name);
	getchar();
	srand(time(0));
	char key = 'D';
	getMapDataFromFile("MapDjikstra.txt");
	system("cls");
	int randomizer = rand();
	player->currentPath = findPosition(randomizer);
	randomizer -= 1333;
	opponent->currentPath = findPosition(randomizer);
	printFromFile("Ready.txt");
	while (myTimer(2000) == -1){
		
	}
	while (myTimer(600) == -1){
		
	}
	do{
		system("cls");
		int canMove = 0;
		printPlayerStatus(player->score);
		printMap2D();
		while (myTimer(250) == -1){
			if (kbhit()){
				key = getch();
			}
		}
		movePlayer(key);
		moveOpponent();
	}while(isGameOver == 0);
	isGameOver = 0;
	// Clear the map screen if the game is over to show overall score of the player
	system("cls");
	printFromFile("GameOver.txt");
	printFromFile("GameOver2.txt");
	while (myTimer(500) == -1){
		
	}
	printLine("");
	greenColor();
	printf("==========================================\n");
	printf("%s get %i!\n", player->name, player->score);
	printf("Now saving data...\n");
	printf("==========================================\n");
	char fullPath[200] = {};
	giveFullPath(fullPath, 3, "HighScore/", player->name, ".txt");
	if (getScoreData(fullPath, 1) < player->score){
		printf("REACHED HERE!\n");
		printSeparateFiles(fullPath, "si", 2, player->name, player->score);
	}
	printLine("Your data has been saved successfully!");
	resetColor();
	getchar();
	freeMemory();
}
void TestFunctionality(){
	srand(time(0));
	system("cls");
	getMapDataFromFile("MapDjikstra.txt");
	printf("Reached too!\n");
	unsigned int randomizer = rand();
	player->currentPath = findPosition(randomizer);
	randomizer -= 1333;
	opponent->currentPath = findPosition(randomizer);
	printLine("Now generating paths if opponent reaches player... Get Ready...");
	doDjikstraSearchMap(1);
	greenColor();
	printf("Path has been generated, marked as @! Press enter to continue!\n");
	printMap2D();
	yellowColor();
	printf("Original Map:\n\n");
	printFromFile("MapDjikstra.txt");
    getchar();
    resetColor();
    freeMemory();
}

void showHighScoreMenu(){
	int choice = 0;
	do{
		system("cls");
		printAllFilesFromDirectory("HighScore/");
		magentaColor();
		printLine("Please tell me what to do:");
		printLine("1. Modify Name");
		printLine("2. Sort Data");
		printLine("3. Delete All Data");
		printLine("4. Exit");
		yellowColor();
		choice = giveRangedValues(1, 4, "Choose: ");
		if (choice == 1){
			char fileName[150] = {};
			printf("Please enter the name you'd like to rename: ");
			scanf("%[^\n]", fileName);
			getchar();
			printf("Enter the new name: ");
			scanf("%[^\n]", fileName);
			getchar();
		}
		else if (choice == 2){
			
		}
		else if (choice == 3){
			
		}
	}while(choice != 4);
	resetColor();
}
int main(){
	int choice = 0;
	player = (Player*) malloc(sizeof(Player));
	player->currentMovePoints = 0;
	opponent = (Opponent*) malloc(sizeof(Opponent));
	do{
		system("cls");
		printLine("A beta-version of CHASE-ME program");
		printLine("You can test it if you want to!");
		printLine("Choose what you would like to do!");
		printLine("1. Test Functionality");
		printLine("2. Run Game");
		printLine("3. How to Play");
		printLine("4. Records");
		printLine("5. Exit");
		choice = giveRangedValues(1, 5, "Choose: ");
		if (choice == 1){
			TestFunctionality();	
		}
		else if (choice == 2){
			MainGame();
		}
		else if (choice == 3){
			printFromFile("How To Play.txt");
			getchar();
		}
		else if (choice == 4){
			showHighScoreMenu();
		}
		else if (choice == 5){
			printf("Thank you for using my app!\n");
			getchar();
		}
	}while(choice != 5);
	return 0;
}
