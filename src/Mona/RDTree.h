#ifndef __PATTREE__
#define __PATTREE__


// Mona: sensory/response, neural network, and needs.
class RDTree {
public:

   // Tree configuration parameter.
   static const float DEFAULT_RADIUS;
   float              RADIUS;

   // Tree node.
   class RDNode
   {
public:
      void *pattern;           /* pattern value */
      void *client;            /* client link */
private:
      RDNode *outer_list;      /* outer pattern list */
      RDNode *outer_last;      /* last outer */
      RDNode *equal_next;      /* next sibling */
      RDNode *equal_prev;      /* previous sibling */
      float  distance;         /* distance from outer to parent */
public:
      RDNode(void *pattern, void *client);
      RDNode();
      friend class RDTree;
   };

   // Search element.
   class RDSearch
   {
public:
      RDNode   *node;           /* node */
      float    distance;        /* comparison distance */
      RDSearch *search_next;       /* next on search return list */
private:
      float    workdist;        /* work distance */
      int      state;           /* search state */
      RDSearch *outer_list;      /* outerren */
      RDSearch *equal_next;        /* next sibling */
      RDSearch *equal_prev;        /* previous sibling */
public:
      RDSearch();
      friend class RDTree;
   };

   // Constructors.
   RDTree(float (*dist_func)(void *, void *), void (*del_func)(void *) = NULL);
   RDTree(float radius, float (*dist_func)(void *, void *), void (*del_func)(void *) = NULL);

   // Destructor.
   ~RDTree();
   void DeleteSubtree(RDNode *);

   // Insert, remove, and search.
   void Insert(void *pattern, void *client);
   void Remove(void *pattern);
   RDSearch *Search(void *pattern, int max_find = 1, int max_search = (-1));

   // Load and save tree.
   bool Load(char *filename, void *(*load_pattern)(Stream& s),
             void *(*load_client)(Stream& s) = NULL);
   void Load(Stream& s, void *(*load_pattern)(Stream& s),
             void *(*load_client)(Stream& s) = NULL);
   bool Load(char *filename, void *helper,
             void *(*load_pattern)(void *helper, Stream& s),
             void *(*load_client)(void *helper, Stream& s) = NULL);
   void Load(Stream& s, void *helper,
             void *(*load_pattern)(void *helper, Stream& s),
             void *(*load_client)(void *helper, Stream& s) = NULL);
   bool Store(char *filename, void (*savePatt)(void *pattern, FILE *fp),
             void (*saveClient)(void *client, FILE *fp) = NULL);
   void Store(Stream& s, void (*savePatt)(void *pattern, FILE *fp),
             void (*saveClient)(void *client, FILE *fp) = NULL);

   // Print.
   bool Print(char *filename, void (*print_pattern)(void *pattern, FILE *fp));
   void Print(void (*print_pattern)(void *pattern, FILE *fp), Stream& s = stdout);

private:

   // Tree root.
   RDNode *root;

   // Pattern distance function.
   float (*dist_func)(void *pattern0, void *pattern1);

   // Pattern delete function.
   void (*del_func)(void *pattern);

   // Search states.
   enum
   {
      DISTPENDING = 0,                        /* distance pending */
      DISTDONE    = 1,                        /* distance computed */
      EXPANDED    = 2,                        /* pattern expanded */
      SRCHDONE    = 3                         /* pattern searched */
   };

   static RDSearch *EMPTY;

   // Search stack.
   enum { STKMEM_QUANTUM=32 };              /* stack malloc increment */
   int stkMem;
   struct SrchStk
   {
      RDSearch *current_search;
      RDSearch *outer;
      RDSearch *outer_next;
   };

   // Search work memory.
   enum { SRCHWORKMEM_QUANTUM=128 };         /* search work memory size */

   // Search control.
   struct SearchCtrl
   {
      RDSearch           *search_list;          /* search results */
      RDSearch           *search_best;          /* best search result */
      int                max_find;            /* max number of results */
      int                max_search;          /* max nodes to search (-1=unlimited) */
      int                search_count;        /* search counter */
      int                best_search;         /* best result search counter */
      struct SrchStk     *search_stack;           /* search stack */
      int                search_stack_idx;         /* stack index */
      int                search_stack_sz;          /* current stack size */
      Vector<RDSearch *> search_work;           /* search work space */
      int                search_work_idx;        /* search work index */
      int                search_work_use;        /* used search work */
   };

   // Internal functions.
   void Insert(RDNode *current, RDNode *node);
   void Search(struct SearchCtrl *search_ctrl, RDNode *search_node);
   void FoundPattern(struct SearchCtrl *search_ctrl, RDSearch *sw_found, int *found_count, RDSearch **sw_cut);
   RDSearch *GetSearchWork(struct SearchCtrl *search_ctrl);

   void LoadOuter(Stream& s, RDNode * parent, void *(*load_pattern)(Stream& s),
                     void *(*load_client)(Stream& s));
   void LoadOuter(Stream& s, void *helper, RDNode * parent,
                     void *(*load_pattern)(void *helper, Stream& s),
                     void *(*load_client)(void *helper, Stream& s));
   void StoreOuter(Stream& s, RDNode * parent,
                     void (*savePatt)(void *pattern, FILE *fp),
                     void (*saveClient)(void *client, FILE *fp));
   void PrintNode(Stream& s, RDNode * node, int level,
                  void (*print_pattern)(void *pattern, FILE *fp));
};
#endif
