/* This is the luser.droog mazer modified to use my maze generator.
 * It's now semi-playable in that it will time how long it takes you
 * to reach the far corner. Arrow keys still move, and 'q' will quit.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_rect.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "SDL_image.h"

#define SPRITE "sprite.pbm"

#include "mazes.h"

enum {
  dimx = 25, dimy = 25, 
  cellw = 20, cellh = 20,
  width = dimx*cellw, height = dimy*cellh
};

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Rect rect;
SDL_Surface *surface;
SDL_Surface *rgbsurface;
SDL_Texture *texture;
int playerx, playery;
//char board[dimx][dimy];
//enum { OCCUPIED = 1, NOEAST = 2, NOSOUTH = 4 };
int quit;
int won;
time_t start;

Uint32 Black(){ return SDL_SetRenderDrawColor( renderer, 0,   0,   0,   255 ); }
Uint32 White(){ return SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 ); }
Uint32 Red(){   return SDL_SetRenderDrawColor( renderer, 255, 0,   0,   255 ); }
Uint32 Green(){ return SDL_SetRenderDrawColor( renderer, 0,   255, 0,   255 ); }
Uint32 Blue(){  return SDL_SetRenderDrawColor( renderer, 0,   0,   255, 255 ); }

Uint32 MyColor(){return SDL_SetRenderDrawColor(renderer, 50,  180, 20,  255 ); }

int error( char *msg );
int sdl_error( char *msg );
void set_up( void );
void shut_down( int exitcode );
void drawbg( Uint32 (*bg)() );
void show( void );
int loop( void );

//void clearmaze( void );
//void buildmaze( int skip );
GRID *board;

void draw( Uint32 (*fg)() );

int main(){
  start = time(NULL);
  won = 0;
  set_up();
  draw( MyColor );
  shut_down( loop() );
}


int error( char *msg ){
  fprintf( stderr, "%s\n", msg );
  shut_down( EXIT_FAILURE );
  return 0;
}
int sdl_error( char *msg ){
  fprintf( stderr, "%s: %s\n", msg, SDL_GetError() );
  shut_down( EXIT_FAILURE );
  return 0;
}

void set_up( void ){
  SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) == 0
  	|| sdl_error( "SDL_Init failed" );

  ( SDL_CreateWindowAndRenderer( width, height, 0, &window, &renderer ) == 0 )
  	|| sdl_error( "SDL_CreateWindowAndRenderer failed" );

  int flags = 0;

  ( IMG_Init( flags ) == flags ) || sdl_error( "IMG_init failed" );

  ( surface = IMG_Load( SPRITE ) ) || sdl_error( "IMG_Load failed" );

  ( rgbsurface = SDL_ConvertSurfaceFormat( surface, SDL_GetWindowPixelFormat( window ), 0 ) ) 
    || sdl_error( "SDL_ConvertSurfaceFormat failed" );

  ( texture = SDL_CreateTextureFromSurface( renderer, rgbsurface ) )
    || sdl_error( "SDL_CreateTextureFromSurface failed" );

  //buildmaze( 0 );
  board = creategrid( dimx, dimy, UNVISITED);
  aldbro( board );
  drawbg( Black );
  show();
}

void shut_down( int exitcode ){
  SDL_DestroyWindow( window );
  SDL_DestroyRenderer( renderer );
  SDL_FreeSurface( surface );
  SDL_FreeSurface( rgbsurface );
  SDL_DestroyTexture( texture );
  IMG_Quit();
  SDL_Quit();
  if(won) { printf("Woo-Hoo, finished it!\n"); }
  printf("Took %d seconds\n", (int)(time(NULL) - start));
  exit( exitcode );
}


void drawbg( Uint32 (*bg)() ){
  bg();
  SDL_RenderClear( renderer );
}


void show( void ){
  SDL_RenderPresent( renderer );
}

//static int east( int x, int y ){
//  return !(board[x][y]&NOEAST);
//}
//static int south( int x, int y ){
//  return !(board[x][y]&NOSOUTH);
//}
//static int west( int x, int y ){
//  return x==0 || east( x-1, y );
//}
//static int north( int x, int y ){
//  return x==0 || south( x, y-1 );
//}

/* booleans returning   ~ can you go that way ~ */
static int east ( int x, int y ){
   return (NC != isconnectedbyrc(board, x,y, x+1,y, ANY));
}
static int south ( int x, int y ){
   return (NC != isconnectedbyrc(board, x,y, x,y+1, ANY));
}
static int west ( int x, int y ){
   return (NC != isconnectedbyrc(board, x,y, x-1,y, ANY));
}
static int north ( int x, int y ){
   return (NC != isconnectedbyrc(board, x,y, x,y-1, ANY));
}

