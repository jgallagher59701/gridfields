#include "config_gridfields.h"

#include <math.h>
#include "aggregations.h"

float Aggregate::euclid(float x1, float y1, float x2, float y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float Aggregate::euclid3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

void Aggregate::interpolate3D::operator()(vector<Tuple> &tupset, Tuple &out)
{
	int s = tupset.size();
	float dist[s];
	float x, y, z, v[s][attrs.size()];
	float px, py, pz, *val;
	float sum = 0;

	Tuple *t = &out;
	px = *(float *) t->get("x");
	py = *(float *) t->get("y");
	pz = *(float *) t->get("z");

	val = new float[attrs.size()];
	float null = -9999.0;
	if (s == 0) {
		for (size_t j = 0; j < attrs.size(); j++) {
			out.set(attrs[j], &null);
		}
	}

	for (int i = 0; i < s; i++) {
		x = *(float *) tupset[i].get("x");
		y = *(float *) tupset[i].get("y");
		z = *(float *) tupset[i].get("z");
		for (size_t j = 0; j < attrs.size(); j++) {
			v[i][j] = *(float *) tupset[i].get(attrs[j]);
		}
		dist[i] = euclid3D(x, y, z, px, py, pz);
		sum += dist[i];
	}

	//cout << "s=" << s << endl;
	for (size_t j = 0; j < attrs.size(); j++) {
		val[j] = 0;
		for (int i = 0; i < s; i++) {
			val[j] += (dist[i] / sum) * v[i][j];
		}
		(*(float *) out.get(attrs[j])) = val[j];
		//out.set(attrs[j], &val[j]);
		//cout << "val: " << attrs[j] << "=" << *(float*) &val[j]    << endl;
	}
}
