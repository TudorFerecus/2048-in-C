#include <curses.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>

#define BOX_HEIGHT 17
#define BOX_WIDTH 17
#define WIDTH_MULTIPLIER 2
#define ROW_NR 4
#define COL_NR 4
#define LOOP 1
#define BOX_OFFSET 2
#define BOX_MIDDLE_OFFSET 7
#define MOVE_DELAY 5000

// game state pentru a tine evidenta momentului specific al jocului
typedef enum game_state
{
    START_SCREEN,
    PLAYING,
    PAUSE_SCREEN,
    END_SCREEN,
    QUIT
}game_state;

// schimba doua valori
void swapValuesChar(char *a, char *b)
{
    char aux = *a;
    *a = *b;
    *b = aux;

}

// initializeaza dinamic o matrice de 4 pe 4
int **initMatrix()
{
    int **numbers = malloc((ROW_NR) * sizeof(int *));

    int i;
    for(i = 0; i < ROW_NR; i++)
        numbers[i] = malloc((COL_NR) * sizeof(int));
    
    return numbers;

}

// in afisarea in gridul de 2048, pentru ca nr sa ramana centrate li se
// verifica nr de cifre si se genereaza un offset
int getPlacingOffset(int n)
{
    if(n / 1000 > 0) return 2;
    if(n / 10 > 0) return 3;
    return 4;
}

// copierea numerelor dintr-o matrice in alta
void copyNumbersMatrix(int **copy, int **numbers)
{
    int i, j;
    for(i = 0; i < ROW_NR; i++)
        for(j = 0; j < COL_NR; j++)
            copy[i][j] = numbers[i][j];
}

// in lastNumbers.txt este memorata ultima configuratie a gridului 2048, nr de
// locuri libere si scorul. Functia citeste si atribuie variabilei "numbers"
// ultima configuratie, la fel si pentru "freeSpots" si "score"
void copyNumbers(int **numbers, int *freeSpots, int *score)
{
    FILE *fptr;
    fptr = fopen("lastNumbers.txt", "r");

    int i, j;
    for(i = 0; i < ROW_NR; i++)
        for(j = 0; j < COL_NR; j++)
            fscanf(fptr, "%d",&numbers[i][j]);
    fscanf(fptr, "%d", freeSpots);
    fscanf(fptr, "%d", score);
    fclose(fptr);
}

// functia afiseaza in "lastNumbers.txt" ultima configuratie a matricei de
// numere, atat cat si cate locuri libere au ramas si scorul
void updateNumbers(int **numbers, int freeSpots, int score)
{
    int i, j;
    FILE *fptr;
    fptr = fopen("lastNumbers.txt", "w");
    for(i = 0; i < ROW_NR; i++)
        for(j = 0; j < COL_NR; j++)
            fprintf(fptr, "%d ",numbers[i][j]);
    fprintf(fptr, "%d ", freeSpots);
    fprintf(fptr, "%d ", score);

    fclose(fptr);
}

// in data.txt sunt pastrate scorurile maxime
// cand un joc se termina se schimba lista, sortand-o
void updateData(int score)
{
    int scores[100], i = 0;
    FILE *fptrin = fopen("data.txt", "r");

    // se citesc datele
    while(fscanf(fptrin, "%d", &scores[i++]) == 1)
    {
        ;
    }
    fclose(fptrin);
    
    FILE *fptrout = fopen("data.txt", "w");
    i--;

    // daca nu are scoruri inregistrate se afiseaza doar scorul
    if(i == 0)
    {
        fprintf(fptrout, "%d ", score);
    }

    else
    {
        // se sorteaza descrescator scorurile
        int d, j;
        scores[i++] = score;
        for(j = 0; j < i - 1; j++)
        {
            for(d = j + 1; d < i; d++)
            {
                if(scores[j] < scores[d])
                {
                    int aux = scores[j];
                    scores[j] = scores[d];
                    scores[d] = aux;
                }
            }
        }

        // se afiseaza in fisier
        for(j = 0; j < i; j++)
            fprintf(fptrout, "%d ", scores[j]);
    }
    fclose(fptrout);
}

