//*************************************************************************************
//  
//  Author:  Don A. Grundel
//  e-mail:  grundel@eglin.af.mil
//
//  This program will generate a Multidimensional Assignment Problem
//
//
//  The output file is testproblem.txt.  The first line is the dimension of the
//  problem.  The next line is the number of elements for each dimension.  This line
//  is followed by the cost elements in row major order.  For example, a 3x4x5 problem 
//  looks like:
/*	
				3
				3 4 5
				29
				17
				13
				.
				.
				.
				.

    This problem has 3x4x5=60 cost elements.
*/

//  The program uses an index tree as described by:
//		W. Pierskalla, The multidimensional assignment
//		problem, Operations Research, 16 pp. 422-431, 1968.

//  This code is made available in the hope that it will be useful, 
//  but without any warranty.
//
//****************************************************************************************

# include <iostream>
# include <fstream>
# include <time.h>
# include <stdlib.h>
# include <sstream>

using namespace std;

//MVG
// ------------------------------------------------------------------------- //
inline string UtilIntToStr(const int i){
   stringstream ss;
   ss << i;
   return ss.str();
}



struct DataNode {  // these nodes will be used to set up the dataArray.  There is a node for each assignement
	int cost;      // this is an assignment cost
	int optLB;     // this is the lower bound on cost 
	struct ElementNode* next;    // next is the first element of the assignment.  Each node will be a linked list 
								 // that includes the list of elements in the assignment.  It is arranged as
								 // i j k etc.
};

struct ElementNode {  // these nodes will be used to carry the elements of each assignement
	int element;
	struct ElementNode* next;
};

// RandomOptPath is used to randomly build the optimal path
void RandomOptPath(DataNode* dataArray[], int optimalPath[], int optLBArray[], int kMinArray[], int kMaxArray[], int n, int randomCostLow, int randomCostHigh);

// CheckPath is used to check the feasibility of a node at the next level down in the index tree
int CheckPath(int Path[], int n, int index, DataNode* dataArray[]);


