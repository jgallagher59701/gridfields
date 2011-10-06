#include <fstream>
#include <stdio.h>
#include "arrayreader.h"
#include "gridfield.h"
#include "array.h"
#include "timing.h"
#include "ordmap.h"
#include "unistd.h"

using namespace std;

ArrayReader::~ArrayReader() {
}

FileArrayReader::~FileArrayReader() {
  if (this->f != NULL) { 
    this->f->close(); 
    delete this->f;
  } 
}

FileArrayReader::FileArrayReader(string fn, long off) :
  filename(fn), f(NULL)
{ 
  offset = off;
  addrAttribute = "";
  mode = ios::binary; 
  this->prepFile();
  this->setStream(this->f);
}

FileArrayReader::FileArrayReader(string fn, long off, string pa) : 
  filename(fn), f(NULL)
{ 
  offset = off;
  addrAttribute = pa;
  mode = ios::binary; 
  this->prepFile();
  this->setStream(this->f);
}

FileArrayReader::FileArrayReader(ifstream *f) :
  filename(""), f(f), mode(ios::binary), ArrayReader(f, f->tellg())
{
}

void FileArrayReader::setFilename(string &filename) {
  if (filename != this->filename) {
    this->filename = filename;
    this->f->close();
    this->prepFile();
  }
}

void FileArrayReader::prepFile() {
 
  if (this->f == NULL) 
    this->f = new ifstream((const char *) this->filename.c_str(), 
                           this->mode | ios::in);
  
  
  if (!this->f->is_open()) {
    this->f->open(this->filename.c_str());
  }
  
  if (f->fail()) { 
    for (int i=0; i< 1000000000; i++) {
      
    }
    cout << "Problem accessing file...retrying." << endl;
    this->f->clear();
    if (f->fail()) { 
      cout << "Still failed after clear?" << endl;
    }
    this->f->clear();
    usleep(2000);
    delete this->f;
    this->f = new ifstream((const char *) this->filename.c_str(),
                           this->mode | ios::in);

    if (f->fail()) { 
      cout << "Still failed with brand new stream." << endl;
    }

    if (f->good()) {
      cout << "File is good." << endl;
    }

    if (!this->f->is_open()) this->f->open(this->filename.c_str());
  }

  if (f->fail()) { 
    Fatal("Unable to open file %s.", filename.c_str());
  }

  long start = f->tellg();
  //cout << "seekto " << start + offset << endl;
  this->f->seekg(start + this->offset, fstream::beg); 
}

void ArrayReader::Read(GridField *Gf, int k, Array *arr) {

  size_t size = Gf->Card(k);

  bool cleanup = true;
  int *positions;

  // Look for the attribute that tells us how to read the array
  if (Gf->IsAttribute(k, addrAttribute)) {
    Array *addr = Gf->GetAttribute(k, addrAttribute);
    if (addr->type == INT) {

      addr->getData(positions);
      cleanup = false;
    } else {
      //Warning("address attribute '%s' found, but type is not INT", addrAttribute.c_str());
      positions = new int[size];
      float *fpos;
      addr->getData(fpos);
      for (size_t i=0; i<size; i++) {
	positions[i] = int(fpos[i]);
      }
    }               
  } else if (addrAttribute == "") {
    //assume ordinal positions
    positions = new int[size];
    for (size_t i = 0; i<size; i++) positions[i] = i;
  } else {
    Fatal("'%s' is not an attribute of this Gridfield.", addrAttribute.c_str());
  }

  char *data = (char *) arr->getVals();
  
  Scheme *sch = arr->getScheme();
  if (sch->size() > 1) {
    ReadTuples(*s, positions, size, sch, (UnTypedPtr *) data);
  }  else {
    ReadPrimitives(*s, positions, size, arr->type, data);
  }

  if (cleanup) delete [] positions;
}

void ArrayReader::setOffset(long offset) {
  //this->f->close();
  this->offset = offset;
  //this->prepFile();
}

void TextFileArrayReader::ReadPrimitives(ifstream &f, int *positions, int size, Type t, char *data) {
 
  int line = 0; 
  char textline[256];
  //float x;
//  stringstream ss;
 
//  long o = offset; 
  //assume offset is in lines instead of bytes 
  for (int x=0; x<offset; x++) {
    f.getline(textline,256);
//    cout << "skipped(" << offset << "): " << textline << endl;
  }
  
  switch (t) {
    case INT:
      
      for (int i=0; i<size; i++) {
        while (line <= positions[i]) {
          f.getline(textline, 256);
          line++;
        }
        sscanf(textline, "%i", (int *) &data[i]);
        line++;
      }
      break;
      
    case FLOAT:
      for (int i=0; i<size; i++) {
        while (line < positions[i]) {
          f.getline(textline, 256);
          line++;
        }
        f.getline(textline, 256);
      //  cout << textline << endl;
        sscanf(textline, "%f", (float *) &data[i]);
        line++;
        
      }
      break;
      
    default:
      
      Fatal("ArrayReader: only ints and floats for now...");
  }
  
}

