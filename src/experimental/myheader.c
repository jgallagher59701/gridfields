#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
  int nodes;
  int elements;
  float *x;
  float *y;
  float *depth;
  int *bottom_layer;
  int *top_layer;
  int *nm;
}hgrid;

typedef struct{
  float zmsl;
  int nlevels;
  float *z_level;
}vgrid;

typedef struct{
  char data_format[48];
  char version[48];
  char start_time[48];
  char variable_type[48];
  char variable_dimension[48];
  int  nsteps;
  float dt;
  int   skip;
  int   flag_sv;
  int   flag_dm;
  float z_des;
  vgrid layers;
  hgrid grid;
}header;

typedef struct {
  FILE *fid;
  char id[10];
  float xloc;
  float yloc;
  float staie[3];
  int   element_pos;
}station;

header read_header(FILE *fid){
  int i;
  header fhead;

  fread(fhead.data_format, sizeof(char), 48,fid);
  fread(fhead.version, sizeof(char), 48,fid);
  fread(fhead.start_time, sizeof(char), 48,fid);
  fread(fhead.variable_type, sizeof(char), 48,fid);
  fread(fhead.variable_dimension, sizeof(char), 48,fid);
  fread(&(fhead.nsteps), sizeof(int), 1,fid);
  fread(&(fhead.dt), sizeof(float), 1, fid);
  fread(&(fhead.skip), sizeof(int), 1, fid);
  fread(&(fhead.flag_sv), sizeof(int), 1, fid);
  fread(&(fhead.flag_dm), sizeof(int), 1, fid);
  fread(&(fhead.z_des), sizeof(float), 1, fid);

  fread(&(fhead.layers.zmsl), sizeof(float), 1, fid);
  fread(&(fhead.layers.nlevels), sizeof(int), 1, fid);
  fhead.layers.z_level = (float *) calloc(fhead.layers.nlevels, sizeof(float));
  fread(fhead.layers.z_level, sizeof(float), fhead.layers.nlevels, fid);

  fread(&(fhead.grid.nodes), sizeof(int), 1, fid);
  fread(&(fhead.grid.elements), sizeof(int), 1, fid);
  fhead.grid.x = (float *) calloc(fhead.grid.nodes, sizeof(float));
  fhead.grid.y = (float *) calloc(fhead.grid.nodes, sizeof(float));
  fhead.grid.depth = (float *) calloc(fhead.grid.nodes, sizeof(float));
  fhead.grid.bottom_layer = (int *) calloc(fhead.grid.nodes, sizeof(int));
  fhead.grid.top_layer = (int *) calloc(fhead.grid.nodes, sizeof(int));
  fhead.grid.nm = (int *) calloc((fhead.grid.elements)*3, sizeof(int));
  
  for(i=0;i<fhead.grid.nodes;i++){
    fread(&(fhead.grid.x[i]), sizeof(float), 1, fid);
    fread(&(fhead.grid.y[i]), sizeof(float), 1, fid);
    fread(&(fhead.grid.depth[i]), sizeof(float), 1, fid);
    fread(&(fhead.grid.bottom_layer[i]), sizeof(int), 1, fid);    
  }
  fread(fhead.grid.nm, sizeof(int), (fhead.grid.elements)*3, fid);

  return fhead;
}