// la fiecare schimbare a numerelor curente, se tine evidenta ultimelor nr
void setLastNumbers(int **numbers, int **lastNumbers, 
                        int freeSpots, int *lastFreeSpots)
{
    int i, j;
    for(i = 0; i < ROW_NR; i++)
    {
        for(j = 0; j < COL_NR; j++)
        {
            lastNumbers[i][j] = numbers[i][j];
            *lastFreeSpots = freeSpots;
        }
    }
}

// obtine un nr random intre cele doua capete lower si upper, de regula 1 si 10
int randomizeNr(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

// in functie de numarul random primit, decide daca returneaza 2 sau 4
// 2 are sansa 80%, 4 are 20%
int getNr()
{
    int nr = randomizeNr(1, 10);
    if(nr <= 2) return 4;
    return 2;
}

// returneaza un sir de caractere cu timpul
char *getTime()
{
    time_t currentTime;
    time(&currentTime);
    return ctime(&currentTime);
}

// in functie de spotul random cu valori intre [0, nr de spatii libere]
// plaseaza valoare nr (cu valoare 2 sau 4) in matricea numbers
void addToSpot(int **numbers, int spot, int nr)
{
    int i, j, cnt = 0;
    int found = 0;
    for(i = 0; i < ROW_NR && !found; i++)
    {
        for(j = 0; j < COL_NR && !found; j++)
        {
            if(numbers[i][j] == -1)
            {
                cnt += 1;
                if(cnt == spot)
                {
                    found = 1;
                    numbers[i][j] = nr;
                    break;
                }
            }
        }
    }
}

// initializeaza matricea la inceputul jocului cu cele doua numere 2 sau 4
void initNumbers(int **numbers, int *freeSpots)
{
    int i, j;
    *freeSpots = ROW_NR * COL_NR;

    // initializeaza matricea cu -1, fiind valoarea de celula libera
    for(i = 0; i < ROW_NR; i++)
    {
        for(j = 0; j < COL_NR; j++)
        {
            numbers[i][j] = -1;
        }
    }

    // primeste doua numere intre 0 si 3 reprezentand coordonatele primului
    // element din matrice
    i = randomizeNr(0, 3);
    j = randomizeNr(0, 3);

    // atribuie pozitiei un numar 2 sau 4
    numbers[i][j] = getNr();
    int copI = i, copJ = j;

    // adauga un alt numar 2 sau 4 conform cerintei, avand grija sa nu se
    // suprapuna cu precedentul
    while(copI == i && copJ == j)
    {
        i = randomizeNr(0, 3);
        j = randomizeNr(0, 3);    
    }

    numbers[i][j] = getNr();
    *freeSpots -= 2;
}

// returneaza mijlocul ecranului pe axa X
int getWindowMiddleX(WINDOW *window)
{
    return (getmaxx(window) / 2);
}

// returneaza mijlocul ecranului pe axa Y
int getWindowMiddleY(WINDOW *window)
{
    return (getmaxy(window) / 2);
}

// initializeaza un window cu margini de tip box in mijlocul ecranului
WINDOW *initMiddleBox(WINDOW *window, int height, int width, int offset)
{
    // obtine mijlocul ecranului
    int maxX, maxY;
    getmaxyx(window, maxY, maxX);
    int boxStartX = maxX / 2 - width;
    int boxStartY = (maxY - height) / 2 + offset;

    // initializeaza fereastra
    WINDOW *boxWindow = newwin(height, 
                        width * WIDTH_MULTIPLIER - 1, boxStartY, boxStartX);

    keypad(boxWindow, TRUE);
    cbreak();
    noecho();
    return boxWindow;
    
}

// adauga un numar in matrice la o pozitie la intamplare libera, avand
// grija sa nu se fi terminat jocul
void addNumber(int **numbers, int *freeSpots)
{
    if(*freeSpots == 0) 
    {
        printf("YOU LOST\n");
        return;
    }

    int spot = randomizeNr(1, *freeSpots);
    int val = getNr();
    addToSpot(numbers, spot, val);
    *freeSpots -= 1;
}

// realizeaza mutarea la stanga a gridului
int swipeLeft(int **numbers, int *freeSpots, int *score)
{
    // actionMade verifica daca miscarea a modificat matricea
    int actionMade = 0;
    int i, j;

    for(i = 0; i < ROW_NR; i++)
    {
        for(j = 0; j < COL_NR; j++)
        {
            // cauta un numar in dreapta numarului pana cand acesta exista
            // sau se termina randul
            int d = j + 1;
            while (numbers[i][d] == -1 && d < COL_NR)
                d++;
            
            // daca exista un numar
            if (d != COL_NR)
            {
                // daca celula actuala este goala plasam numarul gasit in ea
                if(numbers[i][j] == -1)
                {
                    numbers[i][j] = numbers[i][d];
                    numbers[i][d] = -1;
                    actionMade = 1;
                    j--;
                }

                // daca celula actuala nu este goala si este egala cu numarul
                // gasit, se dubleaza
                else if(numbers[i][j] == numbers[i][d])
                {
                    numbers[i][j] *= 2;
                    *score += numbers[i][j];
                    numbers[i][d] = -1;
                    actionMade = 1;
                    *freeSpots += 1;
                }                    
            }  
        }
    }
    if(actionMade)
    {
        addNumber(numbers, freeSpots);
    }
    return actionMade;
}

// realizeaza mutarea la dreapta a gridului
int swipeRight(int **numbers, int *freeSpots, int *score)
{
    // actionMade verifica daca miscarea a modificat matricea
    int actionMade = 0;
    int i, j;
    for(i = 0; i < ROW_NR; i++)
    {
        for(j = COL_NR - 1; j >= 0; j--)
        {
            // cauta un numar in stanga numarului pana cand acesta exista
            // sau se termina randul
            int d = j - 1;
            while (numbers[i][d] == -1 && d >= 0)
                d--;

            // daca exista un numar
            if (d >= 0)
            {
                // daca celula actuala este goala plasam numarul gasit in ea
                if(numbers[i][j] == -1)
                {
                    numbers[i][j] = numbers[i][d];
                    numbers[i][d] = -1;
                    actionMade = 1;
                    j++;
                }

                // daca celula actuala nu este goala si este egala cu numarul
                // gasit, se dubleaza
                else if(numbers[i][j] == numbers[i][d])
                {
                    numbers[i][j] *= 2;
                    *score += numbers[i][j];
                    numbers[i][d] = -1;
                    actionMade = 1;
                    *freeSpots += 1;
                }                    
            }
            
        }
    }
    if(actionMade)
    {
        addNumber(numbers, freeSpots);
    }
    return actionMade;
}

// realizeaza mutarea in jos a gridului
int swipeDown(int **numbers, int *freeSpots, int *score)
{
    // actionMade verifica daca miscarea a modificat matricea
    int actionMade = 0;
    int i, j;
    for(i = 0; i < COL_NR; i++)
    {
        for(j = ROW_NR - 1; j >= 0; j--)
        {
            // cauta un numar in susul numarului pana cand acesta exista
            // sau se termina coloana
            int d = j - 1;
            while (d >= 0 && numbers[d][i] == -1)
                d--;
            
            // daca exista un numar
            if (d >= 0)
            {
                // daca celula actuala este goala plasam numarul gasit in ea
                if(numbers[j][i] == -1)
                {
                    numbers[j][i] = numbers[d][i];
                    numbers[d][i] = -1;
                    actionMade = 1;
                    j++;
                }

                // daca celula actuala nu este goala si este egala cu numarul
                // gasit, se dubleaza
                else if(numbers[j][i] == numbers[d][i])
                {
                    numbers[j][i] *= 2;
                    *score += numbers[j][i];
                    numbers[d][i] = -1;
                    actionMade = 1;
                    *freeSpots += 1;
                }                    
            }
            
        }

    }
    if(actionMade)
    {
        addNumber(numbers, freeSpots);
    }
    return actionMade;
}

// realizeaza mutarea in sus a gridului
int swipeUp(int **numbers, int *freeSpots, int *score)
{
    // actionMade verifica daca miscarea a modificat matricea
    int actionMade = 0;
    int i, j;
    for(i = 0; i < COL_NR; i++)
    {
        for(j = 0; j < ROW_NR; j++)
        {
            
            // cauta un numar in susul numarului pana cand acesta exista
            // sau se termina randul
            int d = j + 1;
            while (d < ROW_NR && numbers[d][i] == -1)
                d++;
            
            // daca exista un numar
            if (d != ROW_NR)
            {
                // daca celula actuala este goala plasam numarul gasit in ea
                if(numbers[j][i] == -1)
                {
                    numbers[j][i] = numbers[d][i];
                    numbers[d][i] = -1;
                    actionMade = 1;
                    j--;
                }

                // daca celula actuala nu este goala si este egala cu numarul
                // gasit, se dubleaza
                else if(numbers[j][i] == numbers[d][i])
                {
                    numbers[j][i] *= 2;
                    *score += numbers[j][i];
                    numbers[d][i] = -1;
                    actionMade = 1;
                    *freeSpots += 1;
                }
            }
        }
    }
    if (actionMade)
    {
        addNumber(numbers, freeSpots);
    }
    return actionMade;
}

// functia de undo
void undoNumbers(int **numbers, int **lastNumbers, int *freeSpots, 
                    int lastFreeSpots, int *score, int lastScore)
{
    // copiaza in matricea actuala matricea precedenta
    int i, j;
    for(i = 0; i < ROW_NR; i++)
    {
        for(j = 0; j < COL_NR; j++)
        {
            numbers[i][j] = lastNumbers[i][j];
            
        }
    }

    // updateaza scroul si locurile libere
    *freeSpots = lastFreeSpots;
    *score = lastScore;
}

// salveaza numarul maxim din matrice
void updateMax(game_state *gameStateMachine,int **numbers, int *maxNr)
{
    int i, j;
    for(i = 0; i < ROW_NR; i++)
    {
        for(j = 0; j < COL_NR; j++)
        {
            if(numbers[i][j] > *maxNr) *maxNr = numbers[i][j];
        }
    }

}

// initializeaza culorile cu codul lor corespunzator numarului
void initColors()
{
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);
    init_pair(16, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(32, COLOR_CYAN, COLOR_BLACK);
    init_pair(64, COLOR_YELLOW, COLOR_BLACK);
    init_pair(128, COLOR_YELLOW, COLOR_BLACK);
    init_pair(256, COLOR_YELLOW, COLOR_BLACK);
    init_pair(512, COLOR_YELLOW, COLOR_BLACK);
    init_pair(1024, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2048, COLOR_YELLOW, COLOR_BLACK);
}

// printeaza in window instructiunile
void printInstructions(WINDOW *window, int score)
{
    char *time = getTime();
    int offsetX = strlen(time) - 3;
    mvaddstr(0, 0, "Puteti sa mutati '*' folosind tastele:");
	mvaddstr(1, 2, "A - stanga");
	mvaddstr(2, 2, "D - dreapta");
	mvaddstr(3, 2, "W - sus");
	mvaddstr(4, 2, "S - jos");
    mvaddstr(5, 2, "U - undo");
	mvaddstr(6, 0, "Pentru meniu, apasati tasta Q.");
	
    mvaddstr(14, 0, "Mutari Valide:");

    mvaddstr(getWindowMiddleY(window) + BOX_HEIGHT / 2 + 3,
             getWindowMiddleX(window) - offsetX, time);
    move(getWindowMiddleY(window) + BOX_HEIGHT / 2 + 3, 
            getWindowMiddleX(window) + 10);
    printw("Score: %d", score);
}

// printeaza liniile verticale si orizontale ale gridului
void printLines(WINDOW *window)
{
    // calculeaza spatiul dintre liniile orizontale
    int nrOfLines = 3, i;
    int linesGap = (BOX_HEIGHT - 1) / 4;

    // afiseaza linii orizontale de lungimea
    // width-ului - 2 la pozitia corespunzatoare
    for(i = 1; i <= nrOfLines; i++)
    {
        wmove(window, linesGap * i, 1);
        whline(window, ACS_HLINE, (BOX_WIDTH - 1) * WIDTH_MULTIPLIER - 1);
    }
    
    // afiseaza linii orizontale de lungimea
    // height-ului - 2 la pozitia corespunzatoare
    linesGap = (BOX_WIDTH - 1) / 4;
    
    for(i = 1; i <= nrOfLines; i++)
    {
        wmove(window, 1, linesGap * i * WIDTH_MULTIPLIER);
        wvline(window, ACS_VLINE, (BOX_HEIGHT - 2));
    }

}

// afiseaza numerele la pozitia corespunzatoare in grid
void placeNumbers(WINDOW *window, int **numbers)
{
    int i, j;

    // distanta de la un numar la altul pe verticala si orizontala
    int moveOffsetY = (BOX_WIDTH - 1) / 4;
    int moveOffsetX = 2 * moveOffsetY;

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            int nr = numbers[i][j];
            if(nr != -1)
            {
                // initializeaza culoarea caracterului
                start_color();
                int placingOffset = getPlacingOffset(nr);
                wmove(window, 2 + moveOffsetY * i,
                         placingOffset + moveOffsetX * j);

                init_pair(1, COLOR_RED, COLOR_BLACK);
                wattron(window, COLOR_PAIR(nr));
                wprintw(window, "%d", nr);
                wattroff(window, COLOR_PAIR(1));
            }
        }
    }

}

