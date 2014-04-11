#include "config_gridfields.h"

#include <fstream>
#include <stdio.h>
#include <map>
#include "arraywriter.h"
#include "gridfield.h"
#include "array.h"
#include "ordmap.h"
#include "expr.h"

using namespace std;

namespace GF {

ArrayWriter::ArrayWriter(string fn, long off)
{
	filename = fn;
	offset = off;
	addrAttribute = "";
	f = NULL;
	this->prepFile();
}

ArrayWriter::ArrayWriter(string fn, long off, string pa)
{
	filename = fn;
	offset = off;
	addrAttribute = pa;
	f = NULL;
	this->prepFile();
}

ArrayWriter::ArrayWriter(ofstream *file)
{
	filename = "";
	offset = 0;
	addrAttribute = "";
	f = file;
}

void ArrayWriter::setOffset(long offset)
{
	this->offset = offset;
}

void ArrayWriter::prepFile()
{

	this->f = new ofstream((const char *) filename.c_str(), ofstream::app | ofstream::out);
	if (f->fail()) {
		cerr << "Unable to open " << filename << endl;
		exit(1);
	}

	long start = f->tellp();
	//cout << "seekto " << start + offset << endl;
	this->f->seekp(start + offset);
}

void ArrayWriter::Write(const Dataset &ds, string attr)
{

	int size = ds.Size();

	// Look for the attribute that tells us how to write the array
	int *positions;
	if (ds.IsAttribute(addrAttribute)) {
		Array *addr = ds.GetAttribute(addrAttribute);
		if (addr->type == INT) {
			addr->getData(positions);
		}
		else {
			Warning("address attribute '%s' found, but type is not INT", addrAttribute.c_str());

			float *fpos;
			addr->getData(fpos);
			positions = new int[size];
			for (int i = 0; i < size; i++) {
				//cout << fpos[i] << ", ";
				positions[i] = int(fpos[i]);
			}
		}
	}
	else {
		//assume ordinal positions
		positions = new int[size];
		for (int i = 0; i < size; i++)
			positions[i] = i;
	}

	//void **data = new void*[size];
	long valsize = sizeof(float);

	Array *outbound = ds.GetAttribute(attr);
	std::map<int, UnTypedPtr> sorted;
	//int x;
	//UnTypedPtr val;
//  Gf->getScheme()->print();

//  float tot = 0;
	for (int j = 0; j < size; j++) {
		//  tot += *(float *) outbound->getValPtr(positions[j]);
		this->f->write((const char *) outbound->getValPtr(positions[j]), valsize);
	}
	this->setOffset(f->tellp());
//  cout << "sum: " << tot << endl;
	/*
	 for (int j=0; j<size; j++) {
	 x = sorted.size();
	 sorted[positions[j]] = outbound->getValPtr(j);
	 if (x==sorted.size()) {
	 //  cout << j << ", " << positions[j] << ", ";
	 //  val = Gf->getAttributeVal("wetpos", j);
	 //  cout << *(int*)val << endl;
	 }
	 }
	 */
	/*
	 //  cout << "GFsize: " << size << endl;
	 //  cout << "size: " << sorted.size() << endl;
	 //  f.write( (const char *) &size, 4);
	 map<int, UnTypedPtr>::iterator i = sorted.begin();
	 int index = 0;
	 int block = 0;
	 int pos = 0;
	 //  cout << "writing..." << endl;
	 while ( i != sorted.end() ) {
	 //    cout << *(float *)(*i).second << endl;
	 this->f->write( (const char *) (*i).second, valsize );
	 i++;
	 }
	 */
	/*
	 // Write the data in big chunks if possible
	 while ( i != sorted.end() ) {
	 index = distance(sorted.begin(), i);
	 pos = *i;
	 //    cout << positions[i] << endl;
	 block = 1;
	 while (*(i + 1) == *i + 1) {
	 block++;
	 i++;
	 }
	 //    cout << "read " << block << " values" << endl;
	 f.write((char *) &data[index], block * valsize);
	 //    cout << (float *) data[0] << endl;
	 i++;
	 }
	 */
	/*
	 int zpos, bot;
	 float x, y, z;
	 for (int i=0; i<size; i++) {
	 zpos = *(int *) Gf->getAttributeVal("zpos", i);
	 bot = *(int *) Gf->getAttributeVal("b", i);
	 x = *(float *) Gf->getAttributeVal("x", i);
	 y = *(float *) Gf->getAttributeVal("y", i);
	 z = *(float *) Gf->getAttributeVal("z", i);
	 //cout << x << ", " << y << ", " << z << ", " << bot << ", " << zpos << ", " << ((float *) data)[i] << endl;
	 //    getchar();
	 }
	 */
	delete[] positions;
}

} // namespace GF
