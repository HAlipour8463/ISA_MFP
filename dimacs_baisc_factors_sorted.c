
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <math.h>

#define STATS

typedef unsigned int uint;
typedef long int lint;
typedef long long int llint;
typedef unsigned long long int ullint;

double
timer (void)
{
  struct rusage r;

  getrusage(0, &r);
  return (double) (r.ru_utime.tv_sec + r.ru_utime.tv_usec / (double)1000000);
}

struct node;

typedef struct arc
{
	struct node *from;
	struct node *to;
	uint flow;
	lint capacity;
	uint direction;
} Arc;

typedef struct node
{
	uint visited;
	uint numAdjacent;
	uint number;
	uint label;
	lint excess;
	uint incomingCapacity;
	uint outgoingCapacity;
	struct node *parent;
	struct node *childList;
	struct node *nextScan;
	uint numOutOfTree;
	Arc **outOfTree;
	uint nextArc;
	Arc *arcToParent;
	struct node *next;
} Node;


typedef struct root
{
	Node *start;
	Node *end;
} Root;

//---------------  Global variables ------------------
static uint numNodes = 0;
static uint numArcs = 0;
static uint source = 0;
static uint sink = 0;


long SrcAAC = 0, SnkAAC = 0;
int Src_degree = 0, Snk_degree = 0;
ullint AllCap = 0, NnSrcAAC = 0, NnzNnSrcAAC = 0;


#ifdef LOWEST_LABEL
static uint lowestStrongLabel = 1;
#else
static uint highestStrongLabel = 1;
#endif

static Node *adjacencyList = NULL;
static Root *strongRoots = NULL;
static uint *labelCount = NULL;
static Arc *arcList = NULL;
//-----------------------------------------------------


static void
initializeNode (Node *nd, const uint n)
{
	nd->label = 0;
	nd->excess = 0;
	nd->incomingCapacity = 0;
	nd->outgoingCapacity = 0;
	nd->parent = NULL;
	nd->childList = NULL;
	nd->nextScan = NULL;
	nd->nextArc = 0;
	nd->numOutOfTree = 0;
	nd->arcToParent = NULL;
	nd->next = NULL;
	nd->visited = 0;
	nd->numAdjacent = 0;
	nd->number = n;
	nd->outOfTree = NULL;
}


static void
initializeArc (Arc *ac)
{
	ac->from = NULL;
	ac->to = NULL;
	ac->capacity = 0;
	ac->flow = 0;
	ac->direction = 1;
}


static void
readDimacsFileCreateList (void)
{
	uint lineLength=1024, i, capacity, numLines = 0, from, to, first=0, last=0;
	char *line, *word, ch, ch1;

	if ((line = (char *) malloc ((lineLength+1) * sizeof (char))) == NULL)
	{
		printf ("%s, %d: Could not allocate memory.\n", __FILE__, __LINE__);
		exit (1);
	}

	if ((word = (char *) malloc ((lineLength+1) * sizeof (char))) == NULL)
	{
		printf ("%s, %d: Could not allocate memory.\n", __FILE__, __LINE__);
		exit (1);
	}

	while (fgets (line, lineLength, stdin))
	{
		++ numLines;

		switch (*line)
		{
		case 'p':

			sscanf (line, "%c %s %d %d", &ch, word, &numNodes, &numArcs);

			if ((adjacencyList = (Node *) malloc (numNodes * sizeof (Node))) == NULL)
			{
				printf ("%s, %d: Could not allocate memory.\n", __FILE__, __LINE__);
				exit (1);
			}



			if ((arcList = (Arc *) malloc (numArcs * sizeof (Arc))) == NULL)
			{
				printf ("%s, %d: Could not allocate memory.\n", __FILE__, __LINE__);
				exit (1);
			}

			for (i=0; i<numNodes; ++i)
			{
				initializeNode (&adjacencyList[i], (i+1));
			}

			for (i=0; i<numArcs; ++i)
			{
				initializeArc (&arcList[i]);
			}

			first = 0;
			last = numArcs-1;

			break;

		case 'a':

			sscanf (line, "%c %d %d %d", &ch, &from, &to, &capacity);

/* Calculate source and sink adjacant arc capacities ------------------------------------------------------------ */


			AllCap += capacity;

			if (from == source)
			{

				SrcAAC += capacity;
				Src_degree +=1;
			}

			if (to == sink)
			{

				SnkAAC += capacity;
				Snk_degree +=1;
			}
// -----------------------------------------------------------------------------------------------------------------


			if ((from+to) % 2)
			{
				arcList[first].from = &adjacencyList[from-1];
				arcList[first].to = &adjacencyList[to-1];
				arcList[first].capacity = capacity;

				++ first;
			}
			else
			{
				arcList[last].from = &adjacencyList[from-1];
				arcList[last].to = &adjacencyList[to-1];
				arcList[last].capacity = capacity;
				-- last;
			}

			++ adjacencyList[from-1].numAdjacent;
			++ adjacencyList[to-1].numAdjacent;

			// adjacencyList[from-1].outgoingCapcacity += capacity;
			adjacencyList[from-1].excess -= capacity;
			adjacencyList[from-1].outgoingCapacity += capacity;
			// adjacencyList[to-1].incomingCapcacity += capacity;
			adjacencyList[to-1].excess += capacity;
			adjacencyList[to-1].incomingCapacity += capacity;

			break;

		case 'n':

			sscanf (line, "%c %d %c", &ch, &i, &ch1);

			if (ch1 == 's')
			{
				source = i;
			}
			else if (ch1 == 't')
			{
				sink = i;
			}
			else
			{
				printf ("Unrecognized character %c on line %d\n", ch1, numLines);
				exit (1);
			}

			break;
		}
	}


	for (i=0; i<numArcs; i++)
	{
		to = arcList[i].to->number;
		from = arcList[i].from->number;
		capacity = arcList[i].capacity;

	}

	free (line);
	line = NULL;

	free (word);
	word = NULL;
}


//-------------------------------------------------------

// Definition of compare functions used in qsort function;

int compare(const void *elem1, const void *elem2)
{
    lint l = *(const lint *)elem1 ;
    lint r = *(const lint *)elem2;
    if (l == r)
    {
        //printf("%d and %d are equal\n", l, r);
        return 0;
    }
    else if (l < r)
    {
       //printf("%d is less than %d \n", l, r);
       return -1;
    }
    else
    {
       //printf("%d is greater than %d\n", l, r);
       return 1;
    }
}

//******************************************************************************

double median (const lint *a, const int l, const int r)
{
    //printf("\nl: %d, r: %d, a[l]: %f, a[r]: %f\n", l, r, a[l], a[r]);
    double med;
    int Numpoints = r-l+1;
    if (Numpoints%2)
        med = a[(l+r+1)/2];
    else
        med = (double)(a[(l+r)/2] + a[(l+r)/2+1])/(double)(2);
    return med;
}
//******************************************************************************
//******************************************************************************

