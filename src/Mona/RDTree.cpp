#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "RDTree.h"
#include "FileIO.h"

// Tree configuration parameter.
const double RDTree::DEFAULT_RADIUS = 100.0;

RDTree::RDSearch* RDTree::EMPTY = (RDSearch*)(-1);



// Node constructors.
RDTree::RDNode::RDNode(Vector<SENSOR>& pattern, Receptor& client) {
	this->pattern <<= pattern;
	this->client  = &client;
	outer_list    = outer_last = NULL;
	equal_next    = equal_prev = NULL;
	distance      = 0.0;
}


RDTree::RDNode::RDNode() {
	client        = NULL;
	outer_list    = outer_last = NULL;
	equal_next    = equal_prev = NULL;
	distance      = 0.0;
}














// Search element constructor.
RDTree::RDSearch::RDSearch() {
	node      = NULL;
	distance  = 0.0;
	search_next  = NULL;
	workdist  = 0.0;
	state     = DISTPENDING;
	outer_list = equal_next = equal_prev = NULL;
}











// Constructors.
/*RDTree::RDTree(double(*dist_func)(void*, void*), void(*del_func)(void*)) {
	RADIUS         = DEFAULT_RADIUS;
	this->dist_func = dist_func;
	this->del_func  = del_func;
	root           = NULL;
	stkMem         = STKMEM_QUANTUM;
}


RDTree::RDTree(double radius, double(*dist_func)(void*, void*),
			   void(*del_func)(void*)) {
	RADIUS         = radius;
	this->dist_func = dist_func;
	this->del_func  = del_func;
	root           = NULL;
	stkMem         = STKMEM_QUANTUM;
}
*/

// Destructor.
RDTree::~RDTree() {
	DeleteSubtree(root);
}


// Delete subtree.
void RDTree::DeleteSubtree(RDNode* node) {
	RDNode* p, *p2, *p3;

	// convert tree to list
	if ((p = node) == NULL)
		return;

	for (p2 = p; p2 != NULL; p2 = p2->equal_next) {
		if (p2->outer_list != NULL) {
			p3                   = p2->equal_next;
			p2->equal_next          = p2->outer_list;
			p2->equal_next->equal_prev = p2;

			if (p3 != NULL)
				p3->equal_prev = p2->outer_last;

			p2->outer_last->equal_next = p3;
			p2->outer_list          = p2->outer_last = NULL;
		}
	}

	// delete list
	for (p2 = p; p2 != NULL; p2 = p3) {
		p3 = p2->equal_next;

		if ((p2->pattern != NULL) && (del_func != NULL))
			del_func(p2->pattern);

		delete p2;
	}
}


// Insert pattern.
void RDTree::Insert(Vector<SENSOR>& pattern, Receptor& client) {
	RDNode* node = new RDNode(pattern, client);
	ASSERT(node != NULL);
	Insert(*root, *node);
}


void RDTree::Insert(RDNode& current, RDNode& node) {
	RDNode* p, *p2, *p3;
	double  dcn, dnn;
	
	// clear node
	node.outer_list = NULL;
	node.outer_last = NULL;
	node.equal_next   = NULL;
	node.equal_prev   = NULL;
	node.distance  = 0.0;

	// new root?
	/*if (current == NULL) {
		root = &node;
		return;
	}*/
	Panic("TODO: check root checking");

	// add pattern to first acceptable branch
	dcn = dist_func(current.pattern, node.pattern);

	while (1) {
		for (p = current.outer_list; p != NULL; p = p->equal_next) {
			
			// check relative distances
			dnn = dist_func(p->pattern, node.pattern);

			if (dnn <= (p->distance * RADIUS)) {
				
				// change current fragment
				current = p;
				dcn     = dnn;
				break;
			}
		}

		if (p == NULL)
			break;
	}

	// link new as outer of current pattern
	node.distance = dcn;
	node.equal_next  = NULL;
	node.equal_prev  = current.outer_last;

	if (current.outer_last != NULL)
		current.outer_last->equal_next = node;
	else
		current.outer_list = node;

	current.outer_last = node;

	
	// check if previously added patterns should be un-linked from the
	// current pattern and linked as outerren of the new pattern.
	for (p = current.outer_list; p != node && p != NULL; ) {
		dnn = dist_func(p->pattern, node.pattern);

		// if should be linked to new pattern
		if (dnn <= (node.distance * RADIUS)) {
			// re-link outerren
			p2 = p->equal_next;

			if (p->equal_prev != NULL)
				p->equal_prev->equal_next = p2;
			else
				current.outer_list = p2;

			if (p2 != NULL)
				p2->equal_prev = p->equal_prev;

			// convert outer sub-tree to list
			for (p2 = p3 = p, p->equal_next = NULL; ; p3 = p3->equal_next) {
				for ( ; p2->equal_next != NULL; p2 = p2->equal_next) {
				}

				for ( ; p3 != NULL && p3->outer_list == NULL; p3 = p3->equal_next) {
				}

				if (p3 == NULL)
					break;

				p2->equal_next = p3->outer_list;
			}

			// add list to current pattern
			for (p2 = p; p2 != NULL; p2 = p3) {
				p3 = p2->equal_next;
				Insert(current, p2);
			}

			// restart check (since outer configuration may have changed)
			for (p = current.outer_list; p != node && p != NULL; p = p->equal_next) {
			}

			if (p == NULL)
				break;

			p = current.outer_list;
		}
		else
			p = p->equal_next;
	}
}


// Remove pattern.
void RDTree::Remove(void* pattern) {
	RDNode* current, *node, *p, *p2, *p3;
	double  d;

	if ((current = node = root) == NULL)
		return;

	d = dist_func(current->pattern, pattern);

	while (d > 0.0f) {
		for (node = current->outer_list; node != NULL; node = node->equal_next) {
			d = dist_func(node->pattern, pattern);

			if (d <= (node->distance * RADIUS)) {
				if (d > 0.0f)
					current = node;

				break;
			}
		}

		if (node == NULL)
			return;
	}

	// unlink pattern
	if (node == root)
		current = root = NULL;
	else {
		if (node->equal_prev == NULL)
			current->outer_list = node->equal_next;
		else
			node->equal_prev->equal_next = node->equal_next;

		if (node->equal_next == NULL)
			current->outer_last = node->equal_prev;
		else
			node->equal_next->equal_prev = node->equal_prev;
	}

	node->equal_next = node->equal_prev = NULL;
	
	// convert outer sub-tree to list
	p = node->outer_list;

	for (p2 = p; p2 != NULL; p2 = p2->equal_next) {
		if (p2->outer_list != NULL) {
			p3                   = p2->equal_next;
			p2->equal_next          = p2->outer_list;
			p2->equal_next->equal_prev = p2;

			if (p3 != NULL)
				p3->equal_prev = p2->outer_last;

			p2->outer_last->equal_next = p3;
			p2->outer_list          = p2->outer_last = NULL;
		}
	}

	// add list to parent pattern
	for (p2 = p; p2 != NULL; p2 = p3) {
		p3 = p2->equal_next;
		Insert(current, p2);

		if (current == NULL)
			current = root;
	}

	// delete node.
	delete node;
}


// search space for patterns closest to the given pattern
// return best matches
RDTree::RDSearch* RDTree::Search(void* pattern, int max_find, int max_search) {
	RDSearch* sw, *sw2, *sw3;
	RDSearch* search_list = NULL;

	if (root == NULL)
		return (search_list);

	// prepare for search
	struct SearchCtrl search_ctrl;
	search_ctrl.search_list    = NULL;
	search_ctrl.search_best    = NULL;
	search_ctrl.max_find     = max_find;
	search_ctrl.max_search   = max_search;
	search_ctrl.search_count = search_ctrl.best_search = 0;
	search_ctrl.search_stack_sz   = stkMem;
	search_ctrl.search_stack     = (struct SrchStk*)malloc(search_ctrl.search_stack_sz * sizeof(struct SrchStk));
	ASSERT(search_ctrl.search_stack != NULL);
	search_ctrl.search_stack_idx = 0;
	search_ctrl.search_work.Add((RDSearch*)malloc(SRCHWORKMEM_QUANTUM * sizeof(RDSearch)));
	ASSERT(search_ctrl.search_work[0] != NULL);
	memset(search_ctrl.search_work[0], 0, SRCHWORKMEM_QUANTUM * sizeof(RDSearch));
	search_ctrl.search_work_idx = 0;
	search_ctrl.search_work_use = 0;
	RDNode node(pattern, NULL);
	
	// search tree
	Search(&search_ctrl, &node);

	// extract search results
	for (sw = search_ctrl.search_list, sw3 = NULL; sw != NULL; sw = sw->search_next) {
		sw2 = new RDSearch();
		ASSERT(sw2 != NULL);
		sw2->node     = sw->node;
		sw2->distance = sw->distance;

		if (sw3 == NULL)
			search_list = sw2;
		else
			sw3->search_next = sw2;

		sw3 = sw2;
	}

	/* free search memory */
	free(search_ctrl.search_stack);

	for (int i = 0, j = (int)search_ctrl.search_work.GetCount(); i < j; i++)
		free(search_ctrl.search_work[i]);

	return (search_list);
}


/* search space for patterns closest to the given pattern */
/* put best matches on search_list */
void RDTree::Search(struct SearchCtrl* search_ctrl, RDTree::RDNode* search_node) {
	int            numSearch, stkIdx;
	struct SrchStk* stkp, *stkp2;
	RDNode*         p;
	RDSearch*       sw, *sw2, *bsw, *bsw2;
	RDSearch*       sw_cut;
	int            found_count;
	/* initialize */
	sw_cut     = NULL;
	numSearch = found_count = 0;

	if ((search_ctrl->max_search >= 0) && (numSearch >= search_ctrl->max_search))
		return;

	if (root == NULL)
		return;

	stkp                     = &(search_ctrl->search_stack[search_ctrl->search_stack_idx]);
	stkp->current_search           = GetSearchWork(search_ctrl);
	stkp->current_search->node     = root;
	stkp->current_search->distance = dist_func(stkp->current_search->node->pattern, search_node->pattern);
	stkp->current_search->state    = DISTDONE;
	FoundPattern(search_ctrl, stkp->current_search, &found_count, &sw_cut);
	numSearch++;

	if ((search_ctrl->max_search >= 0) && (numSearch >= search_ctrl->max_search))
		return;

	/* for each level of recursion */
	while (search_ctrl->search_stack_idx >= 0) {
		/* find best and next best distances for current search branch */
		for (stkIdx = search_ctrl->search_stack_idx; stkIdx >= 0; stkIdx--) {
			stkp = &(search_ctrl->search_stack[stkIdx]);

			/* expand pattern? */
			if (stkp->current_search->state == DISTDONE) {
				for (p = stkp->current_search->node->outer_list, sw2 = NULL; p != NULL; p = p->equal_next) {
					sw        = GetSearchWork(search_ctrl);
					sw->node  = p;
					sw->state = DISTPENDING;

					if (sw2 == NULL)
						stkp->current_search->outer_list = sw;
					else {
						sw2->equal_next = sw;
						sw->equal_prev  = sw2;
					}

					sw2 = sw;
				}

				stkp->outer           = stkp->outer_next = NULL;
				stkp->current_search->state = EXPANDED;
			}

			/* best and next best distances must be (re)computed? */
			if ((stkp->outer == NULL) ||
				((stkp->outer != NULL) &&
				 (stkp->outer->workdist > 0.0f) &&
				 (stkp->outer_next == EMPTY)) ||
				((stkp->outer != NULL) &&
				 (stkp->outer_next != NULL) &&
				 (stkp->outer_next != EMPTY) &&
				 (stkp->outer->workdist > stkp->outer_next->workdist))) {
				bsw = bsw2 = NULL;

				for (sw = stkp->current_search->outer_list; sw != NULL; sw = sw2) {
					sw2 = sw->equal_next;

					/* have best possible outer? */
					if ((bsw != NULL) && (bsw->workdist == 0.0f)) {
						if (sw2 == NULL)
							bsw2 = NULL;
						else
							bsw2 = EMPTY;

						break;
					}

					/* compute distance? */
					if (sw->state == DISTPENDING) {
						sw->distance = dist_func(sw->node->pattern, search_node->pattern);

						if ((sw->workdist = sw->distance -
											(sw->node->distance * RADIUS)) < 0.0f)
							sw->workdist = 0.0;

						sw->state = DISTDONE;
						/* save pattern on return list */
						FoundPattern(search_ctrl, sw, &found_count, &sw_cut);
						numSearch++;

						/* check for termination of search */
						if ((search_ctrl->max_search >= 0) && (numSearch >= search_ctrl->max_search))
							return;
					}

					/* cut off infeasible or finished branch */
					if ((sw->state == SRCHDONE) ||
						((sw_cut != NULL) && (sw->workdist >= sw_cut->distance))) {
						/* cut off */
						sw->state = SRCHDONE;

						if (sw->equal_prev == NULL)
							stkp->current_search->outer_list = sw->equal_next;
						else
							sw->equal_prev->equal_next = sw->equal_next;

						if (sw->equal_next != NULL)
							sw->equal_next->equal_prev = sw->equal_prev;

						continue;
					}

					/* find best and next best outer branches */
					if ((bsw == NULL) || (bsw->workdist > sw->workdist)) {
						bsw2 = bsw;
						bsw  = sw;
						continue;
					}

					if ((bsw2 == NULL) || (bsw2->workdist > sw->workdist)) {
						bsw2 = sw;
						continue;
					}
				}

				/* change to better branch level? */
				if (stkp->outer != bsw)
					search_ctrl->search_stack_idx = stkIdx;

				stkp->outer     = bsw;
				stkp->outer_next = bsw2;
			}

			/* finished with this level? */
			if (stkp->outer == NULL) {
				stkp->current_search->state = SRCHDONE;
				search_ctrl->search_stack_idx   = stkIdx - 1;

				if (stkIdx > 0)
					(stkp - 1)->outer = NULL;
			}
			else {
				/* set new best distance for branch */
				for (stkp2 = stkp; stkp2 >= search_ctrl->search_stack; stkp2--)
					stkp2->current_search->workdist = stkp->outer->workdist;
			}
		}

		/* expand search deeper */
		if (search_ctrl->search_stack_idx >= 0) {
			search_ctrl->search_stack_idx++;

			if (search_ctrl->search_stack_idx == search_ctrl->search_stack_sz) {
				stkMem            += STKMEM_QUANTUM;
				search_ctrl->search_stack_sz = stkMem;
				search_ctrl->search_stack   = (struct SrchStk*)realloc((void*)(search_ctrl->search_stack), search_ctrl->search_stack_sz * sizeof(struct SrchStk));
				ASSERT(search_ctrl->search_stack != NULL);
			}

			stkp           = &(search_ctrl->search_stack[search_ctrl->search_stack_idx]);
			stkp->current_search = (stkp - 1)->outer;
			stkp->outer    = stkp->outer_next = NULL;
		}
	}
}


/* add pattern to the found list */
void RDTree::FoundPattern(struct SearchCtrl* search_ctrl, RDSearch* sw_found,
						  int* found_count, RDSearch** sw_cut) {
	RDSearch* sw, *sw2;

	for (sw = search_ctrl->search_list, sw2 = NULL; sw != NULL; sw2 = sw, sw = sw->search_next) {
		if (sw->distance <= sw_found->distance)
			break;
	}

	search_ctrl->search_count++;

	if (sw == NULL)
		search_ctrl->best_search = search_ctrl->search_count;

	if (sw2 == NULL) {
		if (*found_count < search_ctrl->max_find) {
			(*found_count)++;
			sw_found->search_next = search_ctrl->search_list;
			search_ctrl->search_list = sw_found;
		}
	}
	else {
		sw_found->search_next = sw2->search_next;
		sw2->search_next     = sw_found;

		if (*found_count < search_ctrl->max_find)
			(*found_count)++;
		else
			search_ctrl->search_list = (search_ctrl->search_list)->search_next;
	}

	if (*found_count == search_ctrl->max_find) {
		/* set cut off */
		*sw_cut = search_ctrl->search_list;
	}
}


/* get a pattern search element */
RDTree::RDSearch* RDTree::GetSearchWork(struct SearchCtrl* search_ctrl) {
	RDSearch* sw;

	if (search_ctrl->search_work_use == SRCHWORKMEM_QUANTUM) {
		search_ctrl->search_work_idx++;
		search_ctrl->search_work.Add((RDSearch*)malloc(SRCHWORKMEM_QUANTUM * sizeof(RDSearch)));
		ASSERT(search_ctrl->search_work[search_ctrl->search_work_idx] != NULL);
		memset(search_ctrl->search_work[search_ctrl->search_work_idx], 0, SRCHWORKMEM_QUANTUM * sizeof(RDSearch));
		search_ctrl->search_work_use = 0;
	}

	sw = &(search_ctrl->search_work[search_ctrl->search_work_idx][search_ctrl->search_work_use]);
	search_ctrl->search_work_use++;
	return (sw);
}


/* load tree */
bool RDTree::Load(String filename, void* (*load_pattern)(Stream& s),
				  Receptor* (*load_client)(Stream& s)) {
	FileIn fp(filename);

	if (!fp.IsOpen())
		return false;

	Load(fp, load_pattern, load_client);
	return true;
}


/* load tree */
void RDTree::Load(Stream& fp, void* (*load_pattern)(Stream& s),
				  Receptor* (*load_client)(Stream& s)) {
	int   n;
	double d;
	DeleteSubtree(root);
	root = NULL;
	FREAD_INT(&n, fp);

	if (n == 1) {
		root = new RDNode();
		ASSERT(root != NULL);
		root->pattern = load_pattern(fp);

		if (load_client != NULL)
			root->client = load_client(fp);

		FREAD_FLOAT(&d, fp);
		root->distance = d;
		LoadOuter(fp, root, load_pattern, load_client);
	}
}


/* load outerren */
void RDTree::LoadOuter(Stream& fp, RDNode* parent,
					   void* (*load_pattern)(Stream& s),
					   Receptor* (*load_client)(Stream& s)) {
	int    n;
	double  d;
	RDNode* p, *p2;
	FREAD_INT(&n, fp);
	p2 = NULL;

	for (int i = 0; i < n; i++) {
		p = new RDNode();
		ASSERT(p != NULL);
		p->pattern = load_pattern(fp);

		if (load_client != NULL)
			p->client = load_client(fp);

		FREAD_FLOAT(&d, fp);
		p->distance = d;

		if (i == 0)
			parent->outer_list = p;

		parent->outer_last = p;

		if (p2 != NULL) {
			p2->equal_next = p;
			p->equal_prev  = p2;
		}

		p2 = p;
		LoadOuter(fp, p, load_pattern, load_client);
	}
}


/* load tree with address resolver */
bool RDTree::Load(String filename, void* helper,
				  void* (*load_pattern)(void* helper, Stream& s),
				  Receptor* (*load_client)(void* helper, Stream& s)) {
	FileIn in(filename);

	if (!in.IsOpen())
		return false;

	Load(in, helper, load_pattern, load_client);
	return true;
}


/* load tree with address resolver */
void RDTree::Load(Stream& fp, void* helper,
				  void* (*load_pattern)(void* helper, Stream& s),
				  Receptor* (*load_client)(void* helper, Stream& s)) {
	int   n;
	double d;
	root = NULL;
	FREAD_INT(&n, fp);

	if (n == 1) {
		root = new RDNode();
		ASSERT(root != NULL);
		root->pattern = load_pattern(helper, fp);

		if (load_client != NULL)
			root->client = load_client(helper, fp);

		FREAD_FLOAT(&d, fp);
		root->distance = d;
		LoadOuter(fp, helper, root, load_pattern, load_client);
	}
}


/* load outerren with address resolver */
void RDTree::LoadOuter(Stream& fp, void* helper,
					   RDNode* parent,
					   void* (*load_pattern)(void* helper, Stream& s),
					   Receptor* (*load_client)(void* helper, Stream& s)) {
	int    n;
	double  d;
	RDNode* p, *p2;
	FREAD_INT(&n, fp);
	p2 = NULL;

	for (int i = 0; i < n; i++) {
		p = new RDNode();
		ASSERT(p != NULL);
		p->pattern = load_pattern(helper, fp);

		if (load_client != NULL)
			p->client = load_client(helper, fp);

		FREAD_FLOAT(&d, fp);
		p->distance = d;

		if (i == 0)
			parent->outer_list = p;

		parent->outer_last = p;

		if (p2 != NULL) {
			p2->equal_next = p;
			p->equal_prev  = p2;
		}

		p2 = p;
		LoadOuter(fp, helper, p, load_pattern, load_client);
	}
}


/* save tree */
bool RDTree::Store(String filename, void (*store_pattern)(void* pattern, Stream& fp),
				   void (*store_client)(void* client, Stream& fp)) {
	FileOut fp(filename);

	if (!fp.IsOpen())
		return false;

	Store(fp, store_pattern, store_client);
	return true;
}


/* save tree */
void RDTree::Store(Stream& fp, void (*store_pattern)(void* pattern, Stream& fp),
				   void (*store_client)(void* client, Stream& fp)) {
	int   n;
	double d;

	if (root == NULL) {
		n = 0;
		FWRITE_INT(&n, fp);
	}
	else {
		n = 1;
		FWRITE_INT(&n, fp);
		store_pattern(root->pattern, fp);

		if (store_client != NULL)
			store_client(root->client, fp);

		d = root->distance;
		FWRITE_FLOAT(&d, fp);
		StoreOuter(fp, root, store_pattern, store_client);
	}
}


/* save outerren */
void RDTree::StoreOuter(Stream& fp, RDNode* parent,
						void (*store_pattern)(void* pattern, Stream& fp),
						void (*store_client)(void* client, Stream& fp)) {
	int    n;
	double  d;
	RDNode* p;

	for (p = parent->outer_list, n = 0; p != NULL; p = p->equal_next, n++) {
	}

	FWRITE_INT(&n, fp);

	for (p = parent->outer_list; p != NULL; p = p->equal_next) {
		store_pattern(p->pattern, fp);

		if (store_client != NULL)
			store_client(p->client, fp);

		d = p->distance;
		FWRITE_FLOAT(&d, fp);
		StoreOuter(fp, p, store_pattern, store_client);
	}
}


// Print tree.
/*
    bool RDTree::Print(String filename, void (*print_pattern)(void *pattern, Stream& fp))
    {
    if (filename.IsEmpty())
    {
      //Print(print_pattern);
      return;
    }
    else
    {
      FileOut fp(filename);
      if (!fp.IsOpen())
         return false;
      Print(print_pattern, fp);
    }
    return true;
    }


    void RDTree::Print(void (*print_pattern)(void *pattern, Stream& fp), Stream& fp)
    {
    fprintf(fp, "<RDTree>\n");
    PrintNode(fp, root, 0, print_pattern);
    fprintf(fp, "</RDTree>\n");
    }


    // print node
    void RDTree::PrintNode(Stream& fp, RDNode *node, int level,
                       void (*print_pattern)(void *pattern, Stream& fp))
    {
    int    i;
    RDNode *p;

    if (node != NULL)
    {
      for (i = 0; i < level; i++)
      {
         fprintf(fp, "  ");
      }
      fprintf(fp, "<node>\n");
      for (i = 0; i < level; i++)
      {
         fprintf(fp, "  ");
      }
      fprintf(fp, "  <pattern>");
      print_pattern(node->pattern, fp);
      fprintf(fp, "</pattern>\n");
      for (i = 0; i < level; i++)
      {
         fprintf(fp, "  ");
      }
      fprintf(fp, "  <distance>%f</distance>\n", node->distance);
      for (i = 0; i < level; i++)
      {
         fprintf(fp, "  ");
      }
      fprintf(fp, "  <outerren>\n");
      for (p = node->outer_list; p != NULL; p = p->equal_next)
      {
         PrintNode(fp, p, level + 1, print_pattern);
      }
      for (i = 0; i < level; i++)
      {
         fprintf(fp, "  ");
      }
      fprintf(fp, "  </outerren>\n");
      for (i = 0; i < level; i++)
      {
         fprintf(fp, "  ");
      }
      fprintf(fp, "</node>\n");
    }
    }*/
