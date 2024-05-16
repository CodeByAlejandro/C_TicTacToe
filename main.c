#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Some hardcoded constants
#define TITLE "Tic Tac Toe"
#define TITLE_LENGTH strlen(TITLE)
#define BLANC_FIELD_VALUE ' '
#define MAX_ROWS 30
#define MAX_COLUMNS 50

// enums
enum yesOrNo { YES, NO };

// function prototypes
int requestGameProperties(void);
int allocateBoardMemory(void);
void freeDynamicMemory(void);
enum yesOrNo yesOrNoQuestion(char *question, enum yesOrNo defaultAnswer);
int requestIntInRange(char *question, int lowerBound, int upperBound);
void initializeBoard(void);
int requestPlayerInput(int playerNbr);
bool markBoard(int fieldNbr, char mark);
bool chkWinCondition(int fieldNbr, char mark);
int chkAdjMarksInDirection(int rowIdx, int columnIdx, int rowStep, int columnStep, char mark);
bool chkForDraw(void);
int getRowIdxForFieldNbr(int fieldNbr);
int getColumnIdxForFieldNbr(int fieldNbr);
int refreshScreen(bool isPlayer1X, int player1Score, int player2Score);
int printHeader(bool isPlayer1X, int player1Score, int player2Score);
void printTitle(int rowWidth);
int printScores(bool isPlayer1X, int player1Score, int player2Score, int rowWidth);
int drawBoard(void);
int calcFieldWidth(void);
int calcNumberWidth(int number);

// Global pointers to dynamically allocated memory
int *ROWS = NULL;
int *COLUMNS = NULL;
int *N_IN_A_ROW = NULL;
char **BOARD = NULL;

/*
 * main function with main game loop
 */
int main(int argc, char **argv) {
    int returnCode = 0;
        
    // Request global constants with game properties
    returnCode = requestGameProperties();
    if (returnCode != 0) return returnCode;
    
    // Allocate board memory
    returnCode = allocateBoardMemory();
    if (returnCode != 0) return returnCode;
    
    // Set main game variables
    bool isPlayer1X = true;
    bool isPlayer1Turn = true;
    int player1Score = 0;
    int player2Score = 0;
    int fieldNbr = 0;
    bool isGameWon = false;
    bool isEscExitGame = false;

    // Initialize game
    initializeBoard();
    returnCode = refreshScreen(isPlayer1X, player1Score, player2Score);
    if (returnCode != 0) {
        freeDynamicMemory();
        return returnCode;
    }
    
    // Main game loop
    do {
        // Request current player's next mark by field number
        char mark;
        if (isPlayer1X) {
            if (isPlayer1Turn) {
                fieldNbr = requestPlayerInput(1);
                mark = 'X';
            } else {
                fieldNbr = requestPlayerInput(2);
                mark = 'O';
            }
        } else {
            if (isPlayer1Turn) {
                fieldNbr = requestPlayerInput(1);
                mark = 'O';
            } else {
                fieldNbr = requestPlayerInput(2);
                mark = 'X';
            }
        }
        // Mark the board at the given field number with the current player's mark
        bool validMark = markBoard(fieldNbr, mark);
        // If the mark is invalid, just repeat the question
        if (!validMark) {
            continue;
        }
        // Refresh the screen to show the new mark
        returnCode = refreshScreen(isPlayer1X, player1Score, player2Score);
        if (returnCode != 0) break;
        // Check if current player has won
        isGameWon = chkWinCondition(fieldNbr, mark);
        // If current player wins, propose optional rematch
        if (isGameWon) {
            // Say who wins
            isPlayer1Turn ? printf("=> Player1 wins!\n\n") : printf("=> Player2 wins!\n\n");
            // Pose rematch game question
            enum yesOrNo answer = yesOrNoQuestion("Do you want a rematch?", YES);
            // Check the answer for the rematch question
            switch (answer) {
                // When rematch is selected
                case YES:
                    // Increment player score
                    isPlayer1Turn ? player1Score++ : player2Score++;
                    // Switch who's X now, since X will always start
                    isPlayer1X = !isPlayer1X;
                    // Switch who gets to start based on who's X
                    isPlayer1Turn = isPlayer1X;
                    // Initialize game again
                    initializeBoard();
                    returnCode = refreshScreen(isPlayer1X, player1Score, player2Score);
                    break;
                // When game is to be exited
                case NO:
                    isEscExitGame = true;
                    printf("\nAs if you have anything better to do... ;)\n");
                    getchar();
                    break;
            }
            if (returnCode != 0) break;
        } else if(chkForDraw()) {
            printf("=> It's a draw!\n\n");
            enum yesOrNo answer = yesOrNoQuestion("Do you want to continue playing?", YES);
            // Check the answer for the continue question
            switch (answer) {
                // When rematch is selected
                case YES:
                    // Switch who's X now, since X will always start
                    isPlayer1X = !isPlayer1X;
                    // Switch who gets to start based on who's X
                    isPlayer1Turn = isPlayer1X;
                    // Initialize game again
                    initializeBoard();
                    returnCode = refreshScreen(isPlayer1X, player1Score, player2Score);
                    break;
                // When game is to be exited
                case NO:
                    isEscExitGame = true;
                    printf("\nAs if you have anything better to do... ;)\n");
                    getchar();
                    break;
            }
            if (returnCode != 0) break;
        } else {
            // Change player turns
            isPlayer1Turn = !isPlayer1Turn;
        }
    } while (!isEscExitGame);
    
    // Return exit code when game is to be exited
    freeDynamicMemory();
    return returnCode;
}

