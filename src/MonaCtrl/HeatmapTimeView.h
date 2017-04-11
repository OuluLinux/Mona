#ifndef _MonaCtrl_HeatmapTimeView_h_
#define _MonaCtrl_HeatmapTimeView_h_

using namespace Upp;

class HeatmapTimeView : public Ctrl {
	/*Session* ses;
	Graph* graph;
	RecurrentSession* rses;
	Array<Image> lines;
	Vector<double> tmp;
	int mode;
	*/
	enum {MODE_SESSION, MODE_GRAPH, MODE_RECURRENTSESSION};
	
public:
	typedef HeatmapTimeView CLASSNAME;
	HeatmapTimeView();
	/*
	virtual void Paint(Draw& d);
	void PaintSession(Draw& d);
	void PaintGraph(Draw& d);
	void PaintRecurrentSession(Draw& d);
	
	void SetSession(Session& ses) {this->ses = &ses; mode = MODE_SESSION;}
	void SetGraph(Graph& g) {this->graph = &g; mode = MODE_GRAPH;}
	void SetRecurrentSession(RecurrentSession& rses) {this->rses = &rses; mode = MODE_RECURRENTSESSION;}
	*/
};

#endif
