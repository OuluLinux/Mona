// Common header

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <algorithm>
using namespace std;
using std::string;

#include "FileIO.h"
#include "GetTime.h"
#include "Log.h"
#include "Matrix.h"
#include "MD5.h"
#include "Quaternion.h"
#include "Random.h"
#include "Vector.h"
#include "ValueSet.h"
#include "RDTree.h"

#define DONT_CARE      (-1)
#define NEARLY_ZERO    0.0001