// afiseaza gridul
void printBox(WINDOW *window, int **numbers)
{
    box(window, 0, 0);
    printLines(window);
    placeNumbers(window, numbers);
}

// functie cu care se afiseaza textul din start screen, formatat corespunzator
void printWordStart(WINDOW *window, int offsetX, int offsetY, char str[])
{
    int maxX, maxY;
    getmaxyx(window, maxY, maxX);

    int middleX = maxX / 2 + offsetX;
    int middleY = maxY / 2 + offsetY;

    wmove(window, middleY, middleX);
    wprintw(window, "%s", str);

}

// functie cu care se afiseaza numere, formatate corespunzator
void printWordStartInt(WINDOW *window, int offsetX, int offsetY, int nr)
{
    int maxX, maxY;
    getmaxyx(window, maxY, maxX);

    int middleX = maxX / 2 + offsetX;
    int middleY = maxY / 2 + offsetY;

    wmove(window, middleY, middleX);
    wprintw(window, "%d", nr);

}

// afisarea textului din start screen
void printStartScreenOptions(WINDOW *window)
{
    printWordStart(window, -3, -2, "NEW GAME");

    printWordStart(window, -2, 0, "RESUME");

    printWordStart(window, -1, 2, "QUIT");
}

// afisarea textului din end screen
void printEndScreenOptions(WINDOW *window, int maxNr)
{

    if(maxNr < 2048)
        printWordStart(window, -4, -7, "YOU LOST!");
    else    
        printWordStart(window, -3, -7, "YOU WON!");

    printWordStart(window, -3, -2, "NEW GAME");

    printWordStart(window, -2, 0, "STATS");

    printWordStart(window, -1, 2, "QUIT");
}

