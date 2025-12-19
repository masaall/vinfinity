
#include "types.h"
#include "user.h"

int board[9][9] =  {{ 3, 0, 6, 5, 0, 8, 4, 0, 0 },
                    { 5, 2, 0, 0, 0, 0, 0, 0, 0 },
                    { 0, 8, 7, 0, 0, 0, 0, 3, 1 },
                    { 0, 0, 3, 0, 1, 0, 0, 8, 0 },
                    { 9, 0, 0, 8, 6, 3, 0, 0, 5 },
                    { 0, 5, 0, 0, 9, 0, 6, 0, 0 },
                    { 1, 3, 0, 0, 0, 0, 2, 5, 0 },
                    { 0, 0, 0, 0, 0, 0, 0, 7, 4 },
                    { 0, 0, 5, 2, 0, 6, 3, 0, 0 }};

int isFull(int *row, int *col){

	for (*row = 0; *row < 9; (*row)++){
		for (*col = 0; *col < 9; (*col)++){
			if (board[*row][*col] == 0)
				return 0;
		}
	}
	return 1;
}

void printBoard(){
	for (int row = 0; row < 9; row++){
		for (int col = 0; col < 9; col++){
			printf("%d ", board[row][col]);
		}
		printf("\n");
	}
	printf("\n");
}

int empty;

void printBoard1(){
	for (int row = 0; row < 9; row++){
		for (int col = 0; col < 9; col++){
			if (board[row][col] == 0) empty++;
			printf("%d ", board[row][col]);
		}
		printf("\n");
	}
	printf("\n");
}

int checkRow(int row, int num){
	for (int col = 0; col < 9; col++){
		if (board[row][col] == num)
			return 0;
	}
	return 1;
}

int checkCol(int col, int num){
	for (int row = 0; row < 9; row++){
		if (board[row][col] == num)
			return 0;
	}
	return 1;
}

int checkBox(int startRow, int startCol, int num){

	for (int row = 0; row < 3; row++){
		for (int col = 0; col < 3; col++){
			if (board[startRow + row][startCol + col] == num)
				return 0;
		}
	}

	return 1;
}

int isSafe(int row, int col, int num){
	return checkRow(row, num) 
		&& checkCol(col, num)
		&& checkBox(row - row % 3, col - col % 3, num)
		&& board[row][col] == 0;
}

void checkCountRowCol(int col, int row, int *rcnt, int *ccnt){

	for (int j = 0; j < 9; j++){
		if (board[j][col] == 0) (*rcnt)++;
	}

	for (int i = 0; i < 9; i++){
		if (board[row][i] == 0) (*ccnt)++;
	}
}

void checkCountBox(int row, int col, int *bcnt){

	for (int j = 0; j < 3; j++){
		for (int i = 0; i < 3; i++){
			if (board[row+j][col+i] == 0) (*bcnt)++;
		}
	}
}

void checkMissRow(int col){
	int i;
	printf("miss row\n");
	for (int i = 0; i < 9; i++){
		printf("%d ", board[i][col]);
	}

	printf("\n");

	for (int num = 1; num <= 9; num++){

		for (i = 0; i < 9; i++){
			if (board[i][col] == num)
				break;
		}
		if (board[i][col] == num)
			continue;
		printf("%d ", num);

		for (int j = 0; j < 9; j++){
			if (isSafe(j, col, num)){
				board[j][col] = num;
			}
		}
	}
	printf("\n");
}

void checkMissCol(int row){
	int i;
	printf("miss col\n");
	for (int i = 0; i < 9; i++){
		printf("%d ", board[row][i]);
	}
	printf("\n");

	for (int num = 1; num <= 9; num++){
		for (i = 0; i < 9; i++){
			if (board[row][i] == num)
				break;
		}
		if (board[row][i] == num)
			continue;
		printf("%d ", num);

		for (int j = 0; j < 9; j++){
			if (isSafe(row, j, num)){
				board[row][j] = num;
			}
		}
	}	
	printf("\n");
}

void checkMissBox(int row, int col){
	int a, b;
	printf("miss box\n");
	for (int j = 0; j < 3; j++){
		for (int i = 0; i < 3; i++){
			printf("%d ", board[row + j][col + i]);
		}
	}
	printf("\n");

	for (int num = 1; num <= 9; num++){
		for (a = 0; a < 3; a++){
			for (b = 0; b < 3; b++){
				if (board[row + a][col + b] == num)
					goto here;
			}
		}
here:
		if (board[row + a][col + b] == num)
			continue;
		printf("%d ", num);

		for (int j = 0; j < 3; j++){
			for (int i = 0; i < 3; i++){
				if (isSafe(row + j, col + i, num)){
					board[row + j][col + i] = num;
				}
			}
		}
	}
	printf("\n");
}

int checkAll(){
		int rcnt, ccnt, bcnt;

	for (int i = 0; i < 9; i++){
		checkCountRowCol(i, i, &rcnt, &ccnt);
		printf("%d  %d %d \n", i, rcnt, ccnt);
		rcnt = 0, ccnt = 0;
	}

	printf("\n");

	for (int j = 0; j < 9; j += 3){
		for (int i = 0; i < 9; i+= 3){
			checkCountBox(j, i, &bcnt);
			printf("%d %d  %d \n", j, i, bcnt);
			bcnt = 0;
		}
	}

	checkMissBox(0, 0);

	return 1;
}

int solveSudoku(){

	for (;;){
		break;
	}

	return 1;
}

int solveSudoku1(){

	int row, col;

	if (isFull(&row, &col))
		return 1;

	for (int num = 1; num <= 9; num++){
		if (isSafe(row, col, num)){
			
			board[row][col] = num;

			if (solveSudoku()){
				return 1;
			}
			
			board[row][col] = 0;
		}
	}

	return 0;
}

int main(void){
	printBoard();

//	checkAll();

	if (solveSudoku1())
		printBoard1();

	printf("empty %d \n", empty);

	exit();
}