void write_header(FILE *fid, header fhead){
  int i;

  fwrite(fhead.data_format, sizeof(char), 48,fid);
  fwrite(fhead.version, sizeof(char), 48,fid);
  fwrite(fhead.start_time, sizeof(char), 48,fid);
  fwrite(fhead.variable_type, sizeof(char), 48,fid);
  fwrite(fhead.variable_dimension, sizeof(char), 48,fid);
  fwrite(&(fhead.nsteps), sizeof(int), 1,fid);
  fwrite(&(fhead.dt), sizeof(float), 1, fid);
  fwrite(&(fhead.skip), sizeof(int), 1, fid);
  fwrite(&(fhead.flag_sv), sizeof(int), 1, fid);
  fwrite(&(fhead.flag_dm), sizeof(int), 1, fid);
  fwrite(&(fhead.z_des), sizeof(float), 1, fid);

  fwrite(&(fhead.layers.zmsl), sizeof(float), 1, fid);
  fwrite(&(fhead.layers.nlevels), sizeof(int), 1, fid);
  fwrite(fhead.layers.z_level, sizeof(float), fhead.layers.nlevels, fid);

  fwrite(&(fhead.grid.nodes), sizeof(int), 1, fid);
  fwrite(&(fhead.grid.elements), sizeof(int), 1, fid);
   
  for(i=0;i<fhead.grid.nodes;i++){
    fwrite(&(fhead.grid.x[i]), sizeof(float), 1, fid);
    fwrite(&(fhead.grid.y[i]), sizeof(float), 1, fid);
    fwrite(&(fhead.grid.depth[i]), sizeof(float), 1, fid);
    fwrite(&(fhead.grid.bottom_layer[i]), sizeof(int), 1, fid);    
  }
  fwrite(fhead.grid.nm, sizeof(int), (fhead.grid.elements)*3, fid);

  return;
}

float datestring2corie(char *s){
  char cyear[5], cmonth[3], cday[3];
  int  year, month, day, year_tmp;
  int  days_month[2][12] = { {0,31,59,90,120,151,181,212,243,273,304,334},
			     {0,31,60,91,121,152,182,213,244,274,305,335}
                           };
  float corie;

  cmonth[0] = s[0];
  cmonth[1] = s[1];
  cmonth[2] = '\0';
  cday[0] = s[3];
  cday[1] = s[4];
  cday[2] = '\0';
  cyear[0] = s[6];
  cyear[1] = s[7];
  cyear[2] = s[8];
  cyear[3] = s[9];
  cyear[4] = '\0';

  year = atoi(cyear);
  month = atoi(cmonth);
  day = atoi(cday);

  corie = 0.0;
  year_tmp = 1996;
  while (year_tmp != year){
    if (year_tmp < year){
      corie += (float) (365 + 1*isleapyear(year_tmp));
      year_tmp++;
    }
    else {
      corie -= (float) (365 + 1*isleapyear(year_tmp-1));
      year_tmp--;      
    }
  }
  corie += (float) (days_month[isleapyear(year)][month-1] + day);
  return corie;
}

int isleapyear(int year){
  int k = 0;

  if (year%100 == 0){
    if (year%400 == 0){
      k = 1;
    }
  }
  else if (year%4 == 0){
    k = 1;
  }
  return k;
}

int compute_header_length(header fhead){
  int nbytes;

  nbytes = 0;
  nbytes += 48*sizeof(char)*5;
  nbytes += sizeof(int)*(7 + fhead.grid.nodes + fhead.grid.elements*3);
  nbytes += sizeof(float)*(3 + fhead.layers.nlevels + fhead.grid.nodes*3);

  return nbytes;
}

