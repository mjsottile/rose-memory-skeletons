#define REAL double
#define INREAL "%lf"
#define OUTREAL %.3e

struct particle{
     REAL x;
     REAL y;
     struct particle *next;
};

struct particleline{
     int length;
     struct particle *Particles;
};
 
struct particle *partalloc(REAL x, REAL y);

                          /* Macros for the integer array FLAG      */
#define C_B      0x0000   /* interior obstacle cells                */
#define B_N      0x0001   /* obstacle cells adjacent to fluid cells */
#define B_S      0x0002   /* in the respective direction            */
#define B_W      0x0004   
#define B_O      0x0008 
#define B_NW     0x0005    
#define B_SW     0x0006      
#define B_NO     0x0009    
#define B_SO     0x000a

#define C_F      0x0010   /* fluid cell */

#define C_E     0x1000   /* empty celle                 */
#define C_N     0x0800   /* free surface cells          */
#define C_S     0x0400   /* adjacent to empty cells     */
#define C_W     0x0200   /* in the respective direction */
#define C_O     0x0100 
#define C_WO    0x0300
#define C_NS    0x0c00
#define C_SW    0x0600
#define C_NW    0x0a00
#define C_NO    0x0900
#define C_SO    0x0500
#define C_SWO   0x0700 
#define C_NSW   0x0e00
#define C_NWO   0x0b00
#define C_NSO   0x0d00
#define C_NSWO  0x0f00


/* Macros for POISSON, denoting whether there is an obstacle cell */
/* adjacent to some direction                                     */
#define eps_E !(FLAG[i+1][j] < C_F)
#define eps_W !(FLAG[i-1][j] < C_F)
#define eps_N !(FLAG[i][j+1] < C_F)
#define eps_S !(FLAG[i][j-1] < C_F)
