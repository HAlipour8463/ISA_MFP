
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
			// adjacencyList[to-1].incomingCapcacity += capacity;
			adjacencyList[to-1].excess += capacity;

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


// BdPerCap = (double)BdPerCap / (double)numArcs;
// GdPerCap = (double)GdPerCap / (double)numArcs; 	


// ---------------------------------------------------------------------------------------------------


for (i=0; i<numArcs; ++i)
{
	
maxcap = (maxcap > arcList[i].capacity) ? maxcap : arcList[i].capacity;
mincap = (mincap < arcList[i].capacity) ? mincap : arcList[i].capacity;


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


//------------ Standard deviation of the AC ------------------------------------------------------------


for (i=0; i<numArcs; ++i)
{
	sumsqr += pow((arcList[i].capacity - AvAC), 2);
	Cmpsumsqr += pow((arcList[i].capacity - CmpAvAC), 2);

}

Cmpsumsqr += ((numNodes*(numNodes-1))-numArcs)*pow(CmpAvAC, 2);

StDAC = sqrt((double)sumsqr / (double)numArcs);
CmpStDAC = sqrt((double)(sumsqr ) / (double)(numNodes*(numNodes-1)));


//------------ Potential excess of each node ------------------------------------------------------------


for (i=0; i<numNodes; i++)
{
	// PotNetExcess +=adjacencyList[i].excess;

	if (adjacencyList[i].excess > 0)
	{
	PotNetExcess +=adjacencyList[i].excess;
	// printf("Excess of node %ld is: %ld\n", i+1, adjacencyList[i].excess);
	// printf("PotNetExcess is : %ld\n", PotNetExcess);
	}
	else if (adjacencyList[i].excess < 0)
	{
	PotNetDeficit -=adjacencyList[i].excess;
	// printf("Deficit of node %ld is: %ld\n", i+1, adjacencyList[i].excess);
	// printf("PotNetDeficit is : %ld\n", PotNetDeficit);

	}


}

 //printf ("sink excess %d is:    %d\n", i, adjacencyList[sink].excess);
 PotNetExcess -= adjacencyList[sink-1].excess;
 PotNetDeficit += adjacencyList[source-1].excess;



 //printf ("PotNetExcess is:     %d\n", PotNetExcess);





// ---------------------------------------------------------------------------------------

	


	printf ("Nodes, Arcs, NetCap, maxcap, mincap, SrcAAC, SnkAAC,  Src_degree, Snk_degree, StDAC, ");
	printf ("NmBdCap, NmGdCap, PotNetExcess, PotNetDeficit\n");
	
	printf ("%u, %u, %f, %f, %f, %ld, %ld, %d, %d, %f, ",   numNodes, numArcs, NetCap, maxcap, mincap, SrcAAC, SnkAAC,  Src_degree, Snk_degree, StDAC);
	printf ("%f, %f, %ld, %ld", BdPerCap, GdPerCap, PotNetExcess, PotNetDeficit);



	freeMemory ();

	return 0;
}
