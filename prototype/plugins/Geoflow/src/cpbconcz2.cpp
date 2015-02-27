///  @file    cpbconcz2.cpp
///  @author  Peter Hui, Andrew Stevens, Kyle Monson, Zhan Chen, Guowei Wei
///  @brief this is the top level of geoflow
///  @ingroup Geoflow
///  @version $Id$
///  @attention
///  @verbatim
///
/// APBS -- Adaptive Poisson-Boltzmann Solver
///
///  Nathan A. Baker (nathan.baker@pnnl.gov)
///  Pacific Northwest National Laboratory
///
///  Additional contributing authors listed in the code documentation.
///
/// Copyright (c) 2010-2014 Battelle Memorial Institute. Developed at the
/// Pacific Northwest National Laboratory, operated by Battelle Memorial
/// Institute, Pacific Northwest Division for the U.S. Department of Energy.
///
/// Portions Copyright (c) 2002-2010, Washington University in St. Louis.
/// Portions Copyright (c) 2002-2010, Nathan A. Baker.
/// Portions Copyright (c) 1999-2002, The Regents of the University of
/// California.
/// Portions Copyright (c) 1995, Michael Holst.
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// Redistributions of source code must retain the above copyright notice, this
/// list of conditions and the following disclaimer.
///
/// Redistributions in binary form must reproduce the above copyright notice,
/// this list of conditions and the following disclaimer in the documentation
/// and/or other materials provided with the distribution.
///
/// Neither the name of the developer nor the names of its contributors may be
/// used to endorse or promote products derived from this software without
/// specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
/// THE POSSIBILITY OF SUCH DAMAGE.
///
/// @endverbatim


#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "cpbconcz2.h"
#include "modules.h"

#define MAGIC_FOO 332.06364

void normalizeSurfuAndEps(Mat<>& surfu, Mat<>& eps, double epsilons,
		double epsilonp)
{
	for (size_t i = 0; i < surfu.size(); i++) {
		if (surfu[i] > 1000.0) { surfu[i] = 1000.0; }
		if (surfu[i] < 0.0) { surfu[i] = 0.0; }
		eps[i] = epsilonp + (epsilons - epsilonp) * ((1000.0 - surfu[i])/1000.0 );
	}
}

/*
 * Compute soleng1 and soleng2 (solvation).  This is an artifact of refactoring
 * the original F90 code.
 * Parameters:
 * 		soleng:		The soleng variable to set (compute).  For soleng1, the phi
 *					array should be phi; for soleng2, the phiarray should be
 *					phivoc.
 * 		phi:		Either phi or phivoc array, depending on which soleng
 * 					variable we are computing
 *		phiDims:	Vector holding the dimensions of the phi array
 * 		natm:		The number of atoms
 * 		loc_qt:		The loc_qt array.  This is a [3][8][natm] int array, but
 * 					must be passed as a pointer, since natm is a variable.
 * 		charget:	charget array.  This is an [8][natm] int array.
 */
void computeSoleng(double& soleng, Mat<>& phi, Mat<>& charget,
		Mat<size_t>& loc_qt)
{
	soleng = 0.0;
	for (size_t iind = 1; iind <= charget.nx(); iind++) {
		for (size_t jind = 1; jind <= charget.ny(); jind++) {
			size_t i = loc_qt(iind,jind,1);
			size_t j = loc_qt(iind,jind,2);
			size_t k = loc_qt(iind,jind,3);

			soleng += 0.5*charget(iind,jind)*phi(i,j,k);
		}
	}
}

