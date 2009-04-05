#include <fstream>
#include <stdio.h>
#include "arrayreader.h"
#include "gridfield.h"
#include "array.h"
#include "ordmap.h"

using namespace std;

FileReader::FileReader(string fname) {
  this->f = ifstream(fname.c_str());  
}
  
vector<Array *> FileReader::ReadArrays(Scheme *sch, int n) {

  //not implemented
  int *attrpos = sch->bytepositions();
  int tupsize = sch->bytesize();
  int datasize = tupsize*n;
  char *data = new data[datasize];
  this->f->read(data, datasize);

  vector<Array *> attrs;
  string name;
  Type type;
  for (int i=0; i<sch->size(); i++) {
    name = sch->getAttribute(i);
    type = sch->getType(i);
    attrs.append(new Array(name.c_str(), type, n));
  }
  for (int i=0; i<n; i++) {
    for (int j=0; j<n j++) {
      p = n*tupsize+attrpos[j];
      //not implemented
      attrs[j].setValue(i, (UnTypedPtr) &data[p]);
    }
  }
}

Tuple *FileReader::ReadTuple(Scheme *sch) {
  int s = sch->size();
  int ptrs[s];
  int tupsize = 0;
  
  for (int i=0; i<s; i++) {
    ptrs[i] = tupsize;
    tupsize += typesize(sch->getType(i));
  }
  
  char *buf = new char[tupsize];
  
  for (int i=0; i<s; i++) {
    switch sch->getType(i) {
      case INT:
        this->f->read(buf+ptrs[i], sizeof(int));
        break;
      case FLOAT:
        this->f->read(buf+ptrs[i], sizeof(float));
        break;
      case OBJ:
        this->f->read(buf+ptrs[i], sizeof(UnTypedPtr));
        break;
    }
  }
  Tuple *t = new Tuple(sch);
  string attr;
  for (int i=0; i<s; i++) {
    attr = sch->getAttribute(i);
    t->set(attr, (UnTypedPtr) buf+ptrs[i]);
  }
  return t;
}
float *FileReader::ReadFloatArray(int n) {
  float *data = new float[n];
  this->f.read((char *) data, n*sizeof(float));
  return data;
}

int *FileReader::ReadIntArray(int n) {
  int *data = new int[n];
  this->f.read((char *) data, n*sizeof(int));
  return data;
}

char *FileReader::ReadCharArray(int n) {
  char *data = new char[n];
  this->f.read(data, n);
  return data;
}

float FileReader::ReadFloat() { 
  float *x = new float;
  this->f.read(x, sizeof(float));
}
int FileReader::ReadInt() {
}
char FileReader::ReadChar() {
}
string FileReader::ReadString(int n) {
}


ArrayReader::ArrayReader(string fn, long off) {
  filename = fn;
  offset = off;
  addrAttribute = "";
  mode = ios::binary; 
}

ArrayReader::ArrayReader(string fn, long off, string pa) {
  filename = fn;
  offset = off;
  addrAttribute = pa;
  mode = ios::binary; 
}

float ArrayReader::CORIEHack(GridField *GF, int pos) {
  float addr, zpos, bot;
  addr = *(int *) GF->getAttributeVal("addr", pos);
  zpos = *(int *) GF->getAttributeVal("zpos", pos);
  bot = *(int *) GF->getAttributeVal("b", pos);
  return addr + (zpos - bot);
}