static void move_player( SDL_Event *pevent ){
  switch( pevent->key.keysym.sym ){
  case SDLK_DOWN:  if( playery < dimy-1 && south( playerx, playery ) ) ++playery; break;
  case SDLK_UP:    if( playery > 0      && north( playerx, playery ) ) --playery; break;
  case SDLK_RIGHT: if( playerx < dimx-1 && east( playerx, playery )  ) ++playerx; break;
  case SDLK_LEFT:  if( playerx > 0      && west( playerx, playery )  ) --playerx; break;
  case SDLK_q:  quit++; break;
  }
}

int loop( void ){
  //SDL_Delay( 5000 );

  while( ! quit ){
    SDL_Event event;
    if ( SDL_PollEvent( &event ) ){
      switch( event.type ){
      case SDL_QUIT: ++quit; break;
      case SDL_KEYDOWN: move_player( &event ); break;
      }
      draw( MyColor );
    } else {
      SDL_Delay( 250 );
    }
  }  
  return 0;
}


//void clearmaze( void ){
//  for( int i = 0; i < 50; ++i )
//    for( int j = 0; j < 50; ++j )
//      board[i][j] = 0;
//}
//
//void buildmaze( int skip ){
//  srand( time(NULL) );
//  int x = rand() % dimx;
//  int y = rand() % dimy;
//  playerx = x;
//  playery = y;
//  enum { RIGHT, DOWN, LEFT, UP } dir;
//  
//  for(  int i = 0; i < 400+skip; ++i ){
//    //printf( "%d, %d\n", x, y );
//    board[x][y] |= OCCUPIED;
//    dir = rand() % 4;
//    switch( dir ){
//    case RIGHT: if( x<(50-1) ){ board[  x++][  y  ] |= NOEAST;  break; } /*fallthrough*/
//    case DOWN:  if( y<(50-1) ){ board[  x  ][  y++] |= NOSOUTH; break; } /*fallthrough*/
//    case LEFT:  if( x>0      ){ board[--x  ][  y  ] |= NOEAST;  break; } /*fallthrough*/
//    case UP:    if( y>0      ){ board[  x  ][--y  ] |= NOSOUTH; break; }
//                if( x<(50-1) ){ board[  x++][  y  ] |= NOEAST;  break; }
//                if( y<(50-1) ){ board[  x  ][  y++] |= NOSOUTH; break; }
//                if( x>0      ){ board[--x  ][  y  ] |= NOEAST;  break; } /*fallthrough*/
//    }
//  }
//}

static void draweastwall( int x, int y ){
  if( !east( x, y ) ) 
  	SDL_RenderDrawLine( renderer, (x+1)*cellw, y*cellh,
					(x+1)*cellw, (y+1)*cellh );
}

static void drawsouthwall( int x, int y ){
  if( !south( x, y ) ) 
  	SDL_RenderDrawLine( renderer, x*cellw, (y+1)*cellh, 
					(x+1)*cellw, (y+1)*cellh );
}

//static void drawbulk( int x, int y ){
//  if( east( x, y ) && south( x, y ) && west( x, y ) && north( x, y ) )
//    SDL_RenderFillRect( renderer, &rect );
//}

static void drawmaze( void ){
  rect.w = cellw;
  rect.h = cellh;
  rect.x = 0;
  for( int i = 0; i < dimx; ++i, rect.x += cellw ){
    rect.y = 0;
    for( int j = 0; j < dimy; ++j, rect.y += cellh ){
      draweastwall( i, j );
      drawsouthwall( i, j );
      //drawbulk( i, j );
    }
  }
}

static void drawplayer( void ){
  rect.w = cellw - 2;
  rect.h = cellh - 2;
  rect.x = playerx*cellw + 1;
  rect.y = playery*cellh + 1;
  SDL_RenderCopy( renderer, texture, NULL, &rect );

  if ((playerx + 1 == dimx) && (playery + 1 == dimy)) {
    won = quit = 1;
  }
}

void draw( Uint32 (*fg)() ){
  drawbg( Black );
  fg();
  drawmaze();
  drawplayer();
  show();
}