int main(int argc, char *argv[]){

  FILE   *fscalar, *fout_max, *fout_min;
  char   fname[100], inDIR[100], outDIR[100];
  float  *scalar, *max_scalar, *min_scalar, time;
  int    *start_loc_scalar, Nvalues, Nfiles;
  int    i,j,k, time_step, header_skip;
  long   byte_length3ds;
  header file_head;

  header read_header(FILE *);
  float  datestring2corie(char *);
  int    isleapyear(int);
  void   write_header(FILE *,header);

  if (argc != 5){
     fprintf(stderr,"Usage : %s <input file path> <output file path> <variable> <number files>\n",argv[0]);
     exit(1);
  }

  sprintf(inDIR,"%s",argv[1]);
  sprintf(outDIR,"%s",argv[2]);
  Nfiles = atoi(argv[4]);

  sprintf(fname,"%s/%d_%s%s",inDIR,1,argv[3],".63");
  if ( (fscalar = fopen(fname,"r")) == NULL){
    fprintf(stderr,"Could not open file %s \n",fname);
    exit(1);
  }

  file_head = read_header(fscalar);

  start_loc_scalar = (int *)calloc(file_head.grid.nodes,sizeof(int));
  Nvalues = 0.0;
  for (j=0;j<file_head.grid.nodes;j++){
    Nvalues += file_head.layers.nlevels-file_head.grid.bottom_layer[j]+1;
  }
  start_loc_scalar[0] = 0;
  for (j=1;j<file_head.grid.nodes;j++){
    start_loc_scalar[j] = start_loc_scalar[j-1] + file_head.layers.nlevels-file_head.grid.bottom_layer[j-1]+1;
  }

  header_skip = compute_header_length(file_head);

  scalar = (float *)calloc(Nvalues,sizeof(float));
  max_scalar = (float *)calloc(Nvalues,sizeof(float));
  min_scalar = (float *)calloc(Nvalues,sizeof(float));
  for (k=0;k<Nvalues;k++){
    max_scalar[k] = -99.0;
    min_scalar[k] = -99.0;
  }

  for (i=1;i<=Nfiles;i++){
    if (i != 1){
      sprintf(fname,"%s/%d_%s%s",inDIR,i,argv[3],".63");
      if ( (fscalar = fopen(fname,"r")) == NULL){
	fprintf(stderr,"Could not open file %s \n",fname);
	exit(1);
      }
    }
    for(j=0;j<file_head.nsteps;j++){
      byte_length3ds = (long) (header_skip + (long) j*(sizeof(float)*(1+Nvalues)+sizeof(int)*(1+file_head.grid.nodes)) + (long)(sizeof(float)*1+ sizeof(int)*(1+file_head.grid.nodes)));
      fseek(fscalar,byte_length3ds,0);
      if (fread(scalar,sizeof(float),Nvalues,fscalar) != Nvalues){
	break;
      }
      for (k=0;k<Nvalues;k++){
	if (scalar[k]>= 0){
	  if (scalar[k] > max_scalar[k]){
	    max_scalar[k] = scalar[k];
	  }
	  if (min_scalar[k]<0 || scalar[k]<min_scalar[k]){
	    min_scalar[k] = scalar[k];
	  }
	}
      }
    }
    fprintf(stdout,"Completed file number %d \n",i);
    fclose(fscalar);
  }

  file_head.nsteps=1;
  for(j=0;j<file_head.grid.nodes;j++){
    file_head.grid.top_layer[j] = file_head.layers.nlevels;
  }
  time = 0.0;
  time_step = 1.0;

  sprintf(fname,"%s/%s%s",outDIR,argv[3],"_max.63");
  if ( (fout_max = fopen(fname,"w")) == NULL){
    fprintf(stderr,"Could not open file %s \n",fname);
    exit(1);
  }
  
  sprintf(fname,"%s/%s%s",outDIR,argv[3],"_min.63");
  if ( (fout_min = fopen(fname,"w")) == NULL){
    fprintf(stderr,"Could not open file %s \n",fname);
    exit(1);
  }
  
  write_header(fout_max,file_head);
  fwrite(&time,sizeof(float),1,fout_max);
  fwrite(&time_step,sizeof(int),1,fout_max);
  fwrite(file_head.grid.top_layer,sizeof(int),file_head.grid.nodes,fout_max);
  fwrite(max_scalar,sizeof(float),Nvalues,fout_max);
  fclose(fout_max);

  write_header(fout_min,file_head);
  fwrite(&time,sizeof(float),1,fout_min);
  fwrite(&time_step,sizeof(int),1,fout_min);
  fwrite(file_head.grid.top_layer,sizeof(int),file_head.grid.nodes,fout_min);
  fwrite(min_scalar,sizeof(float),Nvalues,fout_min);
  fclose(fout_min);

}
