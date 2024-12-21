#include <SDL2/SDL.h>
#include <stdlib.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_BLACK 0x00000000
#define COLOR_WHITE 0xffffffff
#define COLOR_GRAY 0x0f0f0f0f
#define CELL_SIZE 50

int SCREEN_WIDTH = WIDTH;
int SCREEN_HEIGHT = HEIGHT;

typedef struct Coords {
  int x;
  int y;
  struct Coords* next;
} Coords;

typedef struct Snake {
  Coords* st;
  Coords* top;
} Snake;

Snake* createSnake() {
  Snake* snake = (Snake*)malloc(sizeof(Snake));
  snake->top = NULL;
  snake->st = NULL;
  return snake;
}

void pushCoords(Snake* snake, int x, int y) {
  Coords* newCoords = (Coords*)malloc(sizeof(Coords));
  newCoords->x = x;
  newCoords->y = y;
  newCoords->next = NULL;
  
  if (!snake->st) snake->st = newCoords;
  if (!snake->top) snake->top = newCoords;
  else {
    snake->top->next = newCoords;
    snake->top = newCoords;
  }
}

void popCoords(Snake* snake) {
  Coords* temp = snake->st;
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

SDL_Texture* createStaticStage(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) {
  SDL_Texture* stageTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_SetRenderTarget(renderer, stageTexture);
  
  setRendererDrawColor(renderer, COLOR_GRAY);
  for (int i = 0; i <= SCREEN_WIDTH/CELL_SIZE; i++) SDL_RenderDrawLine(renderer, i * CELL_SIZE, 0, i * CELL_SIZE, SCREEN_HEIGHT);
  for (int i = 0; i <= SCREEN_HEIGHT/CELL_SIZE; i++) SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, SCREEN_WIDTH, i * CELL_SIZE);

  SDL_SetRenderTarget(renderer, NULL);
  return stageTexture;
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

  SDL_Texture* stageTexture = createStaticStage(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_RenderCopy(renderer, stageTexture, NULL, NULL);
  
  SDL_RenderPresent(renderer);

  SDL_Rect food = (SDL_Rect){0, 0, CELL_SIZE, CELL_SIZE};
  Snake* snake = createSnake();
  pushCoords(snake, 0, 0);

  SDL_Event event;
  int running = 1;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
        running = 0;
        
        case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
            running = 0;
          }
      }
    }

    
  }

  SDL_DestroyTexture(stageTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
