////////////////////////////////////////////////////
// user-specifiable parameters 
////////////////////////////////////////////////////
// side length 
L = 0.10;

// center 
X0 = 0;
Y0 = 0;

// number of elements per side length
N = 8;

// meshing fineness; i think this is actually not used since we will be 
// defining a structured mesh
lc = L/2; 

////////////////////////////////////////////////////
// end of user-specifiable parameters 
////////////////////////////////////////////////////
Point(1) = { 0, 0, 0, lc};
Point(2) = { L, 0, 0, lc};
Point(3) = { L, L, 0, lc};
Point(4) = { 0, L, 0, lc};

Line(1)  = { 1, 2 };
//Line(2)  = { 2, 3 };
//Line(3)  = { 3, 4 };
//Line(4)  = { 4, 1 };

Transfinite Line{1} = N+1;
Extrude { 0, L, 0  } { Line{1}; Layers{N}; }
//Physical Surface(1) = {6, 15, 19, 23, 27, 28};