void ArrayReader::Read(GridField *Gf, int k, Array *arr) {
  
  ifstream f;
  long off;
  
  Grid *G = Gf->grid;
  f.open((const char *) filename.c_str(), this->mode | ios:: in);
  if (f.fail()) { 
    cerr << "Unable to open " << filename << endl;
    exit(1);
  }

  int size = Gf->card();

  // Look for the attribute that tells us how to read the array
  int *positions;
  if (Gf->isAttribute(addrAttribute.c_str())) {
    Array *addr = Gf->getAttribute(addrAttribute.c_str());
    if (addr->type == INT) {
      addr->getData(positions);
    } else {
  //    Warning("address attribute '%s' found, but type is not INT", addrAttribute.c_str());
      float *fpos;
      addr->getData(fpos);
      positions = new int[size];
      for (int i=0; i<Gf->card(); i++) {
	positions[i] = int(fpos[i]);
      }
    }               
  } else {
    //assume ordinal positions
    positions = new int[size];
    for (int i = 0; i<size; i++) positions[i] = i;
  }


  UnTypedPtr *data = new UnTypedPtr[size];
  Scheme *sch = arr->getScheme();
  if (sch->size() > 1) {
    ReadTuples(f, positions, size, sch, data);
  }  else {
    ReadPrimitives(f, positions, size, arr->type, data);
  }
  
  switch (arr->type) {
  case FLOAT:
    arr->shareData((float *) data, size);
    break;
  case INT:
    arr->shareData((int *) data, size);
    break;
  case OBJ:
    arr->shareData(data, size);
    break;
  default:
    cerr << "Unknown Type...\n";
    exit(1);
  }

}

void TextArrayReader::ReadPrimitives(ifstream &f, int *positions, int size, Type t, UnTypedPtr *data) {
 
  int line = 0; 
  char textline[256];
  float x;
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
        sscanf(textline, "%i", &data[i]);
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
        sscanf(textline, "%f", &data[i]);
        line++;
        
      }
      break;
      
    default:
      
      Fatal("ArrayReader: only ints and floats for now...");
  }
  
}

void ArrayReader::ReadPrimitives(ifstream &f, int *positions, int size, Type t, UnTypedPtr *data) {
  int i = 0;
  int index = 0;
  int block = 0;
  int pos = 0;

  // NOT PORTABLE
  int valsize = sizeof(float);
  
  /*
  char *foo = new char[16];
  f.seekg( offset + positions[1140] * valsize, ios_base::beg);
  f.read(foo, 16);
  cout << *(int *) foo << endl;
  cout << *(float *) foo;
  getchar();  
  */
  // Read the data in big chunks if possible
  while (i<size) {
    index = i;
    pos = positions[i];
    //    cout << positions[i] << endl;
    block = 1;
    while (positions[i + 1] == positions[i] + 1) {
      block++;
      ++i;
    }
    //cout << "read " << block << " " << valsize << " byte values to " << index << endl;
    f.seekg( offset + pos * valsize, ios_base::beg );
    f.read((char *) &data[index], block * valsize);
    //  cout << *(float *) data[0] << endl;
    //     cout << *(float *) &data[0] << endl;
    i++;
  }
}

void TextArrayReader::ReadTuples(ifstream &f, int *positions, int size, Scheme *sch, UnTypedPtr *tupptrs) {
  
  string format = "";
  Tuple *t;
  int s = sch->size();

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

void ArrayReader::ReadTuples(ifstream &f, int *positions, int size, Scheme *sch, UnTypedPtr *tupptrs) {
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
  for (int j=0; j<sch->size(); j++) {
    valsize = valsize + typesize(sch->getType(j));
  }
  
  char *data = new char[size*valsize];
  
  //vector for better allocation
  vector<Tuple> &tups = *(new vector<Tuple>(size,Tuple(sch)));
  
  //ptr structure to satisfy Array requirements.
  for (int j=0; j<size; j++) {
    tupptrs[j] = (void *) &tups[j];
  }
  
  // Read the data in big chunks if possible
  while (i<size) {
    index = i;
    pos = positions[i];
    //    cout << positions[i] << endl;
    block = 1;
    while (positions[i + 1] == positions[i] + 1) {
      block++;
      ++i;
    }
    //    cout << "read " << block << " values" << endl;
    f.seekg( offset + pos * valsize, ios_base::beg );
    f.read((char *) &data[index], block * valsize);
    for (int j=0; j<block; j++) {
      tups[index+j].assign((char *) &data[index*valsize + j*valsize ]);
      //tups[index+j].print();
    }
  //  cout << *(float *) data[0] << endl;
    i++;
  }
}

