/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* tools for a maze grid */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "grid.h"

/* initializes a cell to have no connections
 * of type t
 * with id id
 */
void
initcell(CELL *c, int t, int i, int j, int id)
{
  if(!c) {return;}

  /* calloc()ed whole grid  memset((void*)c, 0, sizeof(CELL)); */
  c->id = id;
  c->row = i;
  c->col = j;
  c->ctype = t;

  for(int d = 0; d < DIRECTIONS; d++) {
    c->dir[d] = NC;
  }

} /* initcell() */


/* frees pointers within a cell */
void
freecell(CELL *c)
{
  if(!c) {return;}

  if(c->name) { free(c->name); }
  if(c->data) { free(c->data); }
} /* freecell() */

GRID*
creategrid(int i, int j, int t)
{
  GRID *g;
  CELL *c;
  int count;

  if((i < 1) || (j < 1)) {
    return (GRID*)NULL;
  }
  
  g = (GRID*)calloc(1, sizeof(GRID));
  if(!g) { return g; }

  count = i*j;
  g->rows = i;
  g->cols = j;
  g->planes = 1;	/* up/down later */
  g->gtype = t;
  g->max = count;
  g->cells = (CELL*)calloc((size_t)count, sizeof(CELL));
  if(!g->cells) {
    free(g);
    return (GRID*)NULL;
  }

  srandom(time(NULL));

  count = 0;
  for (int row = 0; row < g->rows; row ++) {
    for (int col = 0; col < g->cols; col ++) {
      c = &(g->cells[count]);
      initcell(c, t, row, col, count);
      count ++;
    }
  }

  return g;
} /* creategrid() */

void
freegrid(GRID* g)
{
  int i, j;
  if(!g) {return;}

  if(g->name) { free(g->name); }
  if(g->data) { free(g->data); }

  if(g->cells) {
    for(i = 0; i < g->rows; i++) {
      for(j = 0; j < g->cols; j++) {
	freecell(visitrc(g,i,j));
      }
    }
    free(g->cells);
  }

  free(g);
} /* freegrid() */



/* visit a cell by co-ordinates, return pointer or NULL */
CELL*
visitrc(GRID *g, int i, int j)
{
  int index;

  if(!g || (i < 0) || (j < 0) || (i >= g->rows) || (j >= g->cols)) {
    return (CELL*)NULL;
  }

  index = (g->cols * i) + j;
  return &(g->cells[index]);
} /* visit() */

/* visit a cell directly by id, return pointer or NULL */
CELL*
visitid(GRID *g, int id)
{
  if(!g || (id < 0)) {
    return (CELL*)NULL;
  }

  if(id > g->max) {
    return (CELL*)NULL;
  }

  return &(g->cells[id]);
} /* visitid() */

/* from cell c,
 * visit a cell in direction d,
 * subject to connecton status cs:
 *   NC: only if not connected
 *   SYMMETRICAL: only if symmetrically connected
 *   THIS: only if connected in direction D
 *   ANY: not subject to connection status
 * return pointer or NULL
 */
CELL*
visitdir(GRID *g, CELL *c, int d, int cs)
{
  int i, j, ni, nj, id;

  if(!g || !c) {
    return (CELL*)NULL;
  }

  if( (d < FIRSTDIR) || (d > DIRECTIONS) ) {
    return (CELL*)NULL;
  }

  ni = i = c->row;
  nj = j = c->col;

  switch(d) {
    case NORTH: ni = i - 1; id = (g->cols * ni) + nj; break;
    case SOUTH: ni = i + 1; id = (g->cols * ni) + nj; break;
    case WEST:  nj = j - 1; id = (g->cols * ni) + nj; break;
    case EAST:  nj = j + 1; id = (g->cols * ni) + nj; break;
    case UP:   id = NC; break;	/* not yet implemented */
    case DOWN: id = NC; break;
    default:   id = NC;
  }

  /* off grid in any way? */
  if((id < 0) || (id > g->max) || (ni < 0) || (nj < 0) ||
     (ni >= g->rows) || (nj >= g->cols)) {
    return (CELL*)NULL;
  }

  /* now we know id is a possible cell, but does it meet the
   * connection status criteria?
   */

  CELL *that = &(g->cells[id]);

  /* don't care about connection */
  if(cs == ANY) {
    return that;
  }

  /* only if not connected */
  if(cs == NC) {
    if(c->dir[d] == NC) {
      return that;
    }
  }

  /* only if c in dir d connects to that */
  if(cs == THIS) {
    if(c->dir[d] == id) {
      return that;
    }
  }

  /* only if c in dir d connects to that, and
   * that connects to c in opposite direction */
  if(cs == SYMMETRICAL) {
    int od = opposite(d);
    if((c->dir[d] == id) && (c->id == that->dir[od])) {
      return that;
    }
  }

  return (CELL*)NULL;
} /* visitdir() */

