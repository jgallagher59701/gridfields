
  // iterate over timesteps
  int timesteps = 96;
  
  OneGrid *T = new OneGrid("T", timesteps);
  cout << "make range: " << endl;
  Range *addresses = new Range("t", timesteps);
  //( (Array *) addresses)->print();
  GridField *Tt = new GridField(T, 0, (Array *) addresses);
  
  // an attribute with the starts of each timestep
  int timevals = 2*4;
  int surf = H->card()*4;
  int timestephead = timevals + surf;
  int header = 1135132;
  stringstream addrexpr;
  addrexpr << "(t*(" << timestephead;
  addrexpr << " + " << totalsize*4 << "))";
  addrexpr << "+" << timestephead;
  ApplyOp::Apply(Tt, "addresses", addrexpr.str());
  
  ArrayReader *ar;
  float address;
  stringstream attrname;
  attrname << "salt";
  Array *salt = new Array("salt", FLOAT);
  vtkGridField *vtkgrid;
  for (int t=0; t<timesteps; t++) {
    address = *(float *) Tt->getAttributeVal("addresses", t);
    ar = new ArrayReader(argv[1], header + int(address), "wetpos");
    ar->Read(cut, 0, salt);
    cut = new GridField(G, 0);
    cut->Bind(salt);
    
 /* 
    secs = gettime(); 
    vtkgrid = vtkGridField::New();
    vtkgrid->UseNamedPerspective();
    vtkgrid->SetGridField(cut);
    vtkgrid->Update(); 
    cout << gettime() - secs << tab << "( to VTK )" << endl; 
  
    vtkgrid->GetOutput()->GetPointData()->SetActiveScalars("salt");
  
    Visualize(vtkgrid->GetOutput(), bath->GetOutput(), dataprod);
    */
  }  
