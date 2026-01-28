    #include <SDL.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <time.h>

    #ifdef _WIN32
    #include <windows.h>
    #endif

    #define Region_CountMax 100
    #define Color_Count 4
    #define Cell_Size 2
    #define WINDOW_TITLE "Four Color Theorem"
    #define HALLOFFAME "hall_of_fame.txt"

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    typedef enum {
        Easy,
        Medium,
        Hard,
    }Difficulty;

    const int DIFF_REGION_COUNTS[] = {
        5,
        70,
        100
    };

    typedef enum {
        Menu,
        Game
    }GameState;

    typedef struct {
        SDL_Point point; // center of the regions as a dot
        int colorIndex;
    }Region;

    struct Game {
        SDL_Window *window;
        SDL_Renderer *renderer;

        Region *regions;

        int chosenColor;
        int regionCount;

        Difficulty difficulty;
        GameState gameState;

        bool **adjucency;

        Uint32 startTimer;
        Uint32 finishTimer;

        bool winState;
    };

    bool sdl_initialise(struct Game *game);
    bool conflictCheck(const struct  Game *game, int regionIndex);
    bool winCheck(const struct  Game *game);
    void game_cleanup(const struct Game *game);
    void game_renderer(const struct Game *game);
    void mouse_input(const struct Game *game, int x, int y);
    void regionsGenerator(const struct Game *game);
    void menu_renderer(const struct Game* game);
    void adjucencyCheck(const struct Game *game);
    void resultSave(const char *name, Uint32 time);
    void hallOfFamePrint(void);
    static int find_closest_region(const struct Game *game, int x, int y);
    void setDifficulty(struct Game *game, Difficulty diff);
    static int sq2(int x, int y);

    const SDL_Color RGB_palette[Color_Count] = {
        {255,   0,   0, 255}, //RED
        {  0, 255,   0, 255}, //GREEN
        {  0,   0, 255, 255}, //BLUE
        {255, 255,   0, 255} //YELLOW
    };

    //pythagoras square for finding the distance between the cell and the center of the cell
    static int sq2(int const x, int const y)
    {
        return x * x + y * y;
    }

    int main(void) {

    #ifdef _WIN32
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONIN$",  "r", stdin);
    #endif

        srand((unsigned)time(NULL));

        struct Game game = {
            .window = NULL,
            .renderer = NULL,
            .regionCount = 0,
            .chosenColor =0,
            .difficulty = Medium,
            .gameState = Menu,
            .startTimer = 0,
            .finishTimer = 0,
            .winState =  false,

        };

        if (sdl_initialise(&game)) {
            game_cleanup(&game);
            printf("All bad!");
            return EXIT_FAILURE;
        }

        bool isRunning = true;

        printf("Select difficulty: 1 - Easy, 2 - Medium, 3 - Hard\n");

        while (isRunning) {
            SDL_Event e;

            //Poll event for all the control keys in application
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT)
                    isRunning = false;

                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                        isRunning = false;

                    //if menu
                    if (game.gameState == Menu) {
                        if (e.key.keysym.scancode == SDL_SCANCODE_1) {
                            setDifficulty(&game, Easy);
                            game.gameState = Game;
                        }      else if (e.key.keysym.scancode == SDL_SCANCODE_2) {
                            setDifficulty(&game, Medium);
                            game.gameState = Game;
                        }           else if (e.key.keysym.scancode == SDL_SCANCODE_3) {
                            setDifficulty(&game, Hard);
                            game.gameState = Game;
                        }
                    }

                    else if (game.gameState == Game)
                    {
                        if (e.key.keysym.scancode == SDL_SCANCODE_1) game.chosenColor = 0;
                            else if (e.key.keysym.scancode == SDL_SCANCODE_2) game.chosenColor = 1;
                                else if (e.key.keysym.scancode == SDL_SCANCODE_3) game.chosenColor = 2;
                                    else if (e.key.keysym.scancode == SDL_SCANCODE_4) game.chosenColor = 3;
                                        else if (e.key.keysym.scancode == SDL_SCANCODE_R) setDifficulty(&game, game.difficulty);
                    }

                }

                if (game.gameState == Game &&
                   e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    mouse_input(&game, e.button.x, e.button.y);
                   }
            }

            if (game.gameState == Menu) {
                menu_renderer(&game);
            } else if (game.gameState == Game ) {
                game_renderer(&game);
            }

            //exit the main loop if the game state is considered as win
            if (game.gameState == Game && !game.winState) {
                if (winCheck(&game)) {
                    game.winState = true;


                    game.finishTimer = SDL_GetTicks() - game.startTimer;
                    printf("You win! Time: %.2f seconds\n ", game.finishTimer / 1000.0);
                    isRunning = false;
                }
            }

            SDL_Delay(16);
        }

        if (game.winState) {
            char name[101];

            printf ("Enter your name to save the result to the hall of fame: ");

            if (fgets(name, sizeof(name), stdin)) {

                size_t len = strlen(name);
                if (len > 0 && name[len - 1] == '\n')
                    name[len-1] = '\0';
                if (name[0] != '\0')
                    resultSave(name, game.finishTimer);
            }
        }

        printf("\nHall of Fame:\n");
        hallOfFamePrint();
        fflush(stdout);

        printf("\n\nPress ENTER to exit...");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }

        game_cleanup(&game);

        printf("All Good!");
        return EXIT_SUCCESS;
    }


    void resultSave(const char* name, const Uint32 time) {

        FILE *f = fopen(HALLOFFAME, "a");
        if (!f) {
            printf("Cannot open Hallf of fame file!\n");
            return;
        }

        fprintf(f, "%s %.2f\n", name, time / 1000.0f);
        fclose(f);
    }

    void hallOfFamePrint(void) {
        FILE *f = fopen(HALLOFFAME, "r");
        if (!f) {
            printf ("No previous records");
            return;
        }
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            printf("%s", line);
        }
        fclose(f);
    }


    //change difficulty level with region amount
    void setDifficulty(struct Game *game, Difficulty diff)
    {

        game->difficulty = diff;
        int count = DIFF_REGION_COUNTS[diff];
        if (count>Region_CountMax)
            count = Region_CountMax;

        game->regionCount=count;
        regionsGenerator(game);
        adjucencyCheck(game);
        game->chosenColor = 0;
        game->startTimer = SDL_GetTicks();
        game->winState = false;
        game->finishTimer  = 0;
    }

    //Quitting routine
    void game_cleanup(const struct Game *game) {
        if (game->adjucency) {
            for (int i = 0; i < Region_CountMax; ++i) {
                if (game->adjucency[i])
                    free(game->adjucency[i]);
            }
            free(game->adjucency);
        }

        if (game->regions)
            free(game->regions);

        SDL_DestroyRenderer(game->renderer);
        SDL_DestroyWindow(game->window);
        SDL_Quit();
    }

    /*mouse handling proccess for painting. The logic is the same as in comparing the dots in render,
     *but now we track in which area the mouse button was pressed and paint it with chosen color */
    void mouse_input(const struct Game *game, const int x, const int y) {
        const int closest = find_closest_region(game, x, y);
        game->regions[closest].colorIndex = game->chosenColor;
    }

    //place a dot somewhere on a plane randomly
    void regionsGenerator(const struct Game *game) {
        for (int i = 0; i < game->regionCount; ++i) {
            game->regions[i].point.x = rand() % SCREEN_WIDTH;
            game->regions[i].point.y = rand() % SCREEN_HEIGHT;

            game->regions[i].colorIndex = -1;
        }
    }

    void adjucencyCheck(const struct Game *game) {
        for (int i = 0; i < Region_CountMax; ++i) {
            for (int j = 0; j < Region_CountMax; ++j) {
                game->adjucency[i][j] = false;
            }
        }

        //iteration through entire map with step Cell Size
        for (int y = 0; y < SCREEN_HEIGHT; y+= Cell_Size) {
            for (int x = 0; x < SCREEN_WIDTH; x+= Cell_Size) {
                //center of the current region
                const int centerX = x + Cell_Size / 2;
                const int centerY = y + Cell_Size / 2;

                //find the region with minimum distance to point(centerX, centerY)
                const int c = find_closest_region(game,centerX, centerY);

                //right neighbour check
                if ( x + Cell_Size < SCREEN_WIDTH) {
                    //center of right neighbour
                    const int rightCenterX = centerX + Cell_Size;
                    const int rightCenterY = centerY;

                    //region
                    const int cRight = find_closest_region(game, rightCenterX, rightCenterY);

                    //if regions adre diff than they are adjusent
                    if (cRight != c) {
                        game->adjucency[c][cRight] = true;
                        game->adjucency[cRight][c] = true;
                        //A borders B, B borders A
                    }
                }

                //Center of the bottom region
                if (y + Cell_Size < SCREEN_HEIGHT) {
                    const int downCenterX = centerX;
                    const int downCenterY = centerY + Cell_Size;
                    const int cDown = find_closest_region(game, downCenterX, downCenterY);

                    //same proccess here
                    if (cDown != c) {
                        game->adjucency[c][cDown] = true;
                        game->adjucency[cDown][c] = true;
                    }
                }

            }
        }
    }

