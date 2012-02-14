

int plumevol( GridField *H, GridField *V, const char *filename, 
              int addr, string region, string dataprod ) {

  float start = gettime();
  float secs;
  
  float scale = 20.0f;
  
  computeColumnPositions(H,V);
  
  //readArray(H, 0, filename, INT, 1135132 + 8, "surf", "");
      
  // add a data column representing the order in this grid
  V->recordOrdinals("zpos");
  
  GridField *cutH = H;
  GridField *cutV = V;

  Zoom(cutH,cutV,region);
 
  scaleZ(cutV,scale);
 
  GridField *cut = makeWetGrid(cutH, cutV);
  
  computeAddresses(cut);
  
  readArray(cut, 0, filename, FLOAT, addr, "salt", "wetpos");
  
  vtkGridField *vtkgrid = toVTK(cut, "salt");
  
  cout << gettime() - start << tab << "( Total )" << endl; 
  cout << "Selectivity: " << cut->card() << ", " << (829852) << endl;
  vtkGridField *bath = makeBathymetry(cutH, scale);
 
  // call the data product visualization function
  Visualize(vtkgrid->GetOutput(), bath->GetOutput(), dataprod);
}