void QuartIdxs(const int l, const int r, int Qrt_idx[], const char method) // see https://www.tutorialspoint.com/cprogramming/c_return_arrays_from_function.htm for the way of returning  an array
{
    //static int Qrt_idx[2];
    int Numpoints = r-l+1;

    if (method=='S')
    {
        double frc1 = (double)(Numpoints+1)/(double)4;
        double frc3 = (double)3*(Numpoints+1)/(double)4;

        Qrt_idx[0] = (frc1 - floor(frc1) >= 0.5) ? ceil(frc1) : floor(frc1);
        Qrt_idx[1] = (frc3 - floor(frc3) > 0.5) ? ceil(frc3) : floor(frc3);
        //printf("\nNumpoints is %d, l is %f, r is %f ", Numpoints, frc1 ,  frc3);
    }
    else if (Numpoints%2)
    {
        switch (method) // Difference between M&M and Tukey methods can be found here: https://www.mathematics-monster.com/lessons/methods_for_finding_the_quartiles.html
		{
		case 'M':
            Qrt_idx[0] = (l+r+1)/2 -1; // left index of the Qrt_idxian
            Qrt_idx[1] = (l+r+1)/2+1; // right index of the Qrt_idxian

        break;

        case 'T':
            Qrt_idx[0] = (l+r+1)/2; // left index of the Qrt_idxian
            Qrt_idx[1] = (l+r+1)/2; // right index of the Qrt_idxian
        break;
		}
    }
    else
    {
        Qrt_idx[0] = (l+r)/2; // left index of the Qrt_idxian
        Qrt_idx[1] = (l+r)/2+1; // right index of the Qrt_idxian
    }
    //printf ("lIdx= %f, mid= %f, rIdx= %f\n", Qrt_idx[0], Qrt_idx[1], Qrt_idx[2]);
}
//----------------------------------------------------------

static void
freeMemory (void)
{


	free (adjacencyList);

	free (arcList);
}