bool winCheck(const struct  Game *game) {
        for (int i = 0; i < game->regionCount; ++i) {

            if (game->regions[i].colorIndex < 0)
                return false;
            if (conflictCheck(game, i))
                return false;
        }

        return true;
    }


    bool conflictCheck(const struct Game *game, int regionIndex) {
        const int Color = game->regions[regionIndex].colorIndex;
        if (Color< 0 )
            return false;

        for (int i = 0; i < game->regionCount; ++i) {
            if (i == regionIndex)
                continue;

            if (game->adjucency[regionIndex][i]) {
                if (game->regions[i].colorIndex == Color)
                    return true;
            }
        }
        return false;
    }
        //voronoi diagram implementation
        static int find_closest_region(const struct Game *game, const int x, const int y) {
            int closest = 0;
            int bestDistance = 100000000; //any huge number to compare to real distances

            //looking at all regions defined and calculating distance from (cx, cy) to region center(point.x, point,y)
            for (int i = 0; i < game->regionCount; ++i) {
                const int distanceX = x - game->regions[i].point.x;
                const int distanceY = y - game->regions[i].point.y;

                const int dist2 = sq2(distanceX, distanceY);

                if (dist2 < bestDistance) {
                    bestDistance = dist2;
                    closest = i;//now this is the closest distance to points (cx,cy) with region index i
                }
            }
            return closest;
        }

        //render the game itself using voronoi diagrams
        void game_renderer(const struct Game *game) {
            //background
            SDL_SetRenderDrawColor(game->renderer, 20, 20, 20, 255);
            SDL_RenderClear(game->renderer);

            //iterating throught the "cells" with sizes CELL_SIZE*CELL_SIZE
            for (int y = 0; y < SCREEN_HEIGHT; y+= Cell_Size) {
                for (int x = 0; x < SCREEN_WIDTH; x+= Cell_Size) {

                    //center points of the cell by calculatin leftmost corner
                    const int centerX = x + Cell_Size/2;
                    const int centerY = y + Cell_Size/2;

                    const int closest = find_closest_region(game, centerX, centerY);

                    bool isBorder = false;

                    int closestRight = closest;
                    //if there still space find the center of the right region
                    if (x+Cell_Size<SCREEN_WIDTH) {
                        const int rightCenterX = centerX + Cell_Size;
                        const int rightCenterY = centerY;
                        closestRight = find_closest_region(game, rightCenterX, rightCenterY);
                    }

                    int closestBelow = closest;

                    //if there is still place find the center of the bottom region
                    if (y + Cell_Size < SCREEN_HEIGHT) {
                        const int belowCenterX = centerX;
                        const int belowCenterY = centerY + Cell_Size;
                        closestBelow = find_closest_region(game, belowCenterX, belowCenterY);

                    }

                    //if there some point where region is not closes to our origin point than this is a border
                    if (closestRight != closest || closestBelow != closest) {
                        isBorder = true;
                    }
                    SDL_Color color;

                    if (isBorder) {
                        color.r = 0;
                        color.g = 0;
                        color.b = 0;
                        color.a = 255;
                    } else {
                        const int colorI = game->regions[closest].colorIndex; // color index is a color number of the chosen color from 0 to 3.

                        if (colorI >= 0 && colorI < Color_Count) {
                            // if color index is from 0 to 3 than the region is already painted
                            color = RGB_palette[colorI];

                            if (conflictCheck(game,closest)) {
                                color.r = (color.r+255) / 2;
                                color.g = (color.g+255) / 2;
                                color.b = (color.b+255) / 2;
                            }
                        }
                        else { // if not the color is gray by default
                            color.r = 80;
                            color.g = 80;
                            color.b = 80;
                            color.a = 255;
                        }
                    }

                    SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);

                    //create the rectangle
                    SDL_Rect cellRect = { x, y, Cell_Size, Cell_Size };
                    SDL_RenderFillRect(game->renderer, &cellRect);
                }
            }

            //white dots for debugging purposes
            SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
            for (int i = 0; i < game->regionCount; ++i) {
                SDL_Rect r = { game->regions[i].point.x - 2, game->regions[i].point.y - 2, 4, 4 };
                SDL_RenderFillRect(game->renderer, &r);
            }

            //small palettes for user to see chosen color
            for (int i = 0; i < Color_Count; ++i) {
                const int paletteSize = 40;
                const int margin = 10;
                SDL_Rect rect = {margin + i * (paletteSize + margin), SCREEN_HEIGHT - paletteSize - margin,paletteSize, paletteSize};

                const SDL_Color color = RGB_palette[i];
                SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(game->renderer, &rect);

                //making the color selected rec visually intuitive
                if (i == game->chosenColor) {
                    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
                    SDL_RenderDrawRect(game->renderer, &rect);

                    SDL_Rect rr = { rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4 };
                    SDL_RenderDrawRect(game->renderer, &rr);
                } else {
                    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(game->renderer, &rect);
                }
            }

            SDL_RenderPresent(game->renderer);
        }

        //menu function with difficulty selection.
        void menu_renderer(const struct Game *game) {
            SDL_SetRenderDrawColor(game->renderer, 10, 10, 40, 255);
            SDL_RenderClear(game->renderer);


            const int btnW = 300;
            const int btnH = 60;
            const int spacing = 20;
            const int startY = 150;
            const int centerX = SCREEN_WIDTH / 2 - btnW / 2;

            const SDL_Rect easy   = { centerX, startY + 0 * (btnH + spacing), btnW, btnH };
            const SDL_Rect medium = { centerX, startY + 1 * (btnH + spacing), btnW, btnH };
            const SDL_Rect hard  = { centerX, startY + 2 * (btnH + spacing), btnW, btnH };

            SDL_SetRenderDrawColor(game->renderer, 50, 150, 50, 255);
            SDL_RenderFillRect(game->renderer, &easy);
            SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(game->renderer, &easy);

            SDL_SetRenderDrawColor(game->renderer, 200, 200, 50, 255);
            SDL_RenderFillRect(game->renderer, &medium);
            SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(game->renderer, &medium);

            SDL_SetRenderDrawColor(game->renderer, 150, 50, 50, 255);
            SDL_RenderFillRect(game->renderer, &hard);
            SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(game->renderer, &hard);

            SDL_RenderPresent(game->renderer);
        }



        bool sdl_initialise(struct Game *game) {
            if (SDL_Init(SDL_INIT_EVERYTHING)) {
                fprintf(stderr, "SDL could not initialize! SDL_Error:  %s\n", SDL_GetError());
                return true;
            }

            //Window creation
            game -> window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
            if (game->window == NULL) {
                fprintf(stderr, "Window could not be crated! SDL_Error: %s\n", SDL_GetError());
                return true;
            }

            //Renderer setup
            game -> renderer = SDL_CreateRenderer(game->window, -1, 0);
            if (game->renderer == NULL) {
                fprintf(stderr, "Error creating window! SDL_Error: %s\n", SDL_GetError());
                return true;
            }

            game  ->regions = (Region*)malloc(Region_CountMax * sizeof(Region));
             if (!game->regions) {
            fprintf(stderr, "Failed to allocate memory for regions\n");
            return true;
            }

            game -> adjucency = (bool**)malloc(Region_CountMax * sizeof(bool*));
            if (!game->adjucency) {
            fprintf(stderr, "Failed to allocate memory for adjacency rows\n");
            return true;
            }

            for (int i = 0; i < Region_CountMax; ++i)
            game->adjucency[i] = NULL;


            for (int i = 0; i < Region_CountMax; ++i) {
                game->adjucency[i] = (bool*)malloc(Region_CountMax * sizeof(bool));
            if (!game->adjucency[i]) {
                fprintf(stderr, "Failed to allocate memory for adjacency row %d\n", i);
                return true;
                 }
            }

            return false;
        }