/*
 * Request game properties (= constants) and set them through global pointers
 */
int requestGameProperties(void) {
    printTitle(0);
    ROWS = (int *) malloc(sizeof(int));
    if (ROWS == NULL) {
        printf("=> Memory allocation for number of rows failed!\n");
        return 1;
    }
    COLUMNS = (int *) malloc(sizeof(int));
    if (COLUMNS == NULL) {
        printf("=> Memory allocation for number of columns failed!\n");
        free(ROWS);
        return 1;
    }
    N_IN_A_ROW = (int *) malloc(sizeof(int));
    if (N_IN_A_ROW == NULL) {
        printf("=> Memory allocation for number of similar marks in string failed!\n");
        free(ROWS);
        free(COLUMNS);
        return 1;
    }
    enum yesOrNo answer = yesOrNoQuestion("Do you want to play default TicTacToe?", YES);
    *ROWS = answer == YES ? 3 : requestIntInRange("Enter number of rows for board/grid", 3, MAX_ROWS);
    *COLUMNS = answer == YES ? 3 : requestIntInRange("Enter number of columns for board/grid", 3, MAX_COLUMNS);
    *N_IN_A_ROW = answer == YES ? 3 : requestIntInRange("Enter number of consecutive marks needed for a win", 3, *ROWS < *COLUMNS ? *ROWS : *COLUMNS);
    return 0;
}

/*
 * Dynamically allocate board memory based on game properties
 * and set it through a global pointer
 */
int allocateBoardMemory(void) {
    int returnValue = 0;
    BOARD = (char **) malloc(*ROWS * sizeof(char *));
    if (BOARD == NULL) {
        printf("=> Memory allocation for board failed!\n");
        returnValue = 1;
    } else {
        for (int i = 0; i < *ROWS; i++) {
            BOARD[i] = (char *) malloc(*COLUMNS * sizeof(char));
            if (BOARD[i] == NULL) {
                printf("=> Memory allocation for board row failed!\n");
                returnValue = 1;
                break;
            }
        }
    }
    if (returnValue != 0) {
        free(ROWS);
        free(COLUMNS);
        free(N_IN_A_ROW);
    }
    return returnValue;
}

/*
 * Free dynamically allocated board memory
 */
void freeDynamicMemory(void) {
    for (int i = 0; i < *ROWS; i++) {
        free(BOARD[i]);
    }
    free(BOARD);
    free(ROWS);
    free(COLUMNS);
    free(N_IN_A_ROW);
}

/*
 * Request yes-or-no-question with default value
 */
enum yesOrNo yesOrNoQuestion(char *question, enum yesOrNo defaultAnswer) {
    // Ask question till we get a valid answer
    char answer;
    do {
        // Pose yes-or-no-question
        printf("%s [Y/N] (%c): ", question, defaultAnswer == YES ? 'Y' : 'N');
        // get answer char
        answer = getchar();
        // If nothing is entered, set default answer
        if (answer == '\n') {
            answer = defaultAnswer == YES ? 'Y' : 'N';
        } else {
            // Consume leftover chars till end of line
            for (char c; (c = getchar()) != '\n' && c != EOF;) {
                // When multiple characters were entered,
                // set invalid answer to restart rematch-loop
                if (answer != '\0') {
                    answer = '\0';
                }
            }
        }
    } while (answer != 'Y' && answer != 'N' && answer != 'y' && answer != 'n');
    
