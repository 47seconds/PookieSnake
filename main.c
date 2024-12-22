#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_BLACK 0x00000000
#define COLOR_WHITE 0xffffffff
#define COLOR_GRAY 0x0f0f0f0f
#define CELL_SIZE 30
#define SIMULATION_SPEED 50

int SCREEN_WIDTH = WIDTH;
int SCREEN_HEIGHT = HEIGHT;
int X_OFFSET;
int Y_OFFSET;
int NW;
int NH;

typedef struct Coords {
  int x;
  int y;
} Coords;

typedef struct GridCoords {
  int r;
  int c;
  struct GridCoords* next;
} GridCoords;

typedef struct Snake {
  GridCoords* st;
  GridCoords* top;
} Snake;

GridCoords* newGridCoords(int r, int c) {
  GridCoords* gc = (GridCoords*)malloc(sizeof(GridCoords));
  gc->r = r;
  gc->c = c;
  gc->next = NULL;
  return gc;
}

Snake* createSnake() {
  Snake* snake = (Snake*)malloc(sizeof(Snake));
  snake->top = NULL;
  snake->st = NULL;
  return snake;
}

void pushCoords(Snake* snake, GridCoords* gc) {
  if (!snake->st) snake->st = gc;
  if (!snake->top) snake->top = gc;
  else {
    snake->top->next = gc;
    snake->top = gc;
  }
}

void popGridCoords(Snake* snake) {
  GridCoords* temp = snake->st;
  snake->st = snake->st->next;
  free(temp);
}

