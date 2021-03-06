/* Copyright (C) 2005-2011 M. T. Homer Reid
 *
 * This file is part of SCUFF-EM.
 *
 * SCUFF-EM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SCUFF-EM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * scuff-LDOS.h    -- header file for scuff-LDOS
 *
 * homer reid      -- 3/2015
 */
#ifndef SCUFFLDOS_H
#define SCUFFLDOS_H

#include <libhrutil.h>
#include <libhmat.h>
#include <libTriInt.h>
#include <BZIntegration.h>
#include "libscuff.h"

using namespace scuff;

#define FILETYPE_LDOS 0
#define FILETYPE_BYK  1

/***************************************************************/
/* data structure containing everything needed to execute an   */
/* LDOS calculation                                            */
/***************************************************************/
typedef struct SLDData
 {
   // data on the BEM geometry and linear algebra workspaces
   RWGGeometry *G;
   HMatrix *M;

   // data on evaluation points and DGFs at evaluation points
   HMatrix **XMatrices, **GMatrices;
   char **EPFileBases;
   bool *WrotePreamble[2];
   int NumXGMatrices;
   int TotalEvalPoints;

   // fields relevant for periodic geometries
   void **ABMBCache;

   // other miscellaneous options
   char *FileBase;
   double RelTol;
   int MaxEvals;
   bool LDOSOnly;
   MatProp *HalfSpaceMP;
   bool GroundPlane;

   // internal data storage for BZ-integrated calculations
   cdouble Omega;
   double *kBloch;

 } SLDData;

/***************************************************************/
/* Function prototypes *****************************************/
/***************************************************************/

// CreateLDOSData.cc
void WriteFilePreamble(char *FileName, int FileType, int LDim);
SLDData *CreateSLDData(char *GeoFile, char **EPFiles, int nEPFiles);

// GetLDOS.cc
void WriteData(SLDData *Data, cdouble Omega, double *kBloch,
               int FileType, int WhichMatrix, 
               double *Result, double *Error);
void WriteData(SLDData *Data, cdouble Omega, double *kBloch,
               int FileType, double *Result, double *Error);
void GetLDOS(void *Data, cdouble Omega, double *kBloch, 
             double *Result);

// AnalyticalDGFs.cc
int GetHalfSpaceDGFs(double z, cdouble Omega, double kBloch[2],
                     HMatrix *RLBasis, double BZVolume, MatProp *MP,
                     double RelTol, double AbsTol, int MaxCells,
                     cdouble GE[3][3], cdouble GM[3][3]);

void GetGroundPlaneDGFs(double z, cdouble Omega, double *kBloch,
                        HMatrix *LBasis, cdouble GE[3][3], cdouble GM[3][3]);

#endif //#ifndef SCUFFLDOS_H