    // Translate answer to enum
    enum yesOrNo answerAsEnum = defaultAnswer;
    switch(answer) {
        case 'Y':
        case 'y':
            answerAsEnum = YES;
            break;
        case 'N':
        case 'n':
            answerAsEnum = NO;
            break;
    }
    
    // Return enum answer
    return answerAsEnum;
}

/*
 * Request an integer number in a given range, denoted by a lower bound and upper bound (inclusive)
 */
int requestIntInRange(char *question, int lowerBound, int upperBound) {
    int returnValue;
    bool validInput = false;
    do {
        printf("\n%s (min. %d, max. %d) > ", question, lowerBound, upperBound);
        int result = scanf("%d", &returnValue);
        if (result != 1) {
            printf("Please provide a valid integer number!\n");
        } else if (returnValue < lowerBound || returnValue > upperBound) {
            printf("Please provide a valid number in the range %d - %d!\n", lowerBound, upperBound);
        } else {
            validInput = true;
        }
        // Consume leftover chars till end of line
        for (char c; (c = getchar()) != '\n' && c != EOF;);
    } while (!validInput);
    return returnValue;
}

/*
 * Initialize the board (a.k.a. the playfield)
 */
void initializeBoard(void) {
    for (int i = 0; i < *ROWS; i++) {
        for (int j = 0; j < *COLUMNS; j++) {
            BOARD[i][j] = BLANC_FIELD_VALUE;
        }
    }
}

/*
 * Request a valid field number from a player
 */
int requestPlayerInput(int playerNbr) {
    int fieldNbr;
    bool validInput = false;
    do {
        printf("Player%d: Enter a field number > ", playerNbr);
        int result = scanf("%d", &fieldNbr);
        if (result != 1) {
            printf("Please provide a valid integer that represents a field number!\n\n");
        } else if (fieldNbr < 1 || fieldNbr > *ROWS * *COLUMNS) {
            printf("Please provide a valid field number in the range 1 - %d!\n\n", *ROWS * *COLUMNS);
        } else {
            validInput = true;
        }
        // Consume leftover chars till end of line
        for (char c; (c = getchar()) != '\n' && c != EOF;);
    } while (!validInput);
    return fieldNbr;
}

/*
 * Mark the board (a.k.a. the playfield) at the field denoted by the fieldNbr
 * with the character denoted by the mark, but only if it is not set already.
 */
bool markBoard(int fieldNbr, char mark) {
    // Initialize return value
    bool validMark = false;
    
    // Translate field number to row and column indices
    int rowIdx = getRowIdxForFieldNbr(fieldNbr);
    int columnIdx = getColumnIdxForFieldNbr(fieldNbr);
    
    // Mark board if field is still blanc
    if (BOARD[rowIdx][columnIdx] == BLANC_FIELD_VALUE) {
        BOARD[rowIdx][columnIdx] = mark;
        validMark = true;
    } else {
        printf("You cannot mark a field that has already been marked!\n\n");
    }
    
    // Return if mark is valid
    return validMark;
}

/*
 * Check if the current move made the current player win the game
 */