void setRendererDrawColor(SDL_Renderer* renderer, Uint32 hexcolor) {
  Uint8 r = (hexcolor >> 24) & 0xff;
  Uint8 g = (hexcolor >> 16) & 0xff;
  Uint8 b = (hexcolor >> 8) & 0xff;
  Uint8 a = hexcolor & 0xff;

  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

GridCoords* coordsToGridCoords(Coords* c) {
  GridCoords* gc = (GridCoords*)malloc(sizeof(GridCoords));
  gc->c = ((c->x - X_OFFSET) / CELL_SIZE) + 1;
  gc->r = ((c->y - Y_OFFSET) / CELL_SIZE) + 1;
  gc->next = NULL;
  return gc;
}

Coords* gridCoordsToCoords(GridCoords* gc) {
  Coords* c = (Coords*)malloc(sizeof(Coords));
  c->x = (gc->c * CELL_SIZE) + X_OFFSET;
  c->y = (gc->r * CELL_SIZE) + Y_OFFSET;
  return c;
}

SDL_Texture* createStaticStage(SDL_Renderer* renderer, int NW, int NH, int X_OFFSET, int Y_OFFSET) {
  SDL_Texture* stageTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_SetRenderTarget(renderer, stageTexture);
  
  setRendererDrawColor(renderer, COLOR_GRAY);
  for (int i = 0; i <= NW; i++) SDL_RenderDrawLine(renderer, X_OFFSET + i * CELL_SIZE, Y_OFFSET, X_OFFSET + i * CELL_SIZE, SCREEN_HEIGHT - Y_OFFSET);
  for (int i = 0; i <= NH; i++) SDL_RenderDrawLine(renderer, X_OFFSET, Y_OFFSET + i * CELL_SIZE, SCREEN_WIDTH - X_OFFSET, i * CELL_SIZE + Y_OFFSET);

  SDL_SetRenderTarget(renderer, NULL);
  return stageTexture;
}

void drawCell(SDL_Renderer* renderer, GridCoords* gc) {
  Coords* c = gridCoordsToCoords(gc);
  SDL_Rect rect = (SDL_Rect){c->x, c->y, CELL_SIZE, CELL_SIZE};
  SDL_RenderDrawRect(renderer, &rect);
  SDL_RenderFillRect(renderer, &rect);
  free(c);
}

void drawGame(Snake* snake, GridCoords* food, SDL_Renderer* renderer) {
  setRendererDrawColor(renderer, COLOR_WHITE);
  drawCell(renderer, food);
  
  GridCoords* tr = snake->st;
  while(tr) {
    drawCell(renderer, tr);
    tr = tr->next;
  }
}

void spawnFood(GridCoords* food) {
  food->r = rand() % NH;
  food->c = rand() % NW;
}

int collisionDetection(Snake* snake, GridCoords* nextCell) {
  GridCoords* tr = snake->st;

  while (tr) {
    if (tr->r == nextCell->r && tr->c == nextCell->c) return 1;
    tr = tr->next;
  }

  return 0;
}

void nextIteration(Snake* snake, int dir, GridCoords* food, int* foodCaptured, int* game_over) {
  int r, c;
  
  switch (dir) {
    case 1:
      c = snake->top->c;
      r = snake->top->r - 1;
      break;

    case 2:
      c = snake->top->c + 1;
      r = snake->top->r;
      break;
    
    case 3:
      c = snake->top->c;
      r = snake->top->r + 1;
      break;
    
    case 4:
      c = snake->top->c - 1;
      r = snake->top->r;
      break;
  }

  if (r == food->r && c == food->c) *foodCaptured = 1;
  
  GridCoords* newCell = newGridCoords(r, c);
  if (collisionDetection(snake, newCell)) *game_over = 1;
  if (!(*game_over)) pushCoords(snake, newCell);
  if (!(*foodCaptured)) popGridCoords(snake);
}

int checkGameOver(Snake* snake) {
  if (snake->top->c >= NW || snake->top->c < 0 || snake->top->r >= NH || snake->top->r < 0) return 1;
  return 0;
}

void deleteGame(Snake* snake) {
  GridCoords* tr = snake->st;

  while (tr) {
    GridCoords* temp = tr;
    tr = tr->next;
    free(temp);
  }

  free(tr);
  free(snake);
}

int main (int argc, char **argv) {
  
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Wsnake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  setRendererDrawColor(renderer, COLOR_BLACK);
  SDL_RenderClear(renderer);

  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

  SDL_DisplayMode display;
  SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
  SDL_RenderPresent(renderer);

  X_OFFSET = (SCREEN_WIDTH - (SCREEN_WIDTH/CELL_SIZE) * CELL_SIZE) / 2;
  Y_OFFSET = (SCREEN_HEIGHT - (SCREEN_HEIGHT/CELL_SIZE) * CELL_SIZE) / 2;
  NW = (SCREEN_WIDTH - X_OFFSET) / CELL_SIZE;
  NH = (SCREEN_HEIGHT - Y_OFFSET) / CELL_SIZE;

  SDL_Texture* stageTexture = createStaticStage(renderer, NW, NH, X_OFFSET, Y_OFFSET);
  SDL_RenderCopy(renderer, stageTexture, NULL, NULL);

  Snake* snake = createSnake();
  GridCoords* start = newGridCoords(0 ,0);
  pushCoords(snake, start);
  GridCoords* food = (GridCoords*)malloc(sizeof(GridCoords));
  spawnFood(food);

  drawGame(snake, food, renderer);
  SDL_RenderPresent(renderer);

  SDL_Event event;
  int running = 1;
  int pause = 1;
  int game_over = 0;
  int dir = 0;
  int foodCaptured = 0;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = 0;
          break;
        
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              running = 0;
              break;

            case SDLK_SPACE:
              if (!game_over) pause = !pause;
              break;
            
            case SDLK_RIGHT:
              if (dir != 4) dir = 2;
              if (pause && !game_over) pause = 0;
              break;

            case SDLK_LEFT:
              if (dir != 2) dir = 4;
              if (pause && !game_over) pause = 0;
              break;

            case SDLK_UP:
              if (dir != 3) dir = 1;
              if (pause && !game_over) pause = 0;
              break;

            case SDLK_DOWN:
              if (dir != 1) dir = 3;
              if (pause && !game_over) pause = 0;
              break;
          }
      }
    }

    if (pause) {
      SDL_Delay(SIMULATION_SPEED);
      continue;
    }
    
    if (game_over) pause = 1;

    SDL_RenderCopy(renderer, stageTexture, NULL, NULL);

    if (foodCaptured) {
      spawnFood(food);
      foodCaptured = 0;
    }
    nextIteration(snake, dir, food, &foodCaptured, &game_over);
    
    if (checkGameOver(snake)) {
      game_over = 1;
      continue;
    }
    
    drawGame(snake, food, renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(SIMULATION_SPEED);
  }
  
  deleteGame(snake);
  free(food);

  SDL_DestroyTexture(stageTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