int
main(int argc, char ** argv)
{
	int i, j;	// This command is necessary to use 'i' and 'j' in for loops within the main funciton!

	double readStart, readEnd, initStart, initEnd, solveStart, solveEnd, flowStart, flowEnd;
	uint gap;

	double AvSrcAAC = 0, CmpAvSrcAAC = 0, AvNnSrcAAC = 0, CmpAvNnSrcAAC = 0;
	double AvDegNnSrc = 0, CpmAvDegNnSrc = 0, AvAC, CmpAvAC, Dens1, Dens2;
	double TrivialEtraFlw = 0, EstMinPrcDepFlw, CmpEstMinPrcDepFlw, NetPotPth;
	double PrcNetPotPth, sumsqr, StDAC, StDNdDg, Cmpsumsqr, CmpStDAC;
	double BdPerCap =0, CmpBdPerCap = 0, GdPerCap =0, CmpGdPerCap = 0;
	double maxcap = 0, mincap = 1000000, NetCap = 0, CapDens = 0 , BgCapOut = 0, CmpBgCapOut = 0, RlDiffMnMxCap = 0;
	double CmpOutlMnCap = 0, CmpOutlMxCap = 0, CapAvNddg = 0, RlCapAvNddg = 0;
	lint PotNetExcess = 0, PotNetDeficit = 0;

	readDimacsFileCreateList ();

	// simpleInitialization ();

// ------------------------ Extract Max flow features ------------------------------------
			// dimacsfeaturecalculator ();

// --------Parameters used in extracting features ----------

	Dens1 = (double)(numArcs) / (double)(numNodes*(numNodes-1));
	Dens2 = (double)(numArcs) / (double)(numNodes);
	AvAC = (double)AllCap / (double)(numArcs);
	CmpAvAC = (double)AllCap / (double)(numNodes*(numNodes-1));
	NnSrcAAC = AllCap - SrcAAC;
	NnzNnSrcAAC = numArcs - Src_degree;
	AvSrcAAC = (double)SrcAAC / (double)Src_degree;
	CmpAvSrcAAC = (double)SrcAAC / (double)(numNodes - 1);
	AvNnSrcAAC = (double)NnSrcAAC / (double)(numArcs - Src_degree);
	CmpAvNnSrcAAC = (double)NnSrcAAC / (double)((numNodes-1)*(numNodes-1));
	AvDegNnSrc = (double)(numArcs - Src_degree)/(double)(numNodes-1);
	CpmAvDegNnSrc = numNodes - 2;

//-----------------------------Extract features--------------------------------

TrivialEtraFlw = (double)(SrcAAC - SnkAAC)/ (double)(SrcAAC);
EstMinPrcDepFlw = (double)(AvNnSrcAAC*AvDegNnSrc)/(double)(AvSrcAAC);
CmpEstMinPrcDepFlw = (double)(CmpAvNnSrcAAC*CpmAvDegNnSrc)/(double)CmpAvSrcAAC;
NetPotPth = (double)SrcAAC / (double)AvNnSrcAAC;
PrcNetPotPth = (double)NetPotPth / (double)NnzNnSrcAAC;

//------------------------------------------------------------------------------

for (i=0; i<numArcs; ++i)
{
	if (arcList[i].capacity < AvAC)
	 {
	   BdPerCap +=1;
	 }

	else if (arcList[i].capacity > AvAC)

	 {
	   GdPerCap +=1;
	 }

	if (arcList[i].capacity < CmpAvAC)
	 {
	   CmpBdPerCap +=1;
	 }

	else if (arcList[i].capacity > CmpAvAC)

	 {
	   CmpGdPerCap +=1;
	 }
}
// ---------------------------------------------------------------------------------------------------
for (i=0; i<numArcs; ++i)
{
   maxcap = (maxcap > arcList[i].capacity) ? maxcap : arcList[i].capacity;
   mincap = (mincap < arcList[i].capacity) ? mincap : arcList[i].capacity;
}

lint MaxExcess = 0, MinExcess = 10000000, MaxInnerNdDg = 0, MinInnerNdDg = 1000000;


for (i=0; i<numNodes ; ++i)
{
   if (i!= (source-1) && (i!= sink-1))
   {
       MaxExcess = (MaxExcess > adjacencyList[i].excess) ? MaxExcess : adjacencyList[i].excess;
       MinExcess = (MinExcess < adjacencyList[i].excess) ? MinExcess : adjacencyList[i].excess;
       MaxInnerNdDg = (MaxInnerNdDg > adjacencyList[i].numAdjacent) ? MaxInnerNdDg : adjacencyList[i].numAdjacent;
       MinInnerNdDg = (MinInnerNdDg < adjacencyList[i].numAdjacent) ? MinInnerNdDg : adjacencyList[i].numAdjacent;
   }
}


NetCap = AllCap;
CapDens = (double)NetCap / (double)(maxcap * (numNodes*(numNodes-1)));
BgCapOut =  (double)AvAC / (double)maxcap;
CmpBgCapOut = (double)CmpAvAC / (double)maxcap;
RlDiffMnMxCap = (double)( maxcap - mincap ) / (double)maxcap;

CmpOutlMnCap = (double)CmpAvAC / (double)mincap;
CmpOutlMxCap = (double)maxcap / (double)CmpAvAC;
CapAvNddg = (double)NetCap / (double)(numNodes-1);
RlCapAvNddg = (double)NetCap / (double)(maxcap *(numNodes-1));

//------------ Standard deviation of AC ------------------------------------------------------------

for (i=0; i<numArcs; ++i)
{
	sumsqr += pow((arcList[i].capacity - AvAC), 2);
	Cmpsumsqr += pow((arcList[i].capacity - CmpAvAC), 2);
}

Cmpsumsqr += ((numNodes*(numNodes-1))-numArcs)*pow(CmpAvAC, 2);
StDAC = sqrt((double)sumsqr / (double)(numArcs-1)); // Dominator is normalized with n-1 instead of n; sample vs populations;
CmpStDAC = sqrt((double)(sumsqr ) / (double)(numNodes*(numNodes-1)));

//------------ Potential excess of each node ------------------------------------------------------------

for (i=0; i<numNodes; i++)
{
	// PotNetExcess +=adjacencyList[i].excess;
	if (adjacencyList[i].excess > 0)
	{
	PotNetExcess +=adjacencyList[i].excess;
	// printf("excess of node %ld: %ld\n", i+1, adjacencyList[i].excess);
	// printf("PotNetExcess is : %ld\n", PotNetExcess);
	}
	else if (adjacencyList[i].excess < 0)
	{
	PotNetDeficit -=adjacencyList[i].excess;
	// printf("Deficit of node %ld: %ld\n", i+1, adjacencyList[i].excess);
	// printf("PotNetDeficit is : %ld\n", PotNetDeficit);

	}
}
 //printf ("sink excess %d:    %d\n", i, adjacencyList[sink].excess);
 PotNetExcess -= adjacencyList[sink-1].excess;
 PotNetDeficit += adjacencyList[source-1].excess;

 //printf ("PotNetExcess:     %d\n", PotNetExcess);

 //------------------------- STD of Positive PotNetExcess and node degree---------------------

 double sumsqrPotExcess = 0, AvPotNetExcess, StDPotNetExcess = 0;
 double sumsqrPotDeficit = 0, AvPotNetDeficit, StDPotNetDeficit = 0;
 double sumsqrNddegree = 0, AvNddegree, AvInnerNddegree, StDNddegree = 0;

 AvPotNetExcess = (double)PotNetExcess / (double)(numNodes-2);
 AvPotNetDeficit = (double)PotNetDeficit / (double)(numNodes-2);
 AvNddegree = (double)2*numArcs/(double)(numNodes); // degrees are regarded regardless of the arc direction.
                                                    // Therefore, each arc is counted two times- one tome for tail node snd one time for head node;
AvInnerNddegree = (double)(2*numArcs-Src_degree-Snk_degree)/(double)(numNodes-2);

 //printf("\nsource is %d with excess: %d", source, adjacencyList[source-1].excess);
 //printf("\nsink is %d with excess: %d", sink, adjacencyList[sink-1].excess);

for (i=0; i<numNodes; i++)
{
    sumsqrNddegree += pow((adjacencyList[i].numAdjacent - AvNddegree), 2);

    if (adjacencyList[i].excess <0 && i!= source-1)
        sumsqrPotDeficit += pow((adjacencyList[i].excess - AvPotNetDeficit), 2);

    else if(adjacencyList[i].excess >= 0 && i!= source-1 && i!= sink-1)
        sumsqrPotDeficit += pow((0 - AvPotNetDeficit), 2);

    if (adjacencyList[i].excess >0 && i!= sink-1)
        sumsqrPotExcess += pow((adjacencyList[i].excess - AvPotNetExcess), 2);

    else if(adjacencyList[i].excess <= 0 && i!= source-1 && i!= sink-1)
        sumsqrPotExcess += pow((0 - AvPotNetExcess), 2);
}
StDPotNetExcess = sqrt(sumsqrPotExcess / (numNodes-3)); // dominator is n-1 and source and sink nodes are precluded also.
                                                        // Therefore, 3 is subtracted from numNodes in the dominator.
StDPotNetDeficit = sqrt(sumsqrPotDeficit / (numNodes-3));
StDNddegree = sqrt(sumsqrNddegree / (numNodes-1));

 // ---------- Sort arcs based on their capacities--------------------------

lint capArcs[numArcs];

for (i=0; i< numArcs; ++i)
{
    capArcs[i] = arcList[i].capacity;
    //printf("cap %f\n", capArcs[i]);
}

qsort((void *)capArcs, numArcs, sizeof(capArcs[0]), compare);
/*
for (i=0; i<numArcs; ++i)
{
    printf("%d  %d\n", i,  capArcs[i]);
}
*/
// --------- Quartiles of arc capacities ---------------------------------------
int Qrt_idx[2];

/* ------------------ Moore and McCabe (M&M) method ----------------------------*/
char M;

QuartIdxs(0, numArcs-1, Qrt_idx, 'M');
int AC_MM_midLIdx = Qrt_idx[0];
int AC_MM_midRIdx = Qrt_idx[1];

//printf("l is %d with cap  %d, r is %d with cap: %d", AC_MM_midLIdx, capArcs[AC_MM_midLIdx], AC_MM_midRIdx, arcList[AC_MM_midRIdx].capacity);

double AC_MM_md = median(capArcs, 0, numArcs-1); // median
double AC_MM_Q1 = median(capArcs, 0, AC_MM_midLIdx); // the first quartile
double AC_MM_Q3 = median(capArcs, AC_MM_midRIdx, numArcs-1); // the third quartile
double AC_MM_IQR = AC_MM_Q3 - AC_MM_Q1; //interquartile range

//-------------detect outliers, see: https://www.itl.nist.gov/div898/handbook/prc/section1/prc16.htm
double AC_MM_LIF = AC_MM_Q1-1.5*AC_MM_IQR, AC_MM_UIF = AC_MM_Q3+1.5*AC_MM_IQR,  AC_MM_LOF = AC_MM_Q1-3*AC_MM_IQR, AC_MM_UOF = AC_MM_Q3+3*AC_MM_IQR;

//printf("\nAC_MM_md: %f \n \nAC_MM_Q1: %f, AC_MM_Q3: %f, and AC_MM_IQR is %f \n", AC_MM_md, AC_MM_Q1, AC_MM_Q3, AC_MM_IQR);
//printf("AC_MM_LOF: %f, AC_MM_LIF: %f, AC_MM_UIF: %f,  AC_MM_UOF: %f\n\n", AC_MM_LOF, AC_MM_LIF, AC_MM_UIF,   AC_MM_UOF);

int AC_MM_LExtOut=0, AC_MM_LMldOut=0, AC_MM_UMldOut=0, AC_MM_UExtOut=0;

for (i=0; i<numArcs; ++i)
{
    if (capArcs[i] >= AC_MM_LIF)
        break;
    else if(capArcs[i] < AC_MM_LIF & capArcs[i] >= AC_MM_LOF)
        ++ AC_MM_LMldOut;
    else if( capArcs[i] < AC_MM_LOF)
        ++ AC_MM_LExtOut;
}
//printf("broken in %d, AC_MM_LExtOut: %d, AC_MM_LMldOut: %d\n", i,  AC_MM_LExtOut, AC_MM_LMldOut);

for (i=numArcs-1; i>0; --i)
{
    if (capArcs[i] <= AC_MM_UIF)
        break;
    if(capArcs[i] > AC_MM_UIF & capArcs[i] <= AC_MM_UOF)
        ++ AC_MM_UMldOut;
    else if(capArcs[i] > AC_MM_UOF)
        ++ AC_MM_UExtOut;
    else
       break;
}

//printf("broken in %d, AC_MM_UMldOut: %d, AC_MM_UExtOut: %d\n", i,  AC_MM_UMldOut, AC_MM_UExtOut);

//---------------------Tukey method-------------------------

char T;

QuartIdxs(0, numArcs-1, Qrt_idx, 'T');
int AC_T_midLIdx = Qrt_idx[0];
int AC_T_midRIdx = Qrt_idx[1];

double AC_T_md = median(capArcs, 0, numArcs-1); // median
double AC_T_Q1 = median(capArcs, 0, AC_T_midLIdx); // the first quartile
double AC_T_Q3 = median(capArcs, AC_T_midRIdx, numArcs-1); // the third quartile
double AC_T_IQR = AC_T_Q3 - AC_T_Q1; //interquartile range

//-------------detect outliers, see: https://www.itl.nist.gov/div898/handbook/prc/section1/prc16.htm
double AC_T_LIF = AC_T_Q1-1.5*AC_T_IQR, AC_T_UIF = AC_T_Q3+1.5*AC_T_IQR,  AC_T_LOF = AC_T_Q1-3*AC_T_IQR, AC_T_UOF = AC_T_Q3+3*AC_T_IQR;

//printf("\nAC_T_Q1: %f, AC_T_Q3: %f, and AC_T_IQR is %f \n", AC_T_Q1, AC_T_Q3, AC_T_IQR);
//printf("AC_T_LOF: %f, AC_T_LIF: %f, AC_T_UIF: %f,  AC_T_UOF: %f\n\n", AC_T_LOF, AC_T_LIF, AC_T_UIF,   AC_T_UOF);

int AC_T_LExtOut=0, AC_T_LMldOut=0, AC_T_UMldOut=0, AC_T_UExtOut=0;

for (i=0; i<numArcs; ++i)
{
    if (capArcs[i] >= AC_T_LIF)
        break;
    else if(capArcs[i] < AC_T_LIF & capArcs[i] >= AC_T_LOF)
        ++ AC_T_LMldOut;
    else if( capArcs[i] < AC_T_LOF)
        ++ AC_T_LExtOut;
}
//printf("broken in %d, AC_T_LExtOut: %d, AC_T_LMldOut: %d\n", i,  AC_T_LExtOut, AC_T_LMldOut);

for (i=numArcs-1; i>0; --i)
{
    if (capArcs[i] <= AC_T_UIF)
        break;
    if(capArcs[i] > AC_T_UIF & capArcs[i] <= AC_T_UOF)
        ++ AC_T_UMldOut;
    else if(capArcs[i] > AC_T_UOF)
        ++ AC_T_UExtOut;
    else
       break;
}

//printf("broken in %d, AC_T_UMldOut: %d, AC_T_UExtOut: %d\n", i,  AC_T_UMldOut, AC_T_UExtOut);

//**********************Mendenhall and Sincich method*************************
char S;

QuartIdxs(0, numArcs-1, Qrt_idx, 'S');
int AC_MS_Q1Idx = Qrt_idx[0]-1;
int AC_MS_Q3Idx = Qrt_idx[1]-1;

//printf("\nAC_MS_Q1Idx is : %d and AC_MS_Q3Idx is %d\n", AC_MS_Q1Idx, AC_MS_Q3Idx);
double AC_MS_Q1 = capArcs[AC_MS_Q1Idx];
double AC_MS_Q3 = capArcs[AC_MS_Q3Idx];
double AC_MS_IQR = AC_MS_Q3 - AC_MS_Q1;

//-------------detect outliers --------------------------------------
double AC_MS_LIF = AC_MS_Q1-1.5*AC_MS_IQR, AC_MS_UIF = AC_MS_Q3+1.5*AC_MS_IQR,  AC_MS_LOF = AC_MS_Q1-3*AC_MS_IQR, AC_MS_UOF = AC_MS_Q3+3*AC_MS_IQR;
//printf("\nAC_MS_Q1: %f, AC_MS_Q3: %f, and AC_MS_IQR is %f \n", AC_MS_Q1, AC_MS_Q3, AC_MS_IQR);
//printf("AC_MS_LOF: %f, AC_MS_LIF: %f, AC_MS_UIF: %f,  AC_MS_UOF: %f\n\n", AC_MS_LOF, AC_MS_LIF, AC_MS_UIF,   AC_MS_UOF);

int AC_MS_LExtOut=0, AC_MS_LMldOut=0, AC_MS_UMldOut=0, AC_MS_UExtOut=0;

for (i=0; i<numArcs; ++i)
{
    if (capArcs[i] >= AC_MS_LIF)
        break;
    else if(capArcs[i] < AC_MS_LIF & capArcs[i] >= AC_MS_LOF)
        ++ AC_MS_LMldOut;
    else if( capArcs[i] < AC_MS_LOF)
        ++ AC_MS_LExtOut;
}
//printf("broken in %d, AC_MS_LExtOut: %d, AC_MS_LMldOut: %d\n", i,  AC_MS_LExtOut, AC_MS_LMldOut);

for (i=numArcs-1; i>0; --i)
{
    if (capArcs[i] <= AC_MS_UIF)
        break;
    if(capArcs[i] > AC_MS_UIF & capArcs[i] <= AC_MS_UOF)
        ++ AC_MS_UMldOut;
    else if(capArcs[i] > AC_MS_UOF)
        ++ AC_MS_UExtOut;
    else
       break;
}
//printf("broken in %d, AC_MS_UMldOut: %d, AC_MS_UExtOut: %d\n", i,  AC_MS_UMldOut, AC_MS_UExtOut);

//-------------------------Sorting Nodes based on their excess------------------------------------

lint nodeExcess[numNodes];
for (i=0; i<numNodes; ++i)
{
    if (adjacencyList[i].excess > 0)
        nodeExcess[i] = adjacencyList[i].excess;
    else
        nodeExcess[i] = 0;
}

qsort((void *)nodeExcess, numNodes, sizeof(nodeExcess[0]), compare);
// qsort((void *)adjacencyList, numNodes, sizeof(adjacencyList[0]), compareexcess);
/*
printf("\n AvPotNetExcess: %f\n", AvPotNetExcess);
printf("\n sumsqrPotExcess: %f\n", sumsqrPotExcess);
printf("\n StDPotNetExcess: %f\n", StDPotNetExcess);

printf("Sorting Nodes based on their excess\n");
for (i=0; i<numNodes; ++i)
{
    printf("%d \n",  nodeExcess[i]);
}
*/
/* ------------------ Moore and McCabe (M&M) method ----------------------------*/

QuartIdxs(0, numNodes-1, Qrt_idx, 'M');
int  NdExcess_MM_midLIdx = Qrt_idx[0];
int  NdExcess_MM_midRIdx = Qrt_idx[1];

//printf("l is %d with cap  %d, r is %d with cap: %d",  NdExcess_MM_midLIdx, nodeExcess[ NdExcess_MM_midLIdx],  NdExcess_MM_midRIdx, nodeExcess[ NdExcess_MM_midRIdx]);

 double NdExcess_MM_md = median(nodeExcess, 0, numNodes-1); // median
 double NdExcess_MM_Q1 = median(nodeExcess, 0,  NdExcess_MM_midLIdx); // the first quartile
double  NdExcess_MM_Q3 = median(nodeExcess,  NdExcess_MM_midRIdx, numNodes-1); // the third quartile
double  NdExcess_MM_IQR =  NdExcess_MM_Q3 -  NdExcess_MM_Q1; //erquartile range

//-------------detect outliers, see: https://www.itl.nist.gov/div898/handbook/prc/section1/prc16.htm
double  NdExcess_MM_LIF =  NdExcess_MM_Q1-1.5* NdExcess_MM_IQR,  NdExcess_MM_UIF =  NdExcess_MM_Q3+1.5* NdExcess_MM_IQR,   NdExcess_MM_LOF =  NdExcess_MM_Q1-3* NdExcess_MM_IQR,  NdExcess_MM_UOF =  NdExcess_MM_Q3+3* NdExcess_MM_IQR;

//printf("\n NdExcess_MM_md: %f \n \n NdExcess_MM_Q1: %f,  NdExcess_MM_Q3: %f, and  NdExcess_MM_IQR is %f \n",  NdExcess_MM_md,  NdExcess_MM_Q1,  NdExcess_MM_Q3,  NdExcess_MM_IQR);
//printf(" NdExcess_MM_LOF: %f,  NdExcess_MM_LIF: %f,  NdExcess_MM_UIF: %f,   NdExcess_MM_UOF: %f\n\n",  NdExcess_MM_LOF,  NdExcess_MM_LIF,  NdExcess_MM_UIF,    NdExcess_MM_UOF);

int  NdExcess_MM_LExtOut=0,  NdExcess_MM_LMldOut=0,  NdExcess_MM_UMldOut=0,  NdExcess_MM_UExtOut=0;

for (i=0; i<numNodes; ++i)
{
    if (nodeExcess[i]>=  NdExcess_MM_LIF)
        break;
    else if(nodeExcess[i]<  NdExcess_MM_LIF & nodeExcess[i]>=  NdExcess_MM_LOF)
        ++  NdExcess_MM_LMldOut;
    else if( nodeExcess[i]<  NdExcess_MM_LOF)
        ++  NdExcess_MM_LExtOut;
}
//printf("broken in %d,  NdExcess_MM_LExtOut: %d,  NdExcess_MM_LMldOut: %d\n", i,   NdExcess_MM_LExtOut,  NdExcess_MM_LMldOut);

for (i=numNodes-1; i>0; --i)
{
    if (nodeExcess[i]<=  NdExcess_MM_UIF)
        break;
    if(nodeExcess[i]>  NdExcess_MM_UIF & nodeExcess[i]<=  NdExcess_MM_UOF)
        ++  NdExcess_MM_UMldOut;
    else if(nodeExcess[i]>  NdExcess_MM_UOF)
        ++  NdExcess_MM_UExtOut;
    else
       break;
}

//printf("broken in %d,  NdExcess_MM_UMldOut: %d,  NdExcess_MM_UExtOut: %d\n", i,   NdExcess_MM_UMldOut,  NdExcess_MM_UExtOut);

//---------------------Tukey method-------------------------

QuartIdxs(0, numNodes-1, Qrt_idx, 'T');
int NdExcess_T_midLIdx = Qrt_idx[0];
int NdExcess_T_midRIdx = Qrt_idx[1];

double NdExcess_T_md = median(nodeExcess, 0, numNodes-1); // median
double NdExcess_T_Q1 = median(nodeExcess, 0, NdExcess_T_midLIdx); // the first quartile
double NdExcess_T_Q3 = median(nodeExcess, NdExcess_T_midRIdx, numNodes-1); // the third quartile
double NdExcess_T_IQR = NdExcess_T_Q3 - NdExcess_T_Q1; //erquartile range

//-------------detect outliers, see: https://www.itl.nist.gov/div898/handbook/prc/section1/prc16.htm
double NdExcess_T_LIF = NdExcess_T_Q1-1.5*NdExcess_T_IQR, NdExcess_T_UIF = NdExcess_T_Q3+1.5*NdExcess_T_IQR,  NdExcess_T_LOF = NdExcess_T_Q1-3*NdExcess_T_IQR, NdExcess_T_UOF = NdExcess_T_Q3+3*NdExcess_T_IQR;

//printf("\nNdExcess_T_Q1: %f, NdExcess_T_Q3: %f, and NdExcess_T_IQR is %f \n", NdExcess_T_Q1, NdExcess_T_Q3, NdExcess_T_IQR);
//printf("NdExcess_T_LOF: %f, NdExcess_T_LIF: %f, NdExcess_T_UIF: %f,  NdExcess_T_UOF: %f\n\n", NdExcess_T_LOF, NdExcess_T_LIF, NdExcess_T_UIF,   NdExcess_T_UOF);

int NdExcess_T_LExtOut=0, NdExcess_T_LMldOut=0, NdExcess_T_UMldOut=0, NdExcess_T_UExtOut=0;

for (i=0; i<numNodes; ++i)
{
    if (nodeExcess[i]>= NdExcess_T_LIF)
        break;
    else if(nodeExcess[i]< NdExcess_T_LIF & nodeExcess[i]>= NdExcess_T_LOF)
        ++ NdExcess_T_LMldOut;
    else if( nodeExcess[i]< NdExcess_T_LOF)
        ++ NdExcess_T_LExtOut;
}
//printf("broken in %d, NdExcess_T_LExtOut: %d, NdExcess_T_LMldOut: %d\n", i,  NdExcess_T_LExtOut, NdExcess_T_LMldOut);

for (i=numNodes-1; i>0; --i)
{
    if (nodeExcess[i]<= NdExcess_T_UIF)
        break;
    if(nodeExcess[i]> NdExcess_T_UIF & nodeExcess[i]<= NdExcess_T_UOF)
        ++ NdExcess_T_UMldOut;
    else if(nodeExcess[i]> NdExcess_T_UOF)
        ++ NdExcess_T_UExtOut;
    else
       break;
}

//printf("broken in %d, NdExcess_T_UMldOut: %d, NdExcess_T_UExtOut: %d\n", i,  NdExcess_T_UMldOut, NdExcess_T_UExtOut);

//**********************Mendenhall and Sincich method*************************

QuartIdxs(0, numNodes-1, Qrt_idx, 'S');
int NdExcess_MS_Q1Idx = Qrt_idx[0]-1;
int NdExcess_MS_Q3Idx = Qrt_idx[1]-1;

//printf("\nNdExcess_MS_Q1Idx is : %d and NdExcess_MS_Q3Idx is %d\n", NdExcess_MS_Q1Idx, NdExcess_MS_Q3Idx);
double NdExcess_MS_Q1 = nodeExcess[NdExcess_MS_Q1Idx];
double NdExcess_MS_Q3 = nodeExcess[NdExcess_MS_Q3Idx];
double NdExcess_MS_IQR = NdExcess_MS_Q3 - NdExcess_MS_Q1;

//-------------detect outliers --------------------------------------
double NdExcess_MS_LIF = NdExcess_MS_Q1-1.5*NdExcess_MS_IQR, NdExcess_MS_UIF = NdExcess_MS_Q3+1.5*NdExcess_MS_IQR,  NdExcess_MS_LOF = NdExcess_MS_Q1-3*NdExcess_MS_IQR, NdExcess_MS_UOF = NdExcess_MS_Q3+3*NdExcess_MS_IQR;
//printf("\nNdExcess_MS_Q1: %f, NdExcess_MS_Q3: %f, and NdExcess_MS_IQR is %f \n", NdExcess_MS_Q1, NdExcess_MS_Q3, NdExcess_MS_IQR);
//printf("NdExcess_MS_LOF: %f, NdExcess_MS_LIF: %f, NdExcess_MS_UIF: %f,  NdExcess_MS_UOF: %f\n\n", NdExcess_MS_LOF, NdExcess_MS_LIF, NdExcess_MS_UIF,   NdExcess_MS_UOF);

int NdExcess_MS_LExtOut=0, NdExcess_MS_LMldOut=0, NdExcess_MS_UMldOut=0, NdExcess_MS_UExtOut=0;

for (i=0; i<numNodes; ++i)
{
    if (nodeExcess[i]>= NdExcess_MS_LIF)
        break;
    else if(nodeExcess[i]< NdExcess_MS_LIF & nodeExcess[i]>= NdExcess_MS_LOF)
        ++ NdExcess_MS_LMldOut;
    else if( nodeExcess[i]< NdExcess_MS_LOF)
        ++ NdExcess_MS_LExtOut;
}
//printf("broken in %d, NdExcess_MS_LExtOut: %d, NdExcess_MS_LMldOut: %d\n", i,  NdExcess_MS_LExtOut, NdExcess_MS_LMldOut);

for (i=numNodes-1; i>0; --i)
{
    if (nodeExcess[i]<= NdExcess_MS_UIF)
        break;
    if(nodeExcess[i]> NdExcess_MS_UIF & nodeExcess[i]<= NdExcess_MS_UOF)
        ++ NdExcess_MS_UMldOut;
    else if(nodeExcess[i]> NdExcess_MS_UOF)
        ++ NdExcess_MS_UExtOut;
    else
       break;
}
//printf("broken in %d, NdExcess_MS_UMldOut: %d, NdExcess_MS_UExtOut: %d\n", i,  NdExcess_MS_UMldOut, NdExcess_MS_UExtOut);



//-------------------------Sorting Nodes based on their degrees-------------------
lint nodeDgr[numNodes];
for (i=0; i<numNodes; ++i)
{
    nodeDgr[i] = adjacencyList[i].numAdjacent;
    //printf("%d  %d\n", i, nodeDgr[i] );
}

qsort((void *)nodeDgr, numNodes, sizeof(nodeDgr[0]), compare);
//qsort((void *)adjacencyList, numNodes, sizeof(adjacencyList[0]), compare);

//printf("\nsumsqrNddegree: %f\n", sumsqrNddegree);
//printf("\nAvNddegree: %f\n", AvNddegree);
//printf("\nStDNddegree: %f\n", StDNddegree);
/*
printf("\n sort nodes based on their numAdjacent:\n");
for (i=0; i<numNodes; ++i)
{
    printf("%d  %d\n", i, nodeDgr[i] );
}
*/
/* ------------------ Moore and McCabe (M&M) method ----------------------------*/

QuartIdxs(0, numNodes-1, Qrt_idx, 'M');
 int NdDgr_MM_midLIdx = Qrt_idx[0];
 int NdDgr_MM_midRIdx = Qrt_idx[1];

//printf("l is %d with cap  %d, r is %d with cap: %d", NdDgr_MM_midLIdx, nodeDgr[NdDgr_MM_midLIdx], NdDgr_MM_midRIdx, nodeDgr[NdDgr_MM_midRIdx]);

double NdDgr_MM_md = median(nodeDgr, 0, numNodes-1); // median
double NdDgr_MM_Q1 = median(nodeDgr, 0, NdDgr_MM_midLIdx); // the first quartile
double NdDgr_MM_Q3 = median(nodeDgr, NdDgr_MM_midRIdx, numNodes-1); // the third quartile
double NdDgr_MM_IQR = NdDgr_MM_Q3 - NdDgr_MM_Q1; //erquartile range

//-------------detect outliers, see: https://www.itl.nist.gov/div898/handbook/prc/section1/prc16.htm
double NdDgr_MM_LIF = NdDgr_MM_Q1-1.5*NdDgr_MM_IQR, NdDgr_MM_UIF = NdDgr_MM_Q3+1.5*NdDgr_MM_IQR,  NdDgr_MM_LOF = NdDgr_MM_Q1-3*NdDgr_MM_IQR, NdDgr_MM_UOF = NdDgr_MM_Q3+3*NdDgr_MM_IQR;

//printf("\nNdDgr_MM_md: %f \n \nNdDgr_MM_Q1: %f, NdDgr_MM_Q3: %f, and NdDgr_MM_IQR is %f \n", NdDgr_MM_md, NdDgr_MM_Q1, NdDgr_MM_Q3, NdDgr_MM_IQR);
//printf("NdDgr_MM_LOF: %f, NdDgr_MM_LIF: %f, NdDgr_MM_UIF: %f,  NdDgr_MM_UOF: %f\n\n", NdDgr_MM_LOF, NdDgr_MM_LIF, NdDgr_MM_UIF,   NdDgr_MM_UOF);

int NdDgr_MM_LExtOut=0, NdDgr_MM_LMldOut=0, NdDgr_MM_UMldOut=0, NdDgr_MM_UExtOut=0;

for (i=0; i<numNodes; ++i)
{
    if (nodeDgr[i]>= NdDgr_MM_LIF)
        break;
    else if(nodeDgr[i]< NdDgr_MM_LIF & nodeDgr[i]>= NdDgr_MM_LOF)
        ++ NdDgr_MM_LMldOut;
    else if( nodeDgr[i]< NdDgr_MM_LOF)
        ++ NdDgr_MM_LExtOut;
}
//printf("broken in %d, NdDgr_MM_LExtOut: %d, NdDgr_MM_LMldOut: %d\n", i,  NdDgr_MM_LExtOut, NdDgr_MM_LMldOut);

for (i=numNodes-1; i>0; --i)
{
    if (nodeDgr[i]<= NdDgr_MM_UIF)
        break;
    if(nodeDgr[i]> NdDgr_MM_UIF & nodeDgr[i]<= NdDgr_MM_UOF)
        ++ NdDgr_MM_UMldOut;
    else if(nodeDgr[i]> NdDgr_MM_UOF)
        ++ NdDgr_MM_UExtOut;
    else
       break;
}

//printf("broken in %d, NdDgr_MM_UMldOut: %d, NdDgr_MM_UExtOut: %d\n", i,  NdDgr_MM_UMldOut, NdDgr_MM_UExtOut);

//---------------------Tukey method-------------------------

QuartIdxs(0, numNodes-1, Qrt_idx, 'T');
int NdDgr_T_midLIdx = Qrt_idx[0];
int NdDgr_T_midRIdx = Qrt_idx[1];

double NdDgr_T_md = median(nodeDgr, 0, numNodes-1); // median
double NdDgr_T_Q1 = median(nodeDgr, 0, NdDgr_T_midLIdx); // the first quartile
double NdDgr_T_Q3 = median(nodeDgr, NdDgr_T_midRIdx, numNodes-1); // the third quartile
double NdDgr_T_IQR = NdDgr_T_Q3 - NdDgr_T_Q1; //erquartile range

//-------------detect outliers, see: https://www.itl.nist.gov/div898/handbook/prc/section1/prc16.htm
double NdDgr_T_LIF = NdDgr_T_Q1-1.5*NdDgr_T_IQR, NdDgr_T_UIF = NdDgr_T_Q3+1.5*NdDgr_T_IQR,  NdDgr_T_LOF = NdDgr_T_Q1-3*NdDgr_T_IQR, NdDgr_T_UOF = NdDgr_T_Q3+3*NdDgr_T_IQR;

//printf("\nNdDgr_T_Q1: %f, NdDgr_T_Q3: %f, and NdDgr_T_IQR is %f \n", NdDgr_T_Q1, NdDgr_T_Q3, NdDgr_T_IQR);
//printf("NdDgr_T_LOF: %f, NdDgr_T_LIF: %f, NdDgr_T_UIF: %f,  NdDgr_T_UOF: %f\n\n", NdDgr_T_LOF, NdDgr_T_LIF, NdDgr_T_UIF,   NdDgr_T_UOF);

int NdDgr_T_LExtOut=0, NdDgr_T_LMldOut=0, NdDgr_T_UMldOut=0, NdDgr_T_UExtOut=0;

for (i=0; i<numNodes; ++i)
{
    if (nodeDgr[i]>= NdDgr_T_LIF)
        break;
    else if(nodeDgr[i]< NdDgr_T_LIF & nodeDgr[i]>= NdDgr_T_LOF)
        ++ NdDgr_T_LMldOut;
    else if( nodeDgr[i]< NdDgr_T_LOF)
        ++ NdDgr_T_LExtOut;
}
//printf("broken in %d, NdDgr_T_LExtOut: %d, NdDgr_T_LMldOut: %d\n", i,  NdDgr_T_LExtOut, NdDgr_T_LMldOut);

for (i=numNodes-1; i>0; --i)
{
    if (nodeDgr[i]<= NdDgr_T_UIF)
        break;
    if(nodeDgr[i]> NdDgr_T_UIF & nodeDgr[i]<= NdDgr_T_UOF)
        ++ NdDgr_T_UMldOut;
    else if(nodeDgr[i]> NdDgr_T_UOF)
        ++ NdDgr_T_UExtOut;
    else
       break;
}

//printf("broken in %d, NdDgr_T_UMldOut: %d, NdDgr_T_UExtOut: %d\n", i,  NdDgr_T_UMldOut, NdDgr_T_UExtOut);

//**********************Mendenhall and Sincich method*************************

QuartIdxs(0, numNodes-1, Qrt_idx, 'S');
int NdDgr_MS_Q1Idx = Qrt_idx[0]-1;
int NdDgr_MS_Q3Idx = Qrt_idx[1]-1;

//printf("\nNdDgr_MS_Q1Idx is : %d and NdDgr_MS_Q3Idx is %d\n", NdDgr_MS_Q1Idx, NdDgr_MS_Q3Idx);
double NdDgr_MS_Q1 = nodeDgr[NdDgr_MS_Q1Idx];
double NdDgr_MS_Q3 = nodeDgr[NdDgr_MS_Q3Idx];
double NdDgr_MS_IQR = NdDgr_MS_Q3 - NdDgr_MS_Q1;

//-------------detect outliers --------------------------------------
double NdDgr_MS_LIF = NdDgr_MS_Q1-1.5*NdDgr_MS_IQR, NdDgr_MS_UIF = NdDgr_MS_Q3+1.5*NdDgr_MS_IQR,  NdDgr_MS_LOF = NdDgr_MS_Q1-3*NdDgr_MS_IQR, NdDgr_MS_UOF = NdDgr_MS_Q3+3*NdDgr_MS_IQR;
//printf("\nNdDgr_MS_Q1: %f, NdDgr_MS_Q3: %f, and NdDgr_MS_IQR is %f \n", NdDgr_MS_Q1, NdDgr_MS_Q3, NdDgr_MS_IQR);
//printf("NdDgr_MS_LOF: %f, NdDgr_MS_LIF: %f, NdDgr_MS_UIF: %f,  NdDgr_MS_UOF: %f\n\n", NdDgr_MS_LOF, NdDgr_MS_LIF, NdDgr_MS_UIF,   NdDgr_MS_UOF);

int NdDgr_MS_LExtOut=0, NdDgr_MS_LMldOut=0, NdDgr_MS_UMldOut=0, NdDgr_MS_UExtOut=0;

for (i=0; i<numNodes; ++i)
{
    if (nodeDgr[i]>= NdDgr_MS_LIF)
        break;
    else if(nodeDgr[i]< NdDgr_MS_LIF & nodeDgr[i]>= NdDgr_MS_LOF)
        ++ NdDgr_MS_LMldOut;
    else if( nodeDgr[i]< NdDgr_MS_LOF)
        ++ NdDgr_MS_LExtOut;
}
//printf("broken in %d, NdDgr_MS_LExtOut: %d, NdDgr_MS_LMldOut: %d\n", i,  NdDgr_MS_LExtOut, NdDgr_MS_LMldOut);

for (i=numNodes-1; i>0; --i)
{
    if (nodeDgr[i]<= NdDgr_MS_UIF)
        break;
    if(nodeDgr[i]> NdDgr_MS_UIF & nodeDgr[i]<= NdDgr_MS_UOF)
        ++ NdDgr_MS_UMldOut;
    else if(nodeDgr[i]> NdDgr_MS_UOF)
        ++ NdDgr_MS_UExtOut;
    else
       break;
}
//printf("broken in %d, NdDgr_MS_UMldOut: %d, NdDgr_MS_UExtOut: %d\n", i,  NdDgr_MS_UMldOut, NdDgr_MS_UExtOut);

// ----------------------------------printing outputs-----------------------------

	printf ("Nodes, Arcs, NetCap, maxcap, mincap, MaxExcess, MinExcess, MaxInnerNdDg, MinInnerNdDg, SrcAAC, SnkAAC,  Src_degree, Snk_degree,");
	printf ("NmBdCap, NmGdCap, PotNetExcess, PotNetDeficit, AvAC, StDAC, AvNddegree, AvInnerNddegree, StDNddegree, AvPotNetExcess, StDPotNetExcess,  AvPotNetDeficit, StDPotNetDeficit,");

	printf ("AC_MM_md, AC_MM_Q1, AC_MM_Q3, AC_MM_IQR, AC_MM_LOF, AC_MM_LIF, AC_MM_UIF, AC_MM_UOF, AC_MM_LExtOut, AC_MM_LMldOut, AC_MM_UMldOut, AC_MM_UExtOut,");
	printf ("AC_T_Q1, AC_T_Q3, AC_T_IQR, AC_T_LOF, AC_T_LIF, AC_T_UIF, AC_T_UOF, AC_T_LExtOut, AC_T_LMldOut, AC_T_UMldOut, AC_T_UExtOut,");
	printf ("AC_MS_Q1, AC_MS_Q3, AC_MS_IQR, AC_MS_LOF, AC_MS_LIF, AC_MS_UIF, AC_MS_UOF, AC_MS_LExtOut, AC_MS_LMldOut, AC_MS_UMldOut, AC_MS_UExtOut,");

	printf ("NdExcess_MM_md, NdExcess_MM_Q1, NdExcess_MM_Q3, NdExcess_MM_IQR, NdExcess_MM_LOF, NdExcess_MM_LIF, NdExcess_MM_UIF, NdExcess_MM_UOF, NdExcess_MM_LExtOut, NdExcess_MM_LMldOut, NdExcess_MM_UMldOut, NdExcess_MM_UExtOut,");
	printf ("NdExcess_T_Q1, NdExcess_T_Q3, NdExcess_T_IQR, NdExcess_T_LOF, NdExcess_T_LIF, NdExcess_T_UIF, NdExcess_T_UOF, NdExcess_T_LExtOut, NdExcess_T_LMldOut, NdExcess_T_UMldOut, NdExcess_T_UExtOut,");
	printf ("NdExcess_MS_Q1, NdExcess_MS_Q3, NdExcess_MS_IQR, NdExcess_MS_LOF, NdExcess_MS_LIF, NdExcess_MS_UIF, NdExcess_MS_UOF, NdExcess_MS_LExtOut, NdExcess_MS_LMldOut, NdExcess_MS_UMldOut, NdExcess_MS_UExtOut,");

	printf ("NdDgr_MM_md, NdDgr_MM_Q1, NdDgr_MM_Q3, NdDgr_MM_IQR, NdDgr_MM_LOF, NdDgr_MM_LIF, NdDgr_MM_UIF, NdDgr_MM_UOF, NdDgr_MM_LExtOut, NdDgr_MM_LMldOut, NdDgr_MM_UMldOut, NdDgr_MM_UExtOut,");
	printf ("NdDgr_T_Q1, NdDgr_T_Q3, NdDgr_T_IQR, NdDgr_T_LOF, NdDgr_T_LIF, NdDgr_T_UIF, NdDgr_T_UOF, NdDgr_T_LExtOut, NdDgr_T_LMldOut, NdDgr_T_UMldOut, NdDgr_T_UExtOut,");
	printf ("NdDgr_MS_Q1, NdDgr_MS_Q3, NdDgr_MS_IQR, NdDgr_MS_LOF, NdDgr_MS_LIF, NdDgr_MS_UIF, NdDgr_MS_UOF, NdDgr_MS_LExtOut, NdDgr_MS_LMldOut, NdDgr_MS_UMldOut, NdDgr_MS_UExtOut\n");


	printf ("%u, %u, %f, %f, %f, %d, %d, %d, %d, %ld, %ld, %d, %d, ", numNodes, numArcs, NetCap, maxcap, mincap, MaxExcess, MinExcess, MaxInnerNdDg, MinInnerNdDg, SrcAAC, SnkAAC,  Src_degree, Snk_degree);
	printf ("%f, %f, %ld, %ld, %f, %f, %f, %f, %f, %f, %f, %f, %f,", BdPerCap, GdPerCap, PotNetExcess, PotNetDeficit, AvAC, StDAC, AvNddegree, AvInnerNddegree, StDNddegree, AvPotNetExcess, StDPotNetExcess,  AvPotNetDeficit, StDPotNetDeficit);

	printf ("%f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,", AC_MM_md, AC_MM_Q1, AC_MM_Q3, AC_MM_IQR, AC_MM_LOF, AC_MM_LIF, AC_MM_UIF, AC_MM_UOF, AC_MM_LExtOut, AC_MM_LMldOut, AC_MM_UMldOut, AC_MM_UExtOut);
	printf (" %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,", AC_T_Q1, AC_T_Q3, AC_T_IQR, AC_T_LOF, AC_T_LIF, AC_T_UIF, AC_T_UOF, AC_T_LExtOut, AC_T_LMldOut, AC_T_UMldOut, AC_T_UExtOut);
    printf ("%f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,", AC_MS_Q1, AC_MS_Q3, AC_MS_IQR, AC_MS_LOF, AC_MS_LIF, AC_MS_UIF, AC_MS_UOF, AC_MS_LExtOut, AC_MS_LMldOut, AC_MS_UMldOut, AC_MS_UExtOut);

    printf ("%f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,", NdExcess_MM_md, NdExcess_MM_Q1, NdExcess_MM_Q3, NdExcess_MM_IQR, NdExcess_MM_LOF, NdExcess_MM_LIF, NdExcess_MM_UIF, NdExcess_MM_UOF, NdExcess_MM_LExtOut, NdExcess_MM_LMldOut, NdExcess_MM_UMldOut, NdExcess_MM_UExtOut);
	printf ("%f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,",  NdExcess_T_Q1, NdExcess_T_Q3, NdExcess_T_IQR, NdExcess_T_LOF, NdExcess_T_LIF, NdExcess_T_UIF, NdExcess_T_UOF, NdExcess_T_LExtOut, NdExcess_T_LMldOut, NdExcess_T_UMldOut, NdExcess_T_UExtOut);
    printf ("%f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,",  NdExcess_MS_Q1, NdExcess_MS_Q3, NdExcess_MS_IQR, NdExcess_MS_LOF, NdExcess_MS_LIF, NdExcess_MS_UIF, NdExcess_MS_UOF, NdExcess_MS_LExtOut, NdExcess_MS_LMldOut, NdExcess_MS_UMldOut, NdExcess_MS_UExtOut);

    printf ("%f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,", NdDgr_MM_md, NdDgr_MM_Q1, NdDgr_MM_Q3, NdDgr_MM_IQR, NdDgr_MM_LOF, NdDgr_MM_LIF, NdDgr_MM_UIF, NdDgr_MM_UOF, NdDgr_MM_LExtOut, NdDgr_MM_LMldOut, NdDgr_MM_UMldOut, NdDgr_MM_UExtOut);
	printf ("%f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d,", NdDgr_T_Q1, NdDgr_T_Q3, NdDgr_T_IQR, NdDgr_T_LOF, NdDgr_T_LIF, NdDgr_T_UIF, NdDgr_T_UOF, NdDgr_T_LExtOut, NdDgr_T_LMldOut, NdDgr_T_UMldOut, NdDgr_T_UExtOut);
    printf ("%f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d", NdDgr_MS_Q1, NdDgr_MS_Q3, NdDgr_MS_IQR, NdDgr_MS_LOF, NdDgr_MS_LIF, NdDgr_MS_UIF, NdDgr_MS_UOF, NdDgr_MS_LExtOut, NdDgr_MS_LMldOut, NdDgr_MS_UMldOut, NdDgr_MS_UExtOut);



/*

AC, NDDgree, NdExcess
MM_md
MM: MM_Q1, MM_Q3, MM_IQR, MM_LOF, MM_LIF, MM_UIF, MM_UOF, MM_LExtOut, MM_LMldOut, MM_UMldOut, MM_UExtOut
T: T_Q1, T_Q3, T_IQR, T_LOF, T_LIF, T_UIF,   T_UOF, T_LExtOut, T_LMldOut, T_UMldOut, T_UExtOut
MS: MS_Q1, MS_Q3, MS_IQR, MS_LOF, MS_LIF, MS_UIF,   MS_UOF, MS_LExtOut, MS_LMldOut, MS_UMldOut, MS_UExtOut

AvPotNetExcess, StDPotNetExcess
 AvPotNetDeficit, StDPotNetDeficit

int AC_MM_midLIdx = Qrt_idx[0];
int AC_MM_midRIdx = Qrt_idx[1];
double AC_MM_md = median(capArcs, 0, numArcs-1); // median
double AC_MM_Q1 = median(capArcs, 0, AC_MM_midLIdx); // the first quartile
double AC_MM_Q3 = median(capArcs, AC_MM_midRIdx, numArcs-1); // the third quartile
double AC_MM_IQR = AC_MM_Q3 - AC_MM_Q1; //interquartile range
double AC_MM_LIF = AC_MM_Q1-1.5*AC_MM_IQR, AC_MM_UIF = AC_MM_Q3+1.5*AC_MM_IQR,  AC_MM_LOF = AC_MM_Q1-3*AC_MM_IQR, AC_MM_UOF = AC_MM_Q3+3*AC_MM_IQR;
int AC_MM_LExtOut=0, AC_MM_LMldOut=0, AC_MM_UMldOut=0, AC_MM_UExtOut=0;
*/

//------------------------------------------------------------
	freeMemory ();
	return 0;
} //end of Main loop
