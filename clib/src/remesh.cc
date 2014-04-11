#include "gridfield.h"
#include "array.h"
#include "remesh.h"
#include "apply.h"
#include "expr.h"
#include "timing.h"
#include "RTree.h"
#include "assignments.h"
#include "triangulate.h"
using namespace std;
using namespace Assign;
using namepsace GF {

	RemeshOp::RemeshOp(string attrx,string attry,GridField * Gnew, GridFieldOperator *op)
	: UnaryGridFieldOperator(op),Gnew(Gnew),attry(attry),attrx(attrx)
	{
	}

	void RemeshOp::Execute() {
		this->PrepareForExecution();
		Remesh(this->attrx,this->attry, this->GF,this->Gnew);

	}

	RTree<CellId, double, 2> RemeshOp::constructtree(Array* xs,Array* ys,vector< vector<int> > nodess) {
		RTree<CellId, double, 2> tree;
		double xp,yp;
		double min[2];
		double max[2];
		vector<double>a;

		for (int j=0; j<nodess.size(); j++) {
			a.reserve(nodess.at(j).size());
			xp = xs->getValfloat(nodess.at(j).at(0));
			yp = ys->getValfloat(nodess.at(j).at(0));
			min[0]=xp;
			min[1]=yp;
			max[0]=xp;
			max[1]=yp;
			for (int k1=1;k1<nodess.at(j).size();k1++)
			{   xp = xs->getValfloat(nodess.at(j).at(k1));
				yp = ys->getValfloat(nodess.at(j).at(k1));
				a.push_back(nodess.at(j).at(k1));
				if (xp < min[0]) min[0] = xp;
				if (xp > max[0]) max[0] = xp;
				if (yp < min[1]) min[1] = yp;
				if (yp > max[1]) max[1] = yp;}
			tree.Insert(min, max, j+1); // Note, all values including zero are fine in this version

		}
		return tree;
	}

	bool RemeshOp::point_inside_polygon(double x,double y,CPolygon poly) {

		int n=3,c=0;
		bool inside =0;
		double xinters;
		double p1x= poly.getpoint(1).getx(1);
		double p1y=poly.getpoint(1).getx(2);
		for (int j=2;j<=n+1;j++) {
			int i;i=j-1;i=i%n;i=i+1;

			double p2x= poly.getpoint(i).getx(1);
			double p2y=poly.getpoint(i).getx(2);
			if (y-min(p1y,p2y)>1.0e-15) {
				if (y - max(p1y,p2y)<=1.0e-15) {
					if ((x- max(p1x,p2x))<= 1.0e-15) {
						if (abs(p1y - p2y)>1.0e-15) {
							xinters = (y-p1y)*(p2x-p1x)/(p2y-p1y)+p1x;}
						if (abs(p1x-p2x)<1.0e-15 || x-xinters<= 1.0e-15) {
							inside = 1-inside;}
					}
				}
			}
			p1x= p2x;
			p1y= p2y;

		}
		c=inside;

		return c;
	}

	bool RemeshOp::inside(vector<double>& p,vector<double>& plane) //test if points are inside or outside polygon
	{
		double d=p[0]*plane[0]+p[1]*plane[1];
		bool bin;
		if (d>= plane[2])
		{   bin=1;}
		else
		{   bin=0;}

		return bin;}

	void RemeshOp::clip(vector<double>& c,vector<double>& p1,vector<double>& p2,vector<double>& plane) {
		c.clear();
		double d1=p1[0]*plane[0]+p1[1]*plane[1]-plane[2];
		double d2=p2[0]*plane[0]+p2[1]*plane[1]-plane[2];
		double t=(0.0-d1)/(d2-d1);
		c.reserve(2);
		c.push_back(p1[0] + t*(p2[0]-p1[0]));
		c.push_back(p1[1] + t*(p2[1]-p1[1]));
	}

	int RemeshOp::intersect(CPolygon* pout,CPolygon* pin,CPolygon* pin2)
	{   CPolygon* pout1=new CPolygon;CPolygon* pout2=new CPolygon;CPolygon* pout3=new CPolygon;
		double xin1[2],xin2[2];

		xin1[0]=pin2->getpoint(1).getx(1);
		xin1[1]=pin2->getpoint(1).getx(2);

		xin2[0]=pin2->getpoint(2).getx(1);
		xin2[1]=pin2->getpoint(2).getx(2);

		polyclip(pout1,pin,xin1,xin2);

		xin1[0]=pin2->getpoint(2).getx(1);
		xin1[1]=pin2->getpoint(2).getx(2);

		xin2[0]=pin2->getpoint(3).getx(1);
		xin2[1]=pin2->getpoint(3).getx(2);

		if (pout1->Numpts>0)
		{

			polyclip(pout2,pout1,xin1,xin2);}

		xin1[0]=pin2->getpoint(3).getx(1);
		xin1[1]=pin2->getpoint(3).getx(2);

		xin2[0]=pin2->getpoint(1).getx(1);
		xin2[1]=pin2->getpoint(1).getx(2);

		if (pout2->Numpts>0)
		{
			polyclip(pout3,pout2,xin1,xin2);
		}

		int outcheck=0;
		if (pout3->Numpts>=3) {

			double Pm1[2];
			Pm1[0]=0.0;
			Pm1[1]=0.0;
			for (int i1=1;i1<=pout3->Numpts;i1++)
			{
				Pm1[0]=pout3->getpoint(i1).getx(1)/(pout3->Numpts)+Pm1[0];
				Pm1[1]=pout3->getpoint(i1).getx(2)/(pout3->Numpts)+Pm1[1];

			}

			bool b=point_inside_polygon(Pm1[0],Pm1[1],*pin);
			if(b) {outcheck=1;
				pout->Numpts=0;
				for (int i1=1;i1<=pout3->Numpts;i1++)
				{
					pout->addpoint(pout3->getpoint(i1));

				}

			}
		}

		delete pout1;delete pout2;delete pout3;
		return outcheck;
	}

	void RemeshOp::polyclip(CPolygon* pout,CPolygon* pin,double x1[2],double x2[2])
	{

		vector<double> plane;
		plane.reserve(3);

		plane.push_back(x1[1]-x2[1]);
		plane.push_back(x2[0]-x1[0]);
		plane.push_back(x1[0]*plane[0]+x1[1]*plane[1]);

		int n=pin->Numpts;
		vector<double> s;
		s.reserve(2);
		s.push_back(pin->getpoint(n).getx(1));
		s.push_back(pin->getpoint(n).getx(2));

		vector<double> p;
		vector<double> t;
		t.reserve(2);
		p.reserve(2);
		int b;

		for (int c1=1;c1<=n;c1++)
		{
			p.clear();
			p.push_back(pin->getpoint(c1).getx(1));
			p.push_back(pin->getpoint(c1).getx(2));

			if (inside(p,plane)) {
				if (inside(s,plane)) { // case 1
					b=1;
					{
						Cgridpoint a;a.setgridpoint(p.at(0),p.at(1));
						pout->addpoint(a);}
				}
				else { // case 4
					b=2;
					clip(t,s,p,plane);
					double s1=t[0]-p[0];
					double s2=t[1]-p[1];
					if(sqrt(s1*s1+s2*s2)>1.0e-16) {

						{   Cgridpoint a;a.setgridpoint(t.at(0),t.at(1));
							pout->addpoint(a);

							Cgridpoint a1;a1.setgridpoint(p.at(0),p.at(1));
							pout->addpoint(a1);}
					}
					else {
						{
							Cgridpoint a;a.setgridpoint(t.at(0),t.at(1));
							pout->addpoint(a);}
					}
				}
			}
			else {
				if (inside(s,plane)) { // case 2
					b=3;
					clip(t,s,p,plane);
					double s1=t[0]-s[0];
					double s2=t[1]-s[1];
					if(sqrt(s1*s1+s2*s2)>1.0e-16) {
						{
							Cgridpoint a;a.setgridpoint(t.at(0),t.at(1));
							pout->addpoint(a);}
					}
				}
			}
			s.clear();
			s.push_back(p.at(0));
			s.push_back(p.at(1));

		}

	}

	set<int> RemeshOp::constructintersection(RTree<CellId, double, 2>& tree,CPolygon& Tri) {
		double x3, y3;
		double coordsmin[2],coordsmax[2];
		x3 = Tri.getpoint(1).getx(1);
		y3 = Tri.getpoint(1).getx(2);
		set<int> hits;
		hits.clear();

		coordsmin[0]=x3;
		coordsmin[1]=y3;
		coordsmax[0]=x3;
		coordsmax[1]=y3;

		for (int k1=2;k1<=3;k1++) {
			x3 = Tri.getpoint(k1).getx(1);
			y3 = Tri.getpoint(k1).getx(2);
			if (x3 < coordsmin[0]) {coordsmin[0] = x3;}
			if (x3 > coordsmax[0]) {coordsmax[0] = x3;}
			if (y3 < coordsmin[1]) {coordsmin[1] = y3;}
			if (y3 > coordsmax[1]) {coordsmax[1] = y3;}
		}

		tree.Search(coordsmin, coordsmax, TestCallback,&hits);
		return hits;
	}

	extern "C" void dgesv_(const int *N, const int *nrhs, double *A, const int *lda, int
			*ipiv, double *b, const int *ldb, int *info);

	GridField* RemeshOp::Remesh(const string &attrx,const string &attry,GridField *GF, GridField* Gnew) {

		int nums1=0;
		GridField *Out;
		Grid *G = GF->GetGrid();
		string name = "r" + G->name;
		Array* xa=GF->GetAttribute(0, attrx);
		Array* ya=GF->GetAttribute(0, attry);
		Array* ua=GF->GetAttribute(0, "u");
		vector<double> u=ua->makeArrayf();
//  vector<double> x=xa->makeArrayf();

		Array* xab=Gnew->GetAttribute(0, attrx);
		Array* yab=Gnew->GetAttribute(0, attry);
//  vector<double> x1=xab->makeArrayf();
//  vector<double> y1=yab->makeArrayf();
//  vector<double> y=ya->makeArrayf();
		int n=xa->size(), n1=xab->size();
		vector<double> Mt(n1*n1);vector<double> bvec(n1);

		for (int m=1;m<=n1;m++)
		for(int n=1;n<=n1;n++)
		{   Mt[n1*(n-1)+m-1]=0.0;}
		for (int m=1;m<=n1;m++) {bvec[m-1]=0.0;}
		CellArray* Inb=(CellArray*)(Gnew->GetGrid()->getKCells(2));
		CellArray* In = (CellArray*)(GF->GetGrid()->getKCells(2));
		vector< vector<int> > nodes2=Inb->makeArrayInts();
		vector< vector<int> > nodes=In->makeArrayInts();
//  vector< vector<double> >nodesx;
//  vector< vector<double> >nodesy;

		int tot=0;
		RTree<CellId, double, 2> tree=constructtree(xa,ya,nodes);

		vector< vector<int> > index;
		vector< vector<double> > ptot;

		Vector2dVector spts;
		spts.push_back(Vector2d(0.000000000000000,0.000000000000000 ));
		spts.push_back(Vector2d(0.125959254959390,0.125959254959390 ));
		spts.push_back(Vector2d(-0.251918509918779,0.125959254959390 ));
		spts.push_back(Vector2d(0.125959254959390,-0.251918509918779 ));
		spts.push_back(Vector2d(-0.162764025581573,-0.162764025581573 ));
		spts.push_back(Vector2d(0.325528051163147,-0.162764025581573 ));
		spts.push_back(Vector2d(-0.162764025581573,0.325528051163147 ));
		spts.push_back(Vector2d(-0.282786105016302,-0.282786105016302 ));
		spts.push_back(Vector2d( 0.565572210032605,-0.282786105016302 ));
		spts.push_back(Vector2d(-0.282786105016302, 0.565572210032605 ));
		spts.push_back(Vector2d(-0.324938555923375,-0.070220503698695 ));
		spts.push_back(Vector2d( -0.324938555923375,0.395159059622071 ));
		spts.push_back(Vector2d( -0.070220503698695,-0.324938555923375 ));
		spts.push_back(Vector2d( -0.070220503698695,0.395159059622071 ));
		spts.push_back(Vector2d( 0.395159059622071,-0.324938555923375 ));
		spts.push_back(Vector2d( 0.395159059622071,-0.070220503698695 ));

		vector<double> wgts;
		wgts.push_back(0.0721578038388935 );
		wgts.push_back(0.0475458171336425 );
		wgts.push_back(0.0475458171336425 );
		wgts.push_back(0.0475458171336425 );
		wgts.push_back(0.0516086852673590 );
		wgts.push_back(0.0516086852673590 );
		wgts.push_back(0.0516086852673590 );
		wgts.push_back(0.0162292488115990 );
		wgts.push_back(0.0162292488115990 );
		wgts.push_back(0.0162292488115990 );
		wgts.push_back(0.0136151570872175 );
		wgts.push_back(0.0136151570872175 );
		wgts.push_back(0.0136151570872175 );
		wgts.push_back(0.0136151570872175 );
		wgts.push_back(0.0136151570872175 );
		wgts.push_back(0.0136151570872175 );

		int mpoints=16;
		for (int j=0; j<nodes2.size(); j++) {

			Cgridpoint gpt1;
			gpt1.setgridpoint(xab->getValfloat(nodes2.at(j).at(0)),yab->getValfloat(nodes2.at(j).at(0)));
			double x2p1=xab->getValfloat(nodes2.at(j).at(0));
			double y2p1=yab->getValfloat(nodes2.at(j).at(0));
			Cgridpoint gpt2;
			gpt2.setgridpoint(xab->getValfloat(nodes2.at(j).at(1)),yab->getValfloat(nodes2.at(j).at(1)));
			double x2p2=xab->getValfloat(nodes2.at(j).at(1));double y2p2=yab->getValfloat(nodes2.at(j).at(1));
			Cgridpoint gpt3;
			gpt3.setgridpoint(xab->getValfloat(nodes2.at(j).at(2)),yab->getValfloat(nodes2.at(j).at(2)));
			double x2p3=xab->getValfloat(nodes2.at(j).at(2));double y2p3=yab->getValfloat(nodes2.at(j).at(2));
			vector<Cgridpoint> a1;a1.reserve(3);a1.push_back(gpt1);a1.push_back(gpt2);a1.push_back(gpt3);

			CPolygon* gTri2=new CPolygon(3,a1);

			int n21=nodes2.at(j).at(0);
			double c2a1= (x2p2*y2p3 - x2p3*y2p2)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			double c2a2= (y2p2 - y2p3)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			double c2a3= -(x2p2 - x2p3)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			int n22=nodes2.at(j).at(1);
			double c2b1= (x2p3*y2p1 - x2p1*y2p3)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			double c2b2= (y2p3 - y2p1)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			double c2b3= -(x2p3 - x2p1)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			int n23=nodes2.at(j).at(2);
			double c2c1= (x2p1*y2p2 - x2p2*y2p1)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			double c2c2= (y2p1 - y2p2)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));
			double c2c3= -(x2p1 - x2p2)/(x2p3*y2p1 - x2p2*y2p1 + x2p2*y2p3 - x2p3*y2p2 + x2p1*(y2p2 - y2p3));

			set<int>::iterator it;
			set<int> hits=constructintersection(tree,*gTri2);
			int indic=0;
			for (it=hits.begin(); it!=hits.end(); it++)
			{   tot=tot+1;

				Cgridpoint gpoint1;
				double x1p1=xa->getValfloat(nodes.at(*it-1).at(0));double y1p1=ya->getValfloat(nodes.at(*it-1).at(0));
				gpoint1.setgridpoint(xa->getValfloat(nodes.at(*it-1).at(0)),ya->getValfloat(nodes.at(*it-1).at(0)));

				Cgridpoint gpoint2;
				gpoint2.setgridpoint(xa->getValfloat(nodes.at(*it-1).at(1)),ya->getValfloat(nodes.at(*it-1).at(1)));
				double x1p2=xa->getValfloat(nodes.at(*it-1).at(1));double y1p2=ya->getValfloat(nodes.at(*it-1).at(1));

				Cgridpoint gpoint3;
				gpoint3.setgridpoint(xa->getValfloat(nodes.at(*it-1).at(2)),ya->getValfloat(nodes.at(*it-1).at(2)));
				double x1p3=xa->getValfloat(nodes.at(*it-1).at(2));double y1p3=ya->getValfloat(nodes.at(*it-1).at(2));
				vector<Cgridpoint> b4;b4.reserve(3);b4.push_back(gpoint1);b4.push_back(gpoint2);b4.push_back(gpoint3);
				CPolygon* gTri1=new CPolygon(3,b4);vector<Cgridpoint> a2;CPolygon* gTriout=new CPolygon(0,a2);

				int n11=nodes.at(*it-1).at(0);
				double c1a1= (x1p2*y1p3 - x1p3*y1p2)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				double c1a2= (y1p2 - y1p3)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				double c1a3= -(x1p2 - x1p3)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				int n12=nodes.at(*it-1).at(1);
				double c1b1= (x1p3*y1p1 - x1p1*y1p3)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				double c1b2= (y1p3 - y1p1)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				double c1b3= -(x1p3 - x1p1)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				int n13=nodes.at(*it-1).at(2);
				double c1c1= (x1p1*y1p2 - x1p2*y1p1)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				double c1c2= (y1p1 - y1p2)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));
				double c1c3= -(x1p1 - x1p2)/(x1p3*y1p1 - x1p2*y1p1 + x1p2*y1p3 - x1p3*y1p2 + x1p1*(y1p2 - y1p3));

				int b=intersect( gTriout,gTri2,gTri1);

				if(b==1)
				{   indic=1; nums1=nums1+1;

					Vector2dVector a;
					for(int k1=1;k1<=gTriout->Numpts;k1++)
					{   a.reserve(a.size()+1);
						a.push_back(Vector2d(gTriout->getpoint(k1).getx(1),gTriout->getpoint(k1).getx(2)));
					}

					Vector2dVector result;
					//  Invoke the triangulator to triangulate this polygon.
					Triangulate::Process(a,result);

					// print out the results.
					int tcount = result.size()/3;
					//if(tcount>0){indic=1;}
					for (int i=0; i<tcount; i++)
					{
						const Vector2d &p1 = result[i*3+0];
						const Vector2d &p2 = result[i*3+1];
						const Vector2d &p3 = result[i*3+2];
						const double x1 = p1.GetX();
						const double y1 = p1.GetY();
						const double x2 = p2.GetX();
						const double y2 = p2.GetY();
						const double x3 = p3.GetX();
						const double y3 = p3.GetY();

						const double xc = (x1+x2+x3)/3.0;
						const double yc = (y1+y2+y3)/3.0;

//coefficients for equations corresponding to nodes in the source

						double intD1=0.0;//elements of target vs. source
						double intD2=0.0;
						double intD3=0.0;

						double intT1=0.0;//elements of target vs. target
						double intT2=0.0;
						double intT3=0.0;

						for (int m=1; m<=mpoints; m++)
						{
							double s = spts.at(m-1).GetX();
							double t = spts.at(m-1).GetY();

							double xinter= xc + (x2-x1)*s + (x3-x1)*t;
							double yinter= yc + (y2-y1)*s + (y3-y1)*t;

							bvec[n21]=bvec[n21]+u.at(n11)*wgts.at(m-1)*(c1a1+xinter*c1a2+yinter*c1a3)*(c2a1+xinter*c2a2+yinter*c2a3); //elements of target vs. source
							bvec[n21]=bvec[n21]+u.at(n12)*wgts.at(m-1)*(c1b1+xinter*c1b2+yinter*c1b3)*(c2a1+xinter*c2a2+yinter*c2a3);
							bvec[n21]=bvec[n21]+u.at(n13)*wgts.at(m-1)*(c1c1+xinter*c1c2+yinter*c1c3)*(c2a1+xinter*c2a2+yinter*c2a3);

							bvec[n22]=bvec[n22]+u.at(n11)*wgts.at(m-1)*(c1a1+xinter*c1a2+yinter*c1a3)*(c2b1+xinter*c2b2+yinter*c2b3);//elements of target vs. source
							bvec[n22]=bvec[n22]+u.at(n12)*wgts.at(m-1)*(c1b1+xinter*c1b2+yinter*c1b3)*(c2b1+xinter*c2b2+yinter*c2b3);
							bvec[n22]=bvec[n22]+u.at(n13)*wgts.at(m-1)*(c1c1+xinter*c1c2+yinter*c1c3)*(c2b1+xinter*c2b2+yinter*c2b3);

							bvec[n23]=bvec[n23]+u.at(n11)*wgts.at(m-1)*(c1a1+xinter*c1a2+yinter*c1a3)*(c2c1+xinter*c2c2+yinter*c2c3);//elements of target vs. source
							bvec[n23]=bvec[n23]+u.at(n12)*wgts.at(m-1)*(c1b1+xinter*c1b2+yinter*c1b3)*(c2c1+xinter*c2c2+yinter*c2c3);
							bvec[n23]=bvec[n23]+u.at(n13)*wgts.at(m-1)*(c1c1+xinter*c1c2+yinter*c1c3)*(c2c1+xinter*c2c2+yinter*c2c3);

							Mt[n1*n21+n21]+=wgts.at(m-1)*(c2a1+xinter*c2a2+yinter*c2a3)*(c2a1+xinter*c2a2+yinter*c2a3);//elements of target vs. target
							Mt[n1*n21+n22]+=wgts.at(m-1)*(c2b1+xinter*c2b2+yinter*c2b3)*(c2a1+xinter*c2a2+yinter*c2a3);
							Mt[n1*n21+n23]+=wgts.at(m-1)*(c2c1+xinter*c2c2+yinter*c2c3)*(c2a1+xinter*c2a2+yinter*c2a3);

							Mt[n1*n22+n21]+=wgts.at(m-1)*(c2a1+xinter*c2a2+yinter*c2a3)*(c2b1+xinter*c2b2+yinter*c2b3);
							Mt[n1*n22+n22]+=wgts.at(m-1)*(c2b1+xinter*c2b2+yinter*c2b3)*(c2b1+xinter*c2b2+yinter*c2b3);
							Mt[n1*n22+n23]+=wgts.at(m-1)*(c2c1+xinter*c2c2+yinter*c2c3)*(c2b1+xinter*c2b2+yinter*c2b3);

							Mt[n1*n23+n21]+=wgts.at(m-1)*(c2a1+xinter*c2a2+yinter*c2a3)*(c2c1+xinter*c2c2+yinter*c2c3);
							Mt[n1*n23+n22]+=wgts.at(m-1)*(c2b1+xinter*c2b2+yinter*c2b3)*(c2c1+xinter*c2c2+yinter*c2c3);
							Mt[n1*n23+n23]+=wgts.at(m-1)*(c2c1+xinter*c2c2+yinter*c2c3)*(c2c1+xinter*c2c2+yinter*c2c3);

							double intT2=intT2+wgts.at(m-1)*(c2b1+xinter*c2b2+yinter*c2a3)*(c2b1+xinter*c2b2+yinter*c2b3);
							double intT3=intT3+wgts.at(m-1)*(c2c1+xinter*c2c2+yinter*c2a3)*(c2c1+xinter*c2c2+yinter*c2c3);

						}

						//printf("Triangle %d => (%0.0e,%0.0e) (%0.0e,%0.0e) (%0.0e,%0.0e)\n",i+1,p1.GetX(),p1.GetY(),p2.GetX(),p2.GetY(),p3.GetX(),p3.GetY());
					}

				}

				delete gTri1;delete gTriout;
			}
			delete gTri2;

		}

		int info;
		int k=1;
		vector<int> ipiv(n1);

		for (int m=1;m<=n1;m++)
		{   for(int n=1;n<=n1;n++)
			{   double a=Mt[n1*(n-1)+m-1]-Mt[n1*(m-1)+n-1];

			}
		}

		ofstream arrayData("data/test2/array2.txt"); // File Creation(on C drive)

		for(int k=0;k<n1*n1;k++)
		{
			arrayData<<Mt[k]<<endl; //Outputs array to txtFile
		}
		vector<double> a1(Mt);
		vector<double> b1(bvec);

		dgesv_(&n1, &k, &*Mt.begin(), &n1, &*ipiv.begin(), &*bvec.begin(), &n1, &info);
		cout<<"info= "<<info<<endl;
		Array* u2=new Array("u",FLOAT,n1);

		float placehold;
		for(int loop=0;loop<n1;loop++)
		{   placehold=bvec.at(loop);
			u2->set(loop,placehold);

		}
		Gnew->Bind(0,u2);
		double eps = 0.;double eps2 = 0.;
		for (int i = 0; i < n1; ++i)
		{
			double sum = 0.;
			for (int j = 0; j < n1; ++j)
			sum += a1[i + j*n1]*bvec[j];
			eps += fabs(b1[i] - sum);
			eps2 +=b1[i]-bvec[i];
		}
		cout << "check is " << eps<<" "<<eps2 << endl;

		return Gnew;
	}

}