bool chkWinCondition(int fieldNbr, char mark) {
    // Set return variable
    bool hasWon = false;
    
    // Translate field number to row and column indices
    int rowIdx = getRowIdxForFieldNbr(fieldNbr);
    int columnIdx = getColumnIdxForFieldNbr(fieldNbr);
     
    // Initialize variables
    int rowStep = -1;
    int columnStep = -1;
    int initialRowStep = rowStep;
    int initialColumnStep = columnStep;
    char stepIdx = 'C';
    int countMarksInAllDirections[8] = { 0 };
    int arrIdx = 0;
    
    /*
     * Loop all possible directions from the current field by expressing a direction
     * as a -1 or +1 step to apply to either the row index, column index or both indices.
     * 
     * Possible directions are:
     *   |  R-IDX   |  C-IDX   |
     *   |----------|----------|
     *   |  -1      |  -1   |  |
     *   |  -1      |  +0   |  |
     *   |  -1   |  |  +1  \ / |
     *   |  +0   |  |  +1      |
     *   |  +1  \ / |  +1   |  |
     *   |  +1      |  +0   |  |
     *   |  +1   |  |  -1  \ / |
     *   |  +0   |  |  -1      |
     *  (|  -1  \ / |  -1      |) -> end of loop
     * 
     * The downward arrows indicate the order of the loop,
     * which alternates between inversing the column index and the row index.
     */
    do {
        // Count adjacent similar marks in the current direction and store it in the countMarksInAllDirections array
        countMarksInAllDirections[arrIdx] = chkAdjMarksInDirection(rowIdx, columnIdx, rowStep, columnStep, mark);
        // Check if we reach N_IN_A_ROW in the current direction
        if (countMarksInAllDirections[arrIdx] + 1 == *N_IN_A_ROW) {
            hasWon = true;
            break;
        }
        // When over half of the circle of directions, also check if we reach N_IN_A_ROW by combining opposite directions
        if (arrIdx > 3) {
            int countMarksInOppositeDirections = countMarksInAllDirections[arrIdx] + countMarksInAllDirections[arrIdx - 4];
            if (countMarksInOppositeDirections + 1 >= *N_IN_A_ROW) {
                hasWon = true;
                break;
            }
        }
        // Increment the countMarksInAllDirections array index
        arrIdx++;
        
        // Logic to change to the next direction in the circle of 8 directions
        if (stepIdx == 'C') {
            if (columnStep != -initialColumnStep) {
                initialColumnStep < 0 ? columnStep++ : columnStep--;
            }
            if (columnStep == -initialColumnStep) {
                stepIdx = 'R';
                initialRowStep = rowStep;
            }
        } else if (stepIdx == 'R') {
            if (rowStep != -initialRowStep) {
                initialRowStep < 0 ? rowStep++ : rowStep--;
            }
            if (rowStep == -initialRowStep) {
                stepIdx = 'C';
                initialColumnStep = columnStep;
            }
        }
    } while (rowStep != -1 || columnStep != -1);
    
    // Return whether the last mark made the current player win
    return hasWon;
}

/*
 * Check the number of similar adjacent marks in the current direction
 */
int chkAdjMarksInDirection(int rowIdx, int columnIdx, int rowStep, int columnStep, char mark) {
    // Check at max N_IN_A_ROW - 1 positions in the direction denoted by the rowStep and columnStep parameters
    // to find a N_IN_A_ROW long string of the current mark. Also keep track of the number of similar marks found
    // in this direction, since a string might be formed together with the opposite direction.
    int countMarksInDirection = 0;
    int idxNextMarkInStr = 1;
    while (idxNextMarkInStr < *N_IN_A_ROW) {
        // Get indices of next field in direction
        int rowIdxNextMark = rowIdx + (idxNextMarkInStr * rowStep);
        int columnIdxNextMark = columnIdx + (idxNextMarkInStr * columnStep);
        // Check whether the current indices denote a field inside the board grid
        if (rowIdxNextMark < 0 || rowIdxNextMark > *ROWS - 1 ||
            columnIdxNextMark < 0 || columnIdxNextMark > *COLUMNS - 1) {
            break;
        }
        // Check if the mark of this field is what whe are looking for
        if (BOARD[rowIdxNextMark][columnIdxNextMark] == mark) {
            countMarksInDirection++;
        } else {
            // Otherwise break the loop since the string ends here on this side
            break;
        }
        // Go to next index in string
        idxNextMarkInStr++;
    }
    return countMarksInDirection;
}

/*
 * Check if the last move got us in a draw situation
 */
bool chkForDraw(void) {
    bool isDraw = true;
    for (int i = 0; i < *ROWS; i++) {
        for (int j = 0; j < *COLUMNS; j++) {
            if (BOARD[i][j] == BLANC_FIELD_VALUE) {
                isDraw = false;
            }
        }
    }
    return isDraw;
}

/*
 * Get row index in the board array that corresponds to a given fieldNbr
 */
int getRowIdxForFieldNbr(int fieldNbr) {
    int rowIdx;
    if (fieldNbr % *COLUMNS == 0) {
        rowIdx = (fieldNbr / *COLUMNS) - 1;
    } else {
        rowIdx = fieldNbr / *COLUMNS;
    }
    return rowIdx;
}

/*
 * Get column index in the board array that corresponds to a given fieldNbr
 */
int getColumnIdxForFieldNbr(int fieldNbr) {
    int columnIdx;
    if (fieldNbr % *COLUMNS == 0) {
        columnIdx = *COLUMNS - 1;
    } else {
        columnIdx = (fieldNbr % *COLUMNS) - 1;
    }
    return columnIdx;
}

/*
 * Refresh everything drawn on the screen
 */
int refreshScreen(bool isPlayer1X, int player1Score, int player2Score) {
    printf("\033[2J\033[H");
    int returnCode = 0;
    returnCode = printHeader(isPlayer1X, player1Score, player2Score);
    if (returnCode != 0) return returnCode;
    returnCode = drawBoard();
    if (returnCode != 0) return returnCode;
    return returnCode;
}

