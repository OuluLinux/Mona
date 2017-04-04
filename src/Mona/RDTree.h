#ifndef _Mona_RDTree_h_
#define _Mona_RDTree_h_

#include "Common.h"

class Receptor;
class Mona;

typedef double SENSOR;

// Mona: sensory/response, neural network, and needs.
class RDTree : Moveable<RDTree> {
public:

	// Tree configuration parameter.
	static const double DEFAULT_RADIUS;
	double              RADIUS;

	// Tree node.
	class RDNode {
	public:
		Vector<SENSOR> pattern;  // pattern value
		Receptor* client;        // client link
	private:
		RDNode* outer_list;      // outer pattern list
		RDNode* outer_last;      // last outer
		RDNode* equal_next;      // next sibling
		RDNode* equal_prev;      // previous sibling
		double  distance;        // distance from outer to parent
	public:
		RDNode(const Vector<SENSOR>& pattern, Receptor* client);
		RDNode();
		friend class RDTree;
	};

	// Search element.
	class RDSearch {
	public:
		RDNode*   node;           // node
		double    distance;       // comparison distance
		RDSearch* search_next;    // next on search return list
	private:
		double    workdist;       // work distance
		int      state;           // search state
		RDSearch* outer_list;     // outer
		RDSearch* equal_next;        // next sibling
		RDSearch* equal_prev;        // previous sibling
	public:
		RDSearch();
		friend class RDTree;
	};

	// Constructors.
	RDTree();
	//RDTree(double (*dist_func)(void*, void*), void (*del_func)(void*) = NULL);
	//RDTree(double radius, double (*dist_func)(void*, void*), void (*del_func)(void*) = NULL);
	void Init(double (*dist_func)(const Vector<SENSOR>&, const Vector<SENSOR>&), void (*del_func)(Vector<SENSOR>&) = NULL);
	void Init(double radius, double (*dist_func)(const Vector<SENSOR>&, const Vector<SENSOR>&), void (*del_func)(Vector<SENSOR>&) = NULL);
	void Connect(double(*dist_func)(const Vector<SENSOR>&, const Vector<SENSOR>&), void(*del_func)(Vector<SENSOR>&));
	
	// Destructor.
	~RDTree();
	void DeleteSubtree(RDNode*);

	// Insert, remove, and search.
	void Insert(const Vector<SENSOR>& pattern, Receptor& client);
	void Remove(const Vector<SENSOR>& pattern);
	RDSearch* Search(const Vector<SENSOR>& pattern, int max_find = 1, int max_search = (-1));

	// Load and save tree.
	/*void Load(Stream& s, void* (*load_pattern)(Stream& s),
			  Receptor* (*load_client)(Stream& s) = NULL);*/
	void Load(Stream& s, Mona& mona,
			  void (*load_pattern)(Mona& mona, Vector<SENSOR>& sensors, Stream& s),
			  void* (*load_client)(Mona& mona, Stream& s) = NULL);
	void Store(Stream& s, void (*store_pattern)(const Vector<SENSOR>& pattern, Stream& fp),
			   void (*store_client)(void* client, Stream& fp) = NULL);

	// Print.
	//bool Print(String filename, void (*print_pattern)(void *pattern, Stream& fp));
	//void Print(void (*print_pattern)(void *pattern, Stream& fp), Stream& s);

private:

	// Tree root.
	RDNode* root;

	// Pattern distance function.
	double (*dist_func)(const Vector<SENSOR>& pattern0, const Vector<SENSOR>& pattern1);

	// Pattern delete function.
	void (*del_func)(Vector<SENSOR>& pattern);

	// Search states.
	enum {
		DISTPENDING = 0,                        // distance pending
		DISTDONE    = 1,                        // distance computed
		EXPANDED    = 2,                        // pattern expanded
		SRCHDONE    = 3                         // pattern searched
	};

	static RDSearch* EMPTY;

	// Search stack.
	enum { STKMEM_QUANTUM = 32 };               // stack malloc increment
	int stkMem;
	struct SrchStk {
		RDSearch* current_search;
		RDSearch* outer;
		RDSearch* outer_next;
	};

	// Search work memory.
	enum { SRCHWORKMEM_QUANTUM = 128 };         // search work memory size

	// Search control.
	struct SearchCtrl {
		RDSearch*           search_list;        // search results
		RDSearch*           search_best;        // best search result
		int                max_find;            // max number of results
		int                max_search;          // max nodes to search (-1=unlimited)
		int                search_count;        // search counter
		int                best_search;         // best result search counter
		struct SrchStk*     search_stack;            // search stack
		int                search_stack_idx;         // stack index
		int                search_stack_sz;          // current stack size
		Vector<RDSearch*> search_work;          // search work space
		int                search_work_idx;     // search work index
		int                search_work_use;     // used search work
	};

	// Internal functions.
	void Insert(RDNode* current, RDNode& node);
	void Search(struct SearchCtrl* search_ctrl, RDNode* search_node);
	void FoundPattern(struct SearchCtrl* search_ctrl, RDSearch* sw_found, int* found_count, RDSearch** sw_cut);
	RDSearch* GetSearchWork(struct SearchCtrl* search_ctrl);

	/*void LoadOuter(Stream& s, RDNode* parent, void* (*load_pattern)(Stream& s),
				   Receptor* (*load_client)(Stream& s));*/
	void LoadOuter(Stream& s, Mona& mona, RDNode* parent,
				   void (*load_pattern)(Mona& mona, Vector<SENSOR>& sensors, Stream& s),
				   void* (*load_client)(Mona& mona, Stream& s) = NULL);
	void StoreOuter(Stream& s, RDNode* parent,
					void (*store_pattern)(const Vector<SENSOR>& pattern, Stream& fp),
					void (*store_client)(void* client, Stream& fp) = NULL);
	void PrintNode(Stream& s, RDNode* node, int level,
				   void (*print_pattern)(void* pattern, Stream& fp));
};
#endif
