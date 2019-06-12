/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* tools for a maze grid */

#ifndef _GRID_H
#define _GRID_H

#define DIRECTIONS      6
#define FOURDIRECTIONS  4	/* without up / down */
#define FIRSTDIR        0
#define NORTH           0
#define WEST            1
#define EAST            2
#define SOUTH           3

/* for future work */
#define UP              4
#define DOWN            5

/* NC, no connect, is used in returns and calls.
 * others are used in calls only
 */
#define NC              -1
#define SYMMETRICAL     -2
#define ANYDIR          -3
#define ANY             -3
#define EITHER          -4
#define THIS            -5

/* Max size of a name */
#ifndef BUFSIZ
#  define BUFSIZ 1024	/* typical value from stdio.h */
#endif

typedef struct
{
   int id; /* unique id of a cell (sequential numbers) */
   int row,col; /* co-ordinates of a cell */
   /* dir array has id value of a connected cell,
    * or -1 for no connection
    */
   int dir[DIRECTIONS];

   int ctype;	/* for future use, probably determine how to draw a cell */
   char *name;	/* for user use */
   void *data;	/* placeholder */
} CELL;

typedef struct
{
   /* total rows / columns / planes */
   int rows;
   int cols;
   int planes;

   int gtype;	/* initially the same as ctype */
   int max;	/* size of one plane (rows*cols) */

   char *name;	/* for user use */
   void *data;	/* placeholder */

   CELL *cells;
} GRID;

void initcell(CELL*, int /*ctype*/, int /*i*/, int/*j*/, int /*id*/);
void freecell(CELL*);

GRID *creategrid(int /*rows*/, int /*cols*/, int /*gtype*/);
void freegrid(GRID *);


/* visit functions return a CELL pointer */
CELL *visitrc(GRID *, int /*rows*/, int /*cols*/);
CELL *visitid(GRID *, int /*cellid*/);
CELL *visitdir(GRID *, CELL */*cell*/, int/*direction*/, int/* connection status */);
CELL *visitrandom(GRID *);


/* bycell functions use one or two CELL pointers
 * byrc functions take GRID and one or two pairs of row,col
 * byid functions take GRID and one or two ids
 *
 * connect makes connections
 * isconnected tests connections
 * delconnect removes connections
 */
void connectbycell(CELL *, int /* cell1 -> cell2 direction */,
             CELL *, int /* cell2 -> cell1 direction */);

void connectbyrc(GRID *,
		int /*r1*/, int /*c1*/, int /* cell1 -> cell2 direction */,
             	int /*r2*/, int /*c2*/, int /* cell2 -> cell1 direction */);

void connectbyid(GRID *,
		int /*id1*/, int /* cell1 -> cell2 direction */,
             	int /*id2*/, int /* cell2 -> cell1 direction */);

int isconnectedbycell(CELL *, CELL *, int /* direction */);
int isconnectedbyrc(GRID *,
		int /*r1*/, int /*c1*/,
		int /*r2*/, int /*c2*/, int /* direction */);
int isconnectedbyid(GRID *, int /*id1*/, int /*id1*/, int /* direction */);

/* assign a name (to a newly malloced string) to a cell */
int namebycell(CELL *, char *);
int namebyrc(GRID *, int /*row*/, int/*col*/, char *);
int namebyid(GRID *, int /*id*/, char *);

/* assign a name (to a newly malloced string) to a grid */
int namegrid(GRID *, char *);

/* call one function on each cell in a row, col, or grid.
 * that one function takes a pointer to a CELL and a pointer
 * to a custom structure.
 */
int iteraterow(GRID *, int, int(*)(CELL*,void *), void *);
int iteratecol(GRID *, int, int(*)(CELL*,void *), void *);
int iterategrid(GRID *, int(*)(CELL*,void *), void *);

/* find the opposite of a direction */
int opposite( int /*dir*/);

/* turn a direction into a name */
const char *dirtoname(int /*dir*/);

/* naive ascii art version of a grid */
char *ascii_grid(GRID *, int /* use_name */);
#endif