int main(int argc, char ** argv) {

	int m = 0;				// this is the dimension of MAP.  I.e. 3 or 4 or 5, etc.
	int numNodes = 1;		// this is the total number of assignements in the problem.  It is
							// the product of the size of the problem.  For example, a 3 x 4 x 5 is 60.
	int i; 
	int count;				// used as a counter in some loops
	int location;			// used as variable to indicate location in row major array.  Location = 0 for first element.
	int level;				// used to indicate level in index tree.  level = 0 at root.  level = 1 for all nodes begining with element 1 = 1
	int randomCostLow = 1;	// lower and upper bounds on randomized costs for each assignment
	int randomCostHigh = 10;
	
	//srand(time(NULL));		// will be used for random number generation
        //MVG
        int seed = atoi(argv[1]);
        srand(seed);

#if 0
	cout << "This program will generate a fully dense multidimensional\n"
        << "assignement problem with a known unique solution.  The\n"
        << "problem generates random costs with random assignments.\n"
        << "The costs may be adjusted from the current\n"
		<< "setting of integers from a uniform on [1,10].  The program will\n"
        << "ask for the number of dimensions and ask for the number of\n"
        << "elements for each dimension.  Start small as the problem\n"
        << "grows exponential with respect to the dimension.\n\n\n";

	cout << "Enter the number of dimensions.\n";
#endif

        //MVG
	//cin >> m ;
        m = 3;

	int* numDim = new int [m];		  // this is an array that contains number
									  // items in each alternative.  The first
									  // index is that of the alternative with
									  // smallest number of elements.  Each
									  // element in the first alternative must be
									  // totally assigned.

#if 0
	cout << "\nEnter the number of elements for each dimension.\n"
			<< "The first dimension must have the fewest number of elements.\n"
			<< "In general n1 <= n2 <= ... <= nd.\n"
            << "The number of elements need to be positive integers.\n"
			<< "As an example, 3 x 4 x 4 x 5 is entered as 3 4 4 5.\n\n";
#endif

	for (i = 0; i < m; i++) {
           //MVG
           //		cin >> numDim[i];
           numDim[i] = atoi(argv[2]);
		numNodes = numDim[i]*numNodes;  // this uses the formula: 3 x 4 x 4 x 5 = 240
	}

	
	int* conversionArray = new int [m]; // this array is used to convert from and to the row major array

	conversionArray[m-1] = 1;           // initialize the conversion array
	for (i = m - 1; i >= 0; i--) {
		conversionArray[i-1] = conversionArray[i]*numDim[i];
	}

	int* optLocations = new int [numDim[0]];  // array to hold location of opt assignments in the costArray



//	Create dataArray with indexes;  This is an array of pointers to the DataNode

	DataNode** dataArray = new DataNode* [numNodes + 1];  // creates a new data array of pointers to DataNodes
	dataArray[0] = new DataNode;
	dataArray[0]->cost=0;
	dataArray[0]->optLB=0;
	dataArray[0]->next = NULL;

// now fill the dataArray and linked lists for each node

	ElementNode* t;
	count = 1;
	for (i = 1; i <= numNodes; i++) {
		location = count - 1;			// this is actual location in the row major vector
		dataArray[count] = new DataNode;
		dataArray[count]->cost = 0;
		dataArray[count]->optLB = 0;
		dataArray[count]->next = NULL;
		for (int j = m - 1; j >=0 ; j--)  {     // this loop fills the link list of assignment elements
			t = dataArray[count]->next;      // the elements are listed as i,j,k, etc. when done
			dataArray[count]->next = new ElementNode;   // by inserting new elements right after the first datanode
			dataArray[count]->next->element = ((location/conversionArray[j])%numDim[j])+1;
			location = location - (dataArray[count]->next->element-1)*conversionArray[j];
			dataArray[count]->next->next = t;
			t = NULL;
		}
	count = count + 1;
	}

// end filling the dataArray

	int* kArray = new int [numDim[0]+1];  // array to hold node index at each level
													   // for example kArray[0] will be 0 and kArray[1] will 
													   // hold the node index at level 1 in index tree.  
	int* kMinArray = new int [numDim[0]+1];  // array to hold min node index at each level
														  // for example kMinArray[0] be null and kMinArray[1] will be 1 
														  // kMinArray[1] = 1+(1)*(numberNodes/numberEachAlternative[0])
	int* kMaxArray = new int [numDim[0]+1]; // array to hold max node index at each level
														// for example kMaxArray[0] will be null and
													    // kMaxArray[1] = (1)*numberNodes/numberEachAlternative[0]
														// kMaxArray[2] = (2)*numberNodes/numberEachAlternative[0]
// initialize arrays
	kArray[0] = kMinArray[0] = kMaxArray[0] = 0;
    for (count = 1; count <= numDim[0]; count++) {
		kMinArray[count] = 1 + (count - 1)*(numNodes/numDim[0]);
		kMaxArray[count] = (count)*(numNodes/numDim[0]);
		kArray[count]=kMinArray[count]-1;               // this is set at the node just below min at that level
	}

// select optimal assignments and costs.

	int* optimalPath = new int [numDim[0]]; // this is an array of nodes in order of the optimal path
	int* optLBArray = new int [numDim[0]];  // an array to hold the opt lower bound at each level
	int optCost = 0;						// opt cost is null at this point

	for (count = 0; count < numDim[0]; count++)
		optimalPath[count] = 0;		

 	RandomOptPath(dataArray, optimalPath, optLBArray, kMinArray, kMaxArray, numDim[0], randomCostLow, randomCostHigh);

// Fill lower level of index tree

	int isItFeasible;
	int tempPath[1] = {optimalPath[numDim[0]-2]};
	level = numDim[0];

	for (i = kMinArray[level] ; i <= kMaxArray[level] ; i++) {
		if (i != optimalPath[level-1]) {
			isItFeasible = CheckPath(tempPath, 1, i, dataArray);
			if (isItFeasible == 1) {
				dataArray[i]->cost = dataArray[i]->optLB = optLBArray[level-1] + 1 + rand()%(randomCostHigh);
			}
			else {
				dataArray[i]->cost = dataArray[i]->optLB = randomCostLow + rand()%(randomCostHigh - randomCostLow);
			}
		}
	}

// end filling lower level of index tree

// fill rest of tree working from 2nd lowest level on up

	int minOptLB;
	for (level = numDim[0]-1 ; level >= 1 ; level--) {
		for (i = kMinArray[level] ; i <= kMaxArray[level] ; i++) {
			if (i != optimalPath[level - 1]) {
				minOptLB = 100000;
				tempPath[0] = i;
				for (int j = kMinArray[level+1] ; j <= kMaxArray[level+1] ; j++) {
					isItFeasible = CheckPath(tempPath, 1, j, dataArray);
					if (isItFeasible == 1) {
						if (dataArray[j]->optLB < minOptLB) minOptLB = dataArray[j]->optLB;
					}
				}
				dataArray[i]->optLB = optLBArray[level-1] + 1 + rand()%(randomCostHigh+randomCostHigh/2);   // adjustment is made here for spread of costs
				dataArray[i]->cost = dataArray[i]->optLB - minOptLB;
			}
		}
	}


// end filling rest of tree

        //MVG 
        string fileNameSol = "ap3.";
        fileNameSol += UtilIntToStr(numDim[0]);
        fileNameSol += ".";
        fileNameSol += UtilIntToStr(seed);
        fileNameSol += ".sol";
        ofstream examplefileSol (fileNameSol.c_str());
        //ofstream examplefile ("testproblem.txt");
	if (examplefileSol.is_open()) {
           examplefileSol << "\nOPT: " << dataArray[optimalPath[0]]->optLB << "\n";
           for (i = 0; i < numDim[0]; i++) {
              ElementNode* current = dataArray[optimalPath[i]]->next;
              while (current != NULL) {
                 examplefileSol << current->element << " ";
                 current = current->next;
              }
              examplefileSol << endl;
              //cout << " at cost of " << dataArray[optimalPath[i]]->cost << endl;
           }
           examplefileSol << endl;
           examplefileSol.close();
	}
#if 0
	cout << "\nThe optimal solution cost is " << dataArray[optimalPath[0]]->optLB << " using the following assignments\n";
	for (i = 0; i < numDim[0]; i++) {
		ElementNode* current = dataArray[optimalPath[i]]->next;
		while (current != NULL) {
			cout << current->element << " ";
			current = current->next;
		}
		cout << " at cost of " << dataArray[optimalPath[i]]->cost << endl;
	}
	cout << endl;
#endif


        //MVG 
        string fileName = "ap3.";
        fileName += UtilIntToStr(numDim[0]);
        fileName += ".";
        fileName += UtilIntToStr(seed);
        fileName += ".txt";
        ofstream examplefile (fileName.c_str());
        //ofstream examplefile ("testproblem.txt");
	if (examplefile.is_open()) {
		examplefile << m << "\n";
		for (i = 0; i < m; i++) {
			examplefile << numDim[i] << " ";
		}
		examplefile << "\n";
		for (i = 1; i <= numNodes; i++) {
			examplefile << dataArray[i]->cost << endl;
		}
	examplefile.close();
	}

	return 0;
}