/*
 * Print the title and scores and center them against the board
 */
int printHeader(bool isPlayer1X, int player1Score, int player2Score) {
    int returnCode = 0;
    int fieldWidth = calcFieldWidth();
    int rowWidth = (*COLUMNS * (fieldWidth + 2)) + *COLUMNS - 1;
    printTitle(rowWidth);
    returnCode = printScores(isPlayer1X, player1Score, player2Score, rowWidth);
    return returnCode;
}

/*
 * Print the title and center it against the board
 */
void printTitle(int rowWidth) {
    int paddingWidth = (rowWidth - TITLE_LENGTH) / 2;
    if (paddingWidth < 0) {
        paddingWidth = 0;
    }
    printf("%*s%s\n\n", paddingWidth, "", TITLE);
}

/*
 * Print the scores and center them against the board
 */
int printScores(bool isPlayer1X, int player1Score, int player2Score, int rowWidth) {
    int returnCode = 0;
    int player1ScoreWidth = calcNumberWidth(player1Score);
    if (player1ScoreWidth < 0) {
        printf("Player 1 score is to large: %d!\n\n", player1Score);
        returnCode = 1;
    }
    int player2ScoreWidth = calcNumberWidth(player2Score);
    if (player2ScoreWidth < 0) {
        printf("Player 2 score is to large: %d!\n\n", player2Score);
        returnCode = 1;
    }
    if (returnCode == 0) {
        int paddingWidth = (rowWidth - 29 - player1ScoreWidth - player2ScoreWidth) / 2;
        if (paddingWidth < 0) {
            paddingWidth = 0;
        }
        if (isPlayer1X) {
            printf("%*sPlayer1(X): %d  -  Player2(O): %d\n\n", paddingWidth, "", player1Score, player2Score);
        } else {
            printf("%*sPlayer1(O): %d  -  Player2(X): %d\n\n", paddingWidth, "", player1Score, player2Score);
        }
    }
    return returnCode;
}

/*
 * Draw the board itself (a.k.a. the playfield)
 */
int drawBoard(void) {
    int returnCode = 0;
    int fieldWidth = calcFieldWidth();
    if (fieldWidth > 0) {
        for (int i = 0; i < *ROWS; i++) {
            // Sub top row
            for (int j = 0; j < *COLUMNS; j++) {
                if (j != 0) {
                    // Draw unicode box drawing vertical bar character
                    printf("\u2502");
                }
                char fieldValue = BOARD[i][j];
                if (fieldValue != BLANC_FIELD_VALUE) {
                    printf(" %*c ", fieldWidth, fieldValue);
                } else {
                    printf(" %*d ", fieldWidth, i * *COLUMNS + j + 1);
                }
                if (j == *COLUMNS - 1) {
                    printf("\n");
                }
            }
            // Sub bottom row
            for (int j = 0; j < *COLUMNS; j++) {
                if (j != 0 && i != *ROWS - 1) {
                    // Draw unicode box drawing cross character
                    printf("\u253C");
                }
                if (i != *ROWS - 1) {
                    for (int k = 0; k < fieldWidth + 2; k++) {
                        // Draw unicode box drawing horizontal bar character
                        printf("\u2500");
                    }
                } else {
                    printf(" %*c ", fieldWidth, BLANC_FIELD_VALUE);
                }
                if (j == *COLUMNS - 1) {
                    printf("\n");
                }
            }
        }
    } else {
        returnCode = 1;
    }
    return returnCode;
}

/*
 * Calculate the width of a single field in the grid of the board
 */
int calcFieldWidth() {
    int fieldWidth = -1;
    if (*ROWS <= MAX_ROWS && *COLUMNS <= MAX_COLUMNS) {
        fieldWidth = calcNumberWidth(*ROWS * *COLUMNS);
    }
    if (fieldWidth < 0) {
        printf("Board size to large: %d rows by %d columns!\n\n", *ROWS, *COLUMNS);
    }
    return fieldWidth;
}

/*
 * Calculate the width of an integer number
 */
int calcNumberWidth(int number) {
    int numberWidth = -1;
    if (number < 10) {
        numberWidth = 1;
    } else if (number >= 10 && number < 100)  {
        numberWidth = 2;
    } else if (number >= 100 && number < 1000) {
        numberWidth = 3;
    } else if (number >= 1000 && number < 10000) {
        numberWidth = 4;
    }
    return numberWidth;
}