// functie prin care se afiseaza highlight-ul optiunii curente
void changeOption(WINDOW *window, int index)
{
    int middleX = getWindowMiddleX(window), i, offsetY;
    if( index == 0) offsetY = 0;
    if( index == 1) offsetY = 2;
    if( index == 2) offsetY = 4;

    wmove(window, getWindowMiddleY(window) - 2 + offsetY, middleX - BOX_MIDDLE_OFFSET);
    wprintw(window, "|");

    for(i = middleX - BOX_MIDDLE_OFFSET; i <= middleX + BOX_MIDDLE_OFFSET; i++)
    {
        wmove(window, getWindowMiddleY(window) - 3 + offsetY, i);
        wprintw(window, "-");
        wmove(window, getWindowMiddleY(window) - 1 + offsetY, i);
        wprintw(window, "-");
    }
    
    wmove(window, getWindowMiddleY(window) - 2 + offsetY, middleX + BOX_MIDDLE_OFFSET);
    wprintw(window, "|");

}

// functia care seteaza functionalitatea highlight-ului
void printHighlight(WINDOW *window, int *index, int dir)
{
    *index = (*index + dir) % 3;
    if(*index < 0) *index = 2;
    werase(window);
    box(window, 0, 0);
    printStartScreenOptions(window);
    changeOption(window, *index);
    
}

// functia care seteaza functionalitatea highlight-ului pentru end screen
void printHighlightEnd(WINDOW *window, int maxNr, int *index, int dir)
{
    *index = (*index + dir) % 3;
    if(*index < 0) *index = 2;
    werase(window);
    box(window, 0, 0);
    printEndScreenOptions(window, maxNr);
    changeOption(window, *index);
    
}

// actualizeaza matricea si toate variabilele dependente de ea
void changeBestMove(int **numbers, int **bestNumbers, int *bestScore, 
                    int *bestSpots, int *score, int *spots)
{
    copyNumbersMatrix(bestNumbers, numbers);
    *bestScore = *score;
    *bestSpots = *spots;
}

// swipeFun - pointer la functie (swipe: Up, Down, Left, Right)
// apeleaza swipeFun si actiualizeaza matricea si valorile dependente de ea
int swipeAction(int (*swipeFun)(int **, int *, int *), 
                int **numbers, int **lastNumbers, int *freeSpots, 
                int *lastFreeSpots, int *score, int *lastScore)
{
    *lastScore = *score;
    setLastNumbers(numbers, lastNumbers, *freeSpots, lastFreeSpots);
    int actionMade = (*swipeFun)(numbers, freeSpots, score);
    updateNumbers(numbers, *freeSpots, *score);
    return actionMade;
}

// verifica daca utilizatorul a pierdut
int checkLose(int **numbers, int **lastNumbers, 
              int *freeSpots, int *lastFreeSpots, int *score, int *lastScore)
{
    int **copyNr = initMatrix();
    int copyScore = *score, copySpots = *freeSpots, i;
    int (*swipeFun[4])(int **, int *, int *) = {swipeUp, swipeDown, swipeLeft, swipeRight};

    copyNumbersMatrix(copyNr, numbers);

    for(i = 0; i < 4; i++)
    {
        changeBestMove(numbers, copyNr, &copyScore, &copySpots, score, freeSpots);
        swipeAction(swipeFun[i], copyNr, lastNumbers, &copySpots, lastFreeSpots, &copyScore, lastScore);
        if(copySpots > 0)
        {
            return 0;
        }
    }


    return 1;

}

// apeleaza fiecare functie posibila si verifica care este cea mai buna mutare
void checkFutureMove(int **numbers, int **lastNumbers, int *freeSpots, 
                        int *lastFreeSpots, int *score, int *lastScore)
{
    int **copyNr = initMatrix(), **copyLastNr = initMatrix();
    int copyScore = *score, copySpots = *freeSpots;
    int copyLastSpots = *lastFreeSpots, i, copyLastScore = *lastScore;
    
    // vector de pointeri la functiile de miscare
    int (*swipeFun[4])(int **, int *, int *) = 
                {swipeUp, swipeDown, swipeLeft, swipeRight};
    
    // numele mutarilor ce vor fi afisate in legenda
    char moveNames[20][20] = {"Sus", "Jos", "Stanga", "Dreapta"};
    int textOffset = 16;

    // resetarea textului din legenda
    for(i = 0; i < 4; i++)
        mvaddstr(textOffset++, 3, "         ");
    textOffset = 16;

    for(i = 0; i < 4; i++)
    {
        copyNumbersMatrix(copyNr, numbers);
        copyNumbersMatrix(copyLastNr, lastNumbers);
        int actionMade = swipeAction(swipeFun[i], copyNr, copyLastNr,
                     &copySpots, &copyLastSpots, &copyScore, &copyLastScore);
        
        if(actionMade)
        {
            mvaddstr(textOffset++, 3, moveNames[i]);
        }
    }

}

// gaseste cea mai buna mutare
void doBestMove(int **numbers, int **lastNumbers, int *freeSpots,
                 int *lastFreeSpots, int *score, int *lastScore)
{
    int **copyNr = initMatrix(), **bestMove = initMatrix(), bestScore = *score, bestSpots = 0;
    int copyScore = *score, copySpots = *freeSpots, i;

    // vector de pointeri la functiile de miscare
    int (*swipeFun[4])(int **, int *, int *) = {swipeUp, swipeDown, swipeLeft, swipeRight};

    // prelucreaza o copie nu matricea efectiva
    copyNumbersMatrix(copyNr, numbers);

    // itereaza actiunile
    for(i = 0; i < 4; i++)
    {
        // reseteaza valorile-copie
        changeBestMove(numbers, copyNr, &copyScore, &copySpots,
                                                         score, freeSpots);

        // apeleaza actiunea
        int actionMade = swipeAction(swipeFun[i], copyNr, lastNumbers, 
                        &copySpots, lastFreeSpots, &copyScore, lastScore);

        // daca este cazul actualizeaza matricea
        if(copySpots >= bestSpots && actionMade)
        {
            changeBestMove(copyNr, bestMove, &bestScore, 
                                &bestSpots, &copyScore, &copySpots);
        }
    }

    copyNumbersMatrix(numbers, bestMove);
    *score = bestScore;
    *freeSpots = bestSpots; 

}

// afiseaza high-scoreurile user-ului
void printStats()
{
    WINDOW *window = initscr();
    WINDOW *boxWindow = initMiddleBox(window, BOX_HEIGHT,
                                         BOX_WIDTH, BOX_OFFSET);
    box(boxWindow, 0, 0);

    clear();
    refresh();
    wrefresh(boxWindow);

    FILE *fptr = fopen("data.txt", "r");
    int nr, i = 0;

    printWordStart(boxWindow, -5, -7, "HIGH SCORES");

    while(fscanf(fptr, "%d", &nr) == 1)
    {
        if(i <= 6 && nr > 0)
        {
            printWordStartInt(boxWindow, 0, -5 + i, nr);
            i += 2;
        }
    }

    int textOffsetY = 2, textOffsetX = -14;
    mvwprintw(window, getWindowMiddleY(window) + BOX_HEIGHT / 2 + textOffsetY,
         getWindowMiddleX(window) + textOffsetX, "Pentru a iesi - orice tasta");

    wrefresh(window);
    wrefresh(boxWindow);
    fclose(fptr);
    timeout(50000);
    getch();


}

// functionalitatea start screen-ului
void startScreen(game_state *gameStateMachine, int **numbers, 
                    int *freeSpots, int *score)
{
    // initializare
    WINDOW *window = initscr();
    curs_set(0);
    int index = -1;

    WINDOW *boxWindow = initMiddleBox(window, BOX_HEIGHT, BOX_WIDTH, BOX_OFFSET);
    box(boxWindow, 0, 0);

    // afisarea textului
    printStartScreenOptions(boxWindow);
    printHighlight(boxWindow, &index, 1);
    wrefresh(boxWindow);

    while(*gameStateMachine == START_SCREEN || *gameStateMachine == PAUSE_SCREEN)
    {
        int choice = wgetch(boxWindow);
        choice = tolower(choice);

        // verificarea alegerii
        switch(choice)
        {
            case 'q':
                *gameStateMachine = QUIT;
                break;
            case KEY_UP:
                printHighlight(boxWindow, &index, -1);
                break;
            case KEY_DOWN:
                printHighlight(boxWindow, &index, 1);
                break;
            case 10:
                if(index == 0)
                {
                    *score = 0;
                    initNumbers(numbers, freeSpots);
                    updateNumbers(numbers, *freeSpots, *score);
                    *gameStateMachine = PLAYING;
                }
                else if(index == 1) 
                {
                    copyNumbers(numbers, freeSpots, score);
                    *gameStateMachine = PLAYING;
                }
                else
                    *gameStateMachine = QUIT;
                break;
        }
        wrefresh(boxWindow);
    }
}

// functionalitatea end screen-ului
void endScreen(game_state *gameStateMachine, int **numbers,
                    int *freeSpots, int *score, int *maxNr)
{
    // initializarea ferestrelor
    WINDOW *window = initscr();
    int index = -1;

    WINDOW *boxWindow = initMiddleBox(window, BOX_HEIGHT, BOX_WIDTH, BOX_OFFSET);
    box(boxWindow, 0, 0);

    // afisarea textului
    printEndScreenOptions(boxWindow, *maxNr);
    printHighlightEnd(boxWindow, *maxNr, &index, 1);

    clear();
    refresh();
    wrefresh(boxWindow);

    updateData(*score);

    while(*gameStateMachine == END_SCREEN)
    {
        int choice = wgetch(boxWindow);
        choice = tolower(choice);

        // verificarea alegerii
        switch(choice)
        {
            case 'q':
                *gameStateMachine = QUIT;
                break;
            case KEY_UP:
                printHighlightEnd(boxWindow, *maxNr, &index, -1);
                break;
            case KEY_DOWN:
                printHighlightEnd(boxWindow, *maxNr, &index, 1);
                break;
            case 10:
                if(index == 0)
                {
                    *score = 0;
                    *maxNr = 0;
                    initNumbers(numbers, freeSpots);
                    updateNumbers(numbers, *freeSpots, *score);
                    *gameStateMachine = PLAYING;
                }
                else if(index == 1) 
                {
                    printStats();
                    clear();
                    refresh();
                    printHighlightEnd(boxWindow, *maxNr, &index, -1);
                }

                else
                    *gameStateMachine = QUIT;
                break;
        }
        wrefresh(boxWindow);
    }
}

int main()
{
    // initializari
    int **numbers = initMatrix(), **lastNumbers = initMatrix();
    int lastFreeSpots, freeSpots = ROW_NR * COL_NR;
    int score = 0, lastScore = 0, maxNr = 2;
    game_state gameStateMachine = START_SCREEN;

    curs_set(0);
    srand(time(0));

    // intrare in start screen state
    startScreen(&gameStateMachine, numbers, &freeSpots, &score);

    // intializare ferestre de main game
    WINDOW *window = initscr();
    WINDOW *boxWindow = initMiddleBox(window, BOX_HEIGHT, BOX_WIDTH, BOX_OFFSET);

    start_color();
    initColors();

    // verificare daca terminalul are culori
    if (!has_colors()) 
    {
        endwin();
        printf("Terminalul tău nu suportă culori.\n");
        return 1;
    }

    checkFutureMove(numbers, lastNumbers, &freeSpots, 
                        &lastFreeSpots, &score, &lastScore);  

    //printarea UI-ului
    printInstructions(window, score);
    
    printBox(boxWindow, numbers);
    wrefresh(window);
    wrefresh(boxWindow); 

    while(LOOP && gameStateMachine == PLAYING)
    {
        timeout(MOVE_DELAY);
        int choice = getch();

        // daca user-ul n-a facut nicio mutare in MOVE_DELAY secunde
        if(choice == ERR) 
        {
            doBestMove(numbers, lastNumbers, &freeSpots, 
                        &lastFreeSpots, &score, &lastScore);
        }

        //verificare daca s-a terminat jocul
        if((!freeSpots && checkLose(numbers, lastNumbers, &freeSpots, 
                        &lastFreeSpots, &score, &lastScore)) || maxNr == 2048)
        {
            gameStateMachine = END_SCREEN;
            endScreen(&gameStateMachine, numbers, &freeSpots, &score, &maxNr);
        }

        choice = tolower(choice);

        // verificarea alegerii
        switch(choice)
        {
            case 'q':
                gameStateMachine = PAUSE_SCREEN;
                startScreen(&gameStateMachine, numbers, &freeSpots, &score);
                break;

            case 'w':
                swipeAction(swipeUp, numbers, lastNumbers, &freeSpots, 
                                &lastFreeSpots, &score, &lastScore);
                updateMax(&gameStateMachine, numbers, &maxNr);
                break;

            case 's':
                swipeAction(swipeDown, numbers, lastNumbers, &freeSpots, 
                                &lastFreeSpots, &score, &lastScore);
                updateMax(&gameStateMachine, numbers, &maxNr);
                break;

            case 'a':
                swipeAction(swipeLeft, numbers, lastNumbers, &freeSpots, 
                                &lastFreeSpots, &score, &lastScore);
                updateMax(&gameStateMachine, numbers, &maxNr);
                break;
                
            case 'd':
                swipeAction(swipeRight, numbers, lastNumbers, &freeSpots, 
                                &lastFreeSpots, &score, &lastScore);
                updateMax(&gameStateMachine, numbers, &maxNr);
                break;

            case 'u':
                undoNumbers(numbers, lastNumbers, &freeSpots, 
                                lastFreeSpots, &score, lastScore);
                break;
        }
    
        // refresh la ecran cu noile informatii
        wclear(boxWindow);
        printInstructions(window, score);
        printBox(boxWindow, numbers);
        
        checkFutureMove(numbers, lastNumbers, &freeSpots, 
                            &lastFreeSpots, &score, &lastScore);
        wrefresh(window);
        wrefresh(boxWindow); 
    }
    endwin();

    return 0;
}