void ArrayReader::ReadPrimitives(istream &f, int *positions, int size, Type t, char *data) {
  
  int i = 0;
  int index = 0;
  int block = 0;
  int pos = 0;

  //if (!f.is_open()) this->prepFile();
  if (size == 0) return;
   
  // NOT PORTABLE
  int valsize = sizeof(int);
  f.seekg(positions[0] * valsize + this->offset, ifstream::beg);
  if (f.eof()) {
    Fatal("Seek to (%i*%i + %i) past end of file while reading array.", positions[0], valsize, this->offset);
    f.clear();
  }
  
  // Read the data in big chunks if possible
  
  while (i<size) {
    index = i;
    pos = positions[i];
    block = 1;
    while (true) {
      if (i+1 >= size) break;
      if (positions[i + 1] != positions[i] + 1) break;
      ++block;
      ++i;
    }
    f.seekg( offset + pos * valsize, ifstream::beg );
    f.read(&(data[index*valsize]), block * valsize);
    //cout << "value: " << *(float *) &data[index] << ",  " << *(int *) &data[index] << endl;
    i++;
  }
}

void TextFileArrayReader::ReadTuples(ifstream &f, int *positions, int size, Scheme *sch, UnTypedPtr *tupptrs) {
  
  string format = "";
  Tuple *t;
  //int s = sch->size();

  int line = 0;
  char textline[256];
  
  //assume offset is in lines instead of bytes 
  for (int x=0; x<offset; x++) {
    f.getline(textline, 256);
  }
  
  for (int i=0; i<size; i++) {
    t = new Tuple(sch);
    
    //cout << "reading position " << positions[i] << endl;
    while (line < positions[i]) {
      f.getline(textline, 256);
      //cout << "eat tup: " << textline << endl;
      line++;
    }
    
    t->Read(f);
    
    line++;
    
    tupptrs[i] = (UnTypedPtr) t; 
    //((Tuple *) tupptrs[i])->print();
  }
  
}

void ProjectArrayReader::ReadPrimitives(istream &f, int *positions, int size, Type t, char *data) {
  int i = 0;
  int index = 0;
  int block = 0;
  int pos = 0;


  int valsize = typesize(t);
  int tuplesize=this->sch.bytesize();
  int tupleoffset = this->sch.byteposition(this->attr);
  
  char *scratch = new char[size*tuplesize];

  //cout << "Reading primitives: " << this->attr << ": " << tuplesize << ", " << tupleoffset << endl;
  
  // Copy the projectedright data in big chunks if possible
  while (i<size) {
    index = i;
    pos = positions[i];
    block = 1;
    while (true) {
      if (i+1 >= size) break;
      if (positions[i + 1] != positions[i] + 1) break;
      ++block;
      ++i;
    }
    //float r = gettime(); 
    f.seekg(offset + pos * tuplesize, ifstream::beg );
    f.read((char *) &scratch[index*tuplesize], block * tuplesize);
    //float a = gettime();
    //cout << "read call: " << block << ", " << index << ", " << r <<" , " << a << ", " << a - r << endl;
    for (int j=0; j<block; j++) {
      for(int k=0; k<valsize; k++) {
        data[(index+j)*valsize + k] = scratch[(index+j)*tuplesize+tupleoffset+k];
      }
     // cout << *(float *)&data[(index+j)*valsize] << ", "
    }
    i++;
  }

  delete [] scratch;
}

void ProjectArrayReader::ReadTuples(istream &f, int *positions, int size, Scheme *sch, UnTypedPtr *tupptrs) {
  Fatal("ReadTuples is deprecated; Arrays with complex schemes are deprecated.");
}


void ArrayReader::ReadTuples(istream &f, int *positions, int size, Scheme *sch, UnTypedPtr *tupptrs) {
  int i = 0;
  int index = 0;
  int block = 0;
  int pos = 0;


  //for each gridfield in the tuple, seek past it and save the location
  //so: gridfields must store location
  // need catalog information.
  // users will find gridfield compoennts based on metadata
  // mapping metadata to physical locations is a RDBMS job
  // definitely need to query (using our metadata stuff, probably)
  // then need to process the query - where?
  // what about sets of results - how to operate on all at once?
  // something to do with nesting.
   
  // NOT PORTABLE
  int valsize=0;
  for (unsigned int j=0; j<sch->size(); j++) {
    valsize = valsize + typesize(sch->getType(j));
  }
  
  char *data = new char[size*valsize];
  
  //vector for better allocation
  vector<Tuple> &tups = *(new vector<Tuple>(size,Tuple(sch)));
  
  //ptr structure to satisfy Array requirements.
  for (int j=0; j<size; j++) {
    tupptrs[j] = &tups[j];
  }
  
  // Read the data in big chunks if possible
  while (i<size) {
    index = i;
    pos = positions[i];
    block = 1;
    while (true) {
      if (i+1 >= size) break;
      if (positions[i + 1] != positions[i] + 1) break;
      ++block;
      ++i;
    }
    f.seekg( offset + pos * valsize, ifstream::beg );
    f.read((char *) &data[index*valsize], block * valsize);
    for (int j=0; j<block; j++) {
      tups[index+j].assign((char *) &data[index*valsize + j*valsize ]);
      //tups[index+j].print();
    }
  //  cout << *(float *) data[0] << endl;
    i++;
  }
}

