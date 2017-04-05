// Common header

#include <Core/Core.h>
using namespace Upp;

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
#define INVALID_RANDOM    0xffffffffUL
#define INVALID_TIME -1

/*#define FREAD_INT(ptr, s)			s.Get(ptr, sizeof(int))
#define FWRITE_INT(ptr, s)			s.Put(ptr, sizeof(int))
#define FREAD_SHORT(ptr, s)			s.Get(ptr, sizeof(short))
#define FWRITE_SHORT(ptr, s)		s.Put(ptr, sizeof(short))
#define FREAD_LONG(ptr, s)			s.Get(ptr, sizeof(long))
#define FWRITE_LONG(ptr, s)			s.Put(ptr, sizeof(long))
#define FREAD_LONG_LONG(ptr, s)		s.Get(ptr, sizeof(long long))
#define FWRITE_LONG_LONG(ptr, s)	s.Put(ptr, sizeof(long long))
#define FREAD_FLOAT(ptr, s)			s.Get(ptr, sizeof(float))
#define FWRITE_FLOAT(ptr, s)		s.Put(ptr, sizeof(float))
#define FREAD_DOUBLE(ptr, s)		s.Get(ptr, sizeof(double))
#define FWRITE_DOUBLE(ptr, s)		s.Put(ptr, sizeof(double))
#define FREAD_BOOL(ptr, s)			s.Get(ptr, sizeof(bool))
#define FWRITE_BOOL(ptr, s)			s.Put(ptr, sizeof(bool))
#define FREAD_CHAR(ptr, s)			s.Get(ptr, sizeof(char))
#define FWRITE_CHAR(ptr, s)			s.Put(ptr, sizeof(char))
#define FREAD_BYTES(ptr, size, s)	s.Get(ptr, size)
#define FWRITE_BYTES(ptr, size, s)	s.Put(ptr, size)
#define FREAD_STRING(ptr, size, s)	*ptr = s.Get(size);
#define FWRITE_STRING(ptr, size, s)	s.Put(*ptr);*/

double RandomInterval(double min, double max);
bool RandomChance(double p);
bool RandomBoolean();