GeoflowOutput geoflowSolvation(double xyzr[][XYZRWIDTH], size_t natm,
		GeoflowInput gfin)
{
	int ffmodel = gfin.ffmodel;
	double extvalue = gfin.extvalue;
	double* pqr = gfin.pqr;
	int maxstep = gfin.maxstep;
	double crevalue = gfin.crevalue;
	int iadi = gfin.iadi;
	double tottf = gfin.tottf;
	double* ljepsilon = gfin.ljepsilon;
	double alpha = gfin.alpha;
	int igfin = gfin.igfin;
	double epsilons = gfin.epsilons;
	double epsilonp = gfin.epsilonp;
	double tol = gfin.tol;
	int iterf = gfin.iterf;
	double tpb = gfin.tpb;
	int itert = gfin.itert;
	double pres = gfin.pres;
	double gama = gfin.gama;
	double tauval = gfin.tauval;
	double prob = gfin.prob;
	int vdwdispersion = gfin.vdwdispersion;
	double sigmas = gfin.sigmas;
	double density = gfin.density;
	double epsilonw = gfin.epsilonw;

	double elec = 0.0, area = 0.0, volume = 0.0, attint = 0.0;

	double dx = gfin.dcel[0], dy = gfin.dcel[1], dz=gfin.dcel[2];
	comdata.deltax = dx;
	comdata.deltay = dy;
	comdata.deltaz = dz;
	comdata.pi = acos(-1.0);

	lj.ffmodel = ffmodel;
	lj.tauval = tauval;
	lj.prob = prob;
	lj.vdwdispersion = vdwdispersion;
	lj.sigmas = sigmas;
	lj.density = density;
	lj.epsilonw = epsilonw;
	// uberfoo
	std::cout << "density = " << density << std::endl;
	std::cout << "gama = " << gama << std::endl;
	lj.roro = density / gama;
	std::cout << "roro = " << lj.roro << std::endl;
	double potcoe = 1.0 / gama;
	lj.conms = pres / gama;
	std::cout << "conms = " << lj.conms << std::endl;

	if(ffmodel == 1) {
//        std::replace_if(xyzr, numbers+6, bind1st(equal_to<int>(),0) );
		for(size_t i = 0; i < natm; ++i){
			xyzr[i][3] = (xyzr[i][3] < 1e-6) ? 1.21 : xyzr[i][3];
		}
	}

	domainini(xyzr, natm, extvalue);

	Mat<> charget(natm, 8);
	Mat<> corlocqt(natm, 8, 3);
	Mat<size_t> loc_qt(natm,8,3);

	for (size_t iatm = 1; iatm <= natm; iatm++) {
		chargedist(xyzr, pqr, charget, corlocqt, loc_qt, iatm);
	}

	int nx = comdata.nx, ny = comdata.ny, nz = comdata.nz;
	comdata.xc.resize(nx);
	for (int i=0; i<nx; i++) {
		comdata.xc[i] = comdata.xleft + (i-1)*comdata.dcel;
	}

	comdata.yc.resize(ny);
	for (int i=0; i<ny; i++) {
		comdata.yc[i] = comdata.yleft + (i-1)*comdata.dcel;
	}

	comdata.zc.resize(nz);
	for (int i=0; i<nz; i++) {
		comdata.zc[i] = comdata.zleft + (i-1)*comdata.dcel;
	}

	std::vector<double> solv(maxstep+1);
	double diffEnergy = 100;

	int iloop = 0, ipath = 0;
	double tott = 0.0;

	std::cout << "dimensions:\t" << nx << " " << ny << " " << nz << std::endl;
	std::cout << "grid spacing:\t" << dx << " " << dy << " " << dz << std::endl;

	Mat<> phi(nx,ny,nz,dx,dy,dz), phix(phi), phivoc(phi), surfu(phi), eps(phi),
			bg(phi);

	while ((iloop < maxstep) && (diffEnergy > crevalue)) {
		iloop++;
		double deltat = 0; //this is wrong for adi...
		if (!iadi) {
			deltat =  pow(dx*dy*dz, 2.0/3.0)/4.5;
		}
		if (ipath == 0) /* will always be true, but this is how
				  * it was in the original F90 code.
				  * TODO: rewrite this to remove the ipath variable (?) */ {
			double totnow = tottf - iloop + 1;
			if (totnow > 1.0) {
				tott = totnow;
			} else {
				tott = 1.0;
			}
		} else {
			if (iloop == 1) {
				tott = tottf;
			} else {
				tott = iloop*pow(dx*dy*dz, 2.0/3.0)/4.5;
			}
		}

		area = volume = attint = 0.0;
		yhsurface(xyzr, ljepsilon, natm, tott, deltat, phix, surfu, iloop, area,
				volume, attint, alpha, iadi, igfin);
		normalizeSurfuAndEps(surfu, eps, epsilons, epsilonp);

		if (iloop == 1) {
			seteqb(bg, xyzr, pqr, charget, corlocqt, epsilons);
		}

		int iter = 1000;
		double fpb, titer = 0.0;
		pbsolver(eps, phi, bg, tol, iter);
		if (iloop == 1) {
			fpb = titer;
			iterf = iter;
		}
		tpb = tpb + titer;
		itert += iter;

		eps = epsilonp;
		if (iloop == 1) {
			pbsolver(eps, phivoc, bg, tol, iter);
		}

		for (size_t ix = 2; ix <= comdata.nx - 1; ix++) {
			for (size_t iy = 2; iy <= comdata.ny - 1; iy++) {
				for (size_t iz = 2; iz <= comdata.nz - 1; iz++) {
					double phixx = phi(ix+1,iy,iz) - phi(ix-1,iy,iz)/(2.0*dx);
					double phixy = phi(ix,iy+1,iz) - phi(ix,iy-1,iz)/(2.0*dy);
					double phixz = phi(ix,iy,iz+1) - phi(ix,iy,iz-1)/(2.0*dz);

					phix(ix,iy,iz) = 0.5 * (epsilons - epsilonp) * (phixx *
							phixx +	phixy * phixy + phixz * phixz) * potcoe;
				}
			}
		}

		// solvation
		double soleng1, soleng2;
		soleng1 = soleng2 = 0.0;
		computeSoleng(soleng1, phi, charget, loc_qt);
		computeSoleng(soleng2, phivoc, charget, loc_qt);
		elec = (soleng1 - soleng2) * MAGIC_FOO;
		solv[iloop - 1] = elec + gama * (area + volume * lj.conms + attint *
				lj.roro);
		if (iloop > 1) {
			diffEnergy = fabs((solv[iloop - 1] - solv[iloop - 2]));
		}
		std::cout << "diffEnergy = " << diffEnergy << std::endl;
		std::cout << "solv[iloop-1] = " << solv[iloop-1] << std::endl;
	}



	double sumpot = area + volume * lj.conms + attint * lj.roro;
	double nonpolarSolvation = sumpot*gama;
	double totalSolvation = nonpolarSolvation + elec;

	GeoflowOutput gfOut;
	gfOut.area = area;
	gfOut.volume = volume;
	gfOut.attint = attint;
	gfOut.sumpot = sumpot;
	gfOut.totalSolvation = totalSolvation;
	gfOut.nonpolarSolvation = nonpolarSolvation;
	gfOut.elecSolvation = elec;

	return gfOut;
}
