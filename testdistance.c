/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mazes.h"

/* This uses two non-random "maze" structures for test.
 *
 * First a serpentine path, for testing the
 * worst case of a path from point A to B.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |                                     A |
 * +   +---+---+---+---+---+---+---+---+---+
 * |                                       |
 * +---+---+---+---+---+---+---+---+---+   +
 * |                                       |
 * +   +---+---+---+---+---+---+---+---+---+
 * |                                     B |
 * +---+---+---+---+---+---+---+---+---+---+
 *
 * Second, other direction worst case, hollow with no walls at all.
 * Example maze:
 * +---+---+---+---+---+---+---+
 * |                           |
 * +   +   +   +   +   +   +   +
 * |                           |
 * +   +   +   +   +   +   +   +
 * |                           |
 * +   +   +   +   +   +   +   +
 * |                           |
 * +---+---+---+---+---+---+---+
 */

int
printpath(TRAIL *walk, int max)
{
  int steps = 0;
  if(!walk) { return 1; }
  
  do {
    printf("step %d on id(%d); ", steps++, walk->cell_id);
    if(steps == max) {
      printf("\nRunaway\n");
      return 2;
    }
  } while( (walk = walk->next) );

  printf(" done\n");
  return 0;
}

int
main(int notused, char**ignored)
{
  GRID *g;
  CELL *c;
  DMAP *dm;
  char *board;
  int distance;
  int rc;

  g = creategrid(10,10,1);
  if(!g) {
    printf("Create serpentine grid failed.\n");
    return 1;
  }
  iterategrid(g, serpentine, NULL);

  namebyid(g,  9, " A");
  namebyid(g, 99, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,9) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return 1;
  }

  distance = distanceto(dm, visitid(g,99), 1);
  ascii_dmap(dm);

  if(distance != 99) {
    printf("Find distance failed %d\n", distance);
    return 1;
  }
  printf("Distance is correctly %d\n", distance);

  rc = findpath(dm);
  if( rc != 0 ) {
    printf("Find path failed %d\n", rc);
    return 1;
  }
  rc = printpath(dm->path, 101);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return 1;
  }

  freedistancemap(dm);
  freegrid(g);

  g = creategrid(5,10,1);
  if(!g) {
    printf("Create hollow grid failed.\n");
    return 2;
  }
  iterategrid(g, hollow, NULL);
  namebyid(g,  0, " A");
  namebyid(g, g->max - 1, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return 2;
  }

  distance = distanceto(dm, visitid(g,g->max - 1), 1);
  ascii_dmap(dm);

  /* 12 because (width+height-2) when no diagonal movement  */
  if(distance != g->cols + g->rows -2) {
    printf("Find distance failed %d\n", distance);
    return 2;
  }
  printf("Distance is correctly %d\n", distance);

  rc = findpath(dm);
  if( rc != 0 ) {
    printf("Find path failed %d\n", rc);
    return 2;
  }
  rc = printpath(dm->path, distance+2);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return 2;
  }

  freedistancemap(dm);
  freegrid(g);

  g = creategrid(3,3,1);
  if(!g) {
    printf("Create pathless grid failed.\n");
    return 3;
  }
  namebyid(g,  0, " A");
  namebyid(g, g->max - 1, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return 3;
  }

  distance = distanceto(dm, visitid(g,g->max - 1), 1);
  ascii_dmap(dm);

  if(distance != DISTANCE_ERROR) {
    printf("Find distance failed %d\n", distance);
    return 3;
  }
  printf("Distance correctly failed with %d\n", distance);

  rc = findpath(dm);
  if( rc == 0 ) {
    printf("findpath should have failed %d\n", rc);
    return 3;
  }
  printf("findpath correctly failed with %d\n", rc);

  freedistancemap(dm);

  iterategrid(g, serpentine, NULL);
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  distance = distanceto(dm, visitid(g,g->max - 1), 0);
  ascii_dmap(dm);

  if(dm->farthest != 6) {
    printf("find farthest got wrong answer: %d\n", dm->farthest);
    return 4;
  }
  printf("find farthest got to %d, distance %d\n",
  	dm->farthest_id, dm->farthest);
  freedistancemap(dm);

  dm = findlongestpath(g);
  if(!dm) {
    printf("findlongestpath failed\n");
    return 4;
  }
  printf("findlongestpath found:\n");
  ascii_dmap(dm);
  rc = printpath(dm->path, 10);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return 4;
  }
  if((dm->farthest != 8) || (dm->farthest_id != 2)) {
    printf("incorrect longest path\n");
    return 4;
  }
  printf("a longest possible path found\n");
  
  namepath(dm, "STA", NULL, "END");
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  c = visitid(g,1);
  if(!c) {
    printf("something broke with the grid\n");
    return 4;
  }
  if(strncmp("7", c->name, 1)) {
    printf("unexpected name on cell %d: %s\n", c->id, c->name);
    return 4;
  }
  c = visitid(g,2);
  if(!c) {
    printf("something broke with the grid\n");
    return 4;
  }
  if(strncmp("END", c->name, 4)) {
    printf("unexpected name on cell %d: %s\n", c->id, c->name);
    return 4;
  }
  printf("naming iterator works\n");

  freedistancemap(dm);
  freegrid(g);

  printf("\nCreating micro grid to test path.\n");
  g = creategrid(1,1,3);
  if(!g) {
    printf("Create micro grid failed.\n");
    return 5;
  }
  dm = findlongestpath(g);
  if(!dm) {
    printf("findlongestpath microgrid failed\n");
    return 5;
  }
  rc = printpath(dm->path, 2);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return 4;
  }
  if(dm->path->cell_id != 0) {
    printf("findlongestpath microgrid didn't work\n");
  }
  printf("findlongestpath microgrid worked\n");

  freedistancemap(dm);
  freegrid(g);
  return 0;
}