/* a random cell in the grid */
CELL *
visitrandom(GRID *g)
{
  if(!g) { return (CELL*)NULL; }
  
  return visitid(g, random() % g->max);
}

/* {FOO}bycell functions use one or two CELL pointers
 * {FOO}byrc functions take GRID and one or two pairs of row,col
 * {FOO}byid functions take GRID and one or two ids
 *
 * connect{BAR} create a connection
 * isconnected{BAR} test a connection
 * delconnect{BAR} remove a connection
 */

/* connection creation */
/* c2 to c1 direction (c1c2d) need not be opposite of
 * c1 to c2 (c2c1d), and either can be NC: no connection.
 * use SYMMETRICAL for c2c1d to automatically use opposite dir
 */
void 
connectbycell(CELL *c1, int c1c2d, CELL * c2, int c2c1d)
{
  if(!c1) { return; }
  if(!c2) { return; }

  if(c2c1d == SYMMETRICAL) { c2c1d = opposite(c1c2d); }

  if(c1c2d > DIRECTIONS) { return; }
  if(c2c1d > DIRECTIONS) { return; }
  if(c1c2d < NC) { return; }
  if(c2c1d < NC) { return; }

  if(c1c2d > NC) {
    c1->dir[c1c2d] = c2->id;
  }
  if(c2c1d > NC) {
    c2->dir[c2c1d] = c1->id;
  }
} /* connectbycell() */

void connectbyrc(GRID *g, int r1, int c1, int c1c2d,
                          int r2, int c2, int c2c1d)
{
  if(!g) { return; }
  connectbycell(visitrc(g, r1, c1), c1c2d,
                visitrc(g, r2, c2), c2c1d);
} /* connectbyrc() */

void connectbyid(GRID *g, int id1, int c1c2d,
                          int id2, int c2c1d)
{
  if(!g) { return; }
  connectbycell(visitid(g, id1), c1c2d,
                visitid(g, id2), c2c1d);
} /* connectbyid() */


/* check connections */

/* for two cells, returns direction by which c1 connects to c2
 * or NC if no connect.
 * if d is ANYDIR, will try all directions, and returning first found,
 * but if d is a particular direction will only try that one.
 * NOTE that zero is a valid direction, so test results against NC
 * or an expected direction, eg (isconnectedbycell(a, b, dir) == dir)
 */
int 
isconnectedbycell(CELL *c1, CELL *c2, int d)
{
  if(!c1) { return NC; }
  if(!c2) { return NC; }

  if(d != ANYDIR) {
    if(d > DIRECTIONS) { return NC; }
    if(d < FIRSTDIR) { return NC; }

    if(c1->dir[d] == c2->id) {
      return d;
    }
    return NC;
  }

  for(d = FIRSTDIR; d < DIRECTIONS; d ++) {
    if(c1->dir[d] == c2->id) {
      return d;
    }
  }
  return NC;
} /* isconnectedbycell() */

int 
isconnectedbyrc(GRID *g, int r1, int c1, int r2, int c2, int d)
{
  if(!g) { return NC; }

  return isconnectedbycell(visitrc(g,r1,c1), visitrc(g,r2,c2), d);
} /* isconnectedbyrc() */

int 
isconnectedbyid(GRID *g, int id1, int id2, int d)
{
  if(!g) { return NC; }

  return isconnectedbycell(visitid(g,id1), visitid(g,id2), d);
} /* isconnectedbyid() */


/* for a particular cell, return information about the edges it
 * adjoins. Edges are returned as bit status flags, so check
 * results accordingly.
 */