void RandomOptPath(DataNode* dataArray[], int optimalPath[], int optLBArray[], int kMinArray[], int kMaxArray[], int n, int randomCostLow, int randomCostHigh)
{
   int i;
   int index;
	int isItFeasible = 1;
	
	optimalPath[0] =   kMinArray[1] + rand()%(kMaxArray[1] - kMinArray[1] + 1);  // randomly select first node on optPath
	dataArray[optimalPath[0]]->cost = randomCostLow + rand()%(randomCostHigh - randomCostLow);

	for (i = 1 ; i < n ; i++) {		// sets the optimal path and picks some costs.  Goes level by level
		while(1) {
			index =   kMinArray[i+1] + rand()%(kMaxArray[i+1] - kMinArray[i+1] + 1);  // randomly select optPath
			isItFeasible = CheckPath(optimalPath, n, index, dataArray);
			if (isItFeasible == 1) {
				optimalPath[i] = index;
				break;
			}
		}
		dataArray[optimalPath[i]]->cost = randomCostLow + rand()%(randomCostHigh - randomCostLow);
	}
		
	optLBArray[n-1] = dataArray[optimalPath[n-1]]->optLB = dataArray[optimalPath[n-1]]->cost;  // set opt lower bounds, works from bottom up
		
	for (i = n - 2 ; i >= 0; i--) {
		optLBArray[i] = dataArray[optimalPath[i]]->optLB = optLBArray[i+1] + dataArray[optimalPath[i]]->cost;
	}
}

int CheckPath(int Path[], int n, int index, DataNode* dataArray[])
{  // index is node under consideration.  This simply checks to see if any node in the current path has an element in common
	// with the new node under consideration.
	int feasible = 1;  // 1 is true, 0 is false
	for (int i = 0; i < n; i++) {
		ElementNode* current1 = dataArray[Path[i]]->next;
		ElementNode* current2 = dataArray[index]->next;
		while (current1 != NULL) {
			if (current1->element == current2->element) {
				feasible = 0;
				break;
			}
			current1 = current1->next;
			current2 = current2->next;
		}
		if (feasible == 0) break;
	}
	return feasible;
}