int
edgestatusbycell(GRID *g, CELL *c)
{
  int edges = 0;
  if(!g) { return EDGE_ERROR; }
  if(!c) { return EDGE_ERROR; }

  if(c->row == 0) {           edges = edges|NORTH_EDGE; }
  if(c->col == 0) {           edges = edges|WEST_EDGE; }
  if(c->row == g->rows - 1) { edges = edges|SOUTH_EDGE; }
  if(c->col == g->cols - 1) { edges = edges|EAST_EDGE; }

  if (edges == 0) {           edges = NO_EDGES; }
  return edges;
} /* edgestatusbycell() */

int
edgestatusbyrc(GRID *g, int i, int j)
{
  if(!g) { return EDGE_ERROR; }

  return edgestatusbycell(g, visitrc(g,i,j));
} /* edgestatusbyrc */

int
edgestatusbyid(GRID *g, int id)
{
  if(!g) { return EDGE_ERROR; }

  return edgestatusbycell(g, visitid(g,id));
} /* edgestatusbyid */


/* for a particular cell, return information about the walls it
 * adjoins. Walls are returned as bit status flags, so check
 * results accordingly.
 */
int
wallstatusbycell(CELL *c)
{
  int walls = 0;
  if(!c) { return WALL_ERROR; }

  if(c->dir[NORTH] == NC) { walls = walls|NORTH_WALL; }
  if(c->dir[WEST]  == NC) { walls = walls|WEST_WALL; }
  if(c->dir[SOUTH] == NC) { walls = walls|SOUTH_WALL; }
  if(c->dir[EAST]  == NC) { walls = walls|EAST_WALL; }

  if (walls == 0) { walls = NO_WALLS; }
  return walls;
} /* wallstatusbycell() */

int
wallstatusbyrc(GRID *g, int i, int j)
{
  if(!g) { return WALL_ERROR; }

  return wallstatusbycell(visitrc(g,i,j));
} /* wallstatusbyrc */

int
wallstatusbyid(GRID *g, int id)
{
  if(!g) { return WALL_ERROR; }

  return wallstatusbycell(visitid(g,id));
} /* wallstatusbyid */


/* mallocs space and copies a name to a cell */
/* returns -3 if no cell,
 * -2 if malloc failed after free()ing old name
 * -1 if malloc failed with no old name
 * 0 if no previous name
 * 1 if a previous name had to be free()d
 */
int
namebycell(CELL *c, char *name)
{
  int rc = 0;
  int len;
  if(!c) { return -3; }
  if(c->name) { free(c->name); rc = 1; }
  len = strnlen(name, BUFSIZ - 1) + 1;
  c->name = (char*)malloc((size_t)len);
  if(!c->name) { return -1 - rc; }
  strncpy(c->name, name, len);
  return rc;
} /* namecell() */

int
namebyrc(GRID *g, int i, int j, char *name)
{
  if(!g) { return -3; }
  return namebycell(visitrc(g,i,j), name);
}

int
namebyid(GRID *g, int id, char *name)
{
  if(!g) { return -3; }
  return namebycell(visitid(g,id), name);
}


/* grid version of namebycell() */
int
namegrid(GRID *g, char*name)
{
  int rc = 0;
  int len;
  if(!g) { return -3; }
  if(g->name) { free(g->name); rc = 1; }
  len = strnlen(name, BUFSIZ - 1) + 1;
  g->name = (char*)malloc((size_t)len);
  if(!g->name) { return -1 - rc; }
  strncpy(g->name, name, len);
  return rc;
} /* namegrid() */


/* for every cell in grid, run provided function and
 * return sum of results
 */
int
iterategrid(GRID *g, int (*ifunc)(GRID *,CELL *,void *), void *param)
{
  int rc = 0;
  int i, j;

  for(i = 0; i < g->rows; i ++) {
    for(j = 0; j < g->cols; j ++) {
      rc += ifunc(g, visitrc(g,i,j), param);
    }
  }
  return rc;
}

/* for every cell in row i of grid, run provided function and
 * return sum of results
 */
int
iteraterow(GRID *g, int i, int (*ifunc)(GRID *,CELL *,void *), void *param)
{
  int rc = 0;
  if(!g || (i < 0) || (i >= g->rows)) {
    return -1;
  }

  for(int j = 0; j < g->cols; j++) {
    rc += ifunc(g, visitrc(g,i,j), param);
  }
  return rc;
} /* iteraterow() */

int
iteratecol(GRID *g, int j, int (*ifunc)(GRID *,CELL *,void *), void *param)
{
  int rc = 0;
  if(!g || (j < 0) || (j >= g->cols)) {
    return -1;
  }

  for(int i = 0; i < g->rows; i++) {
    rc += ifunc(g, visitrc(g,i,j), param);
  }
  return rc;
} /* iteratecol() */


/* return the opposite of a direction */
int
opposite(int d) {
  switch(d) {
    case NORTH: return SOUTH;
    case SOUTH: return NORTH;
    case WEST: return EAST;
    case EAST: return WEST;
    case UP: return DOWN;
    case DOWN: return UP;
    default: return NC;
  }
}

/* turn a direction into a string (eg for debugging) */
const char *
dirtoname(int d)
{
   struct ns { char *name; };
   const struct ns dirnames[] = {
     { "north" },
     { "west" },
     { "east" },
     { "south" },
     { "up" },
     { "down" },
     { "not a direction" },
   };

   if((d < 0) || (d > DIRECTIONS)) { d = DIRECTIONS; }
   return dirnames[d].name;
} /* dirtoname() */


/* create a ASCII art version of the grid,
 * use_name is a boolean, if true print names in cells
 * (but will be clipped to 3 chars)
 */
char *
ascii_grid(GRID *g, int use_name)
{
  char *out = NULL;
  int i, j, p, outsize;
  char left, top;
  CELL *here, *west, *north;
  
  if(!g) { return out; }

  p = 0; /* index into out */

  /* a cell is "+---" (4) wide + "+\n" (2) on last cell
   *                   |     cols are "-" (2) tall + "-" (1) on last row
   *                   |              " "  |
   *                   |                   |           + final
   *                   |                   |           | null
   *                   v                   v           v    */
  outsize = (g->cols * 4 + 2) * (g->rows * 2 + 1) +    1 ;
  if(use_name && g->name) {
    p = 1 + strnlen(g->name, BUFSIZ);
    use_name += p;
  }
  out = (char*) malloc(outsize);
  if(!out) { return out; }

  if(use_name && g->name) {
    strncpy(out, g->name, p);
    /* replace the final null */
    out[p-1] = '\n';
  }
    
  for(i = 0; i < g->rows; i ++) { /* grid row */
    for(int l = 0; l < 2; l ++) { /* line of output per row */
      for(j = 0; j < g->cols; j ++) { /* grid col */
        left = '|';
        top = '-';
	here = visitrc(g,i,j);

        if(i) {
	  north = visitrc(g,i-1,j);
	  if(isconnectedbycell(here, north, NORTH) != NC) {
	    top = ' ';
	  } else {
            top = '-';
	  }
	}

        if(j) {
	  west = visitrc(g,i,j-1);
	  if(isconnectedbycell(west, here, EAST) != NC) {
	    left = ' ';
	  } else {
	    left = '|';
	  }
	}

        if(l) {
	  out[p++] =left;
	  if(use_name && here->name) {
	    char *s = here->name;
	    if(*s) { out[p++] = *s; s++; } else { out[p++] = ' '; }
	    if(*s) { out[p++] = *s; s++; } else { out[p++] = ' '; }
	    if(*s) { out[p++] = *s; s++; } else { out[p++] = ' '; }
	  } else {
	    out[p++] = ' '; out[p++] = ' '; out[p++] = ' ';
	  }
	} else {
	  out[p++] = '+'; out[p++] = top; out[p++] = top; out[p++] = top;
	}
      }
      /* end of line */
      if(l) { 
        out[p++] = '|';
      } else {
        out[p++] = '+';
      }
      out[p++] = '\n';
    }
  }
  /* final line */
  for(j = 0; j < g->cols; j ++) { /* grid col */
    out[p++] = '+'; out[p++] = '-'; out[p++] = '-'; out[p++] = '-';
  }
  out[p++] = '+';
  out[p++] = '\n';
  out[p++] = '\0';

  return out;
} /* ascii_grid() */

