// RingsSim.cpp
// This is the main file for the rings simulation.

#include<string>
#include<omp.h>

//#define PARALLEL
#define GRAVITY
#define AZIMUTHAL_MIRRORS 3
#define FULL_MIRRORS 3

#include "CollisionFinders.h"
#include "System.h"
#include "GCPopulation.h"
#include "TreeCollisionForcing.h"
//#include "CollisionForcing.h"
//#include "BoundaryConditions.h"
//#include "VariableGridCollisionHash.h"
#include "FixedGridCollisionHash.h"
#include "Distributions.h"
#include "SigmaDistributions.h"
//#include "SigmaDistributions.h"
#include "VelocityEllipsoidOutput.h"
#include "BinaryDumpOutput.h"
#include "Fixed2DBinnedOutput.h"
#include "Particle2DBinnedOutput.h"
#include "MoonForcing.h"
#include "DoubleForce.h"
#include "ParticleMoonForcing.h"
//#include "KDTree2.h"
//#include "GravTreeMirrors.h"
#include "GravCollTree.h"
#include "DoubleOutput.h"
//#include "GravCollTree.h"
#include "SlidingBrickBoundaryConditions.h"

int main(int argc,char **argv) {
//	double minx=0.0060,maxx=0.0068,miny=0.04,maxy=0.1022;
//	double minx=0.0066,maxx=0.0068,miny=0.04,maxy=0.0402;
//	double minx=0.00012,maxx=0.00022,miny=-0.02,maxy=0.0022;
	double minx=-0.00001,maxx=0.00001,miny=-0.00001,maxy=0.00001;

	double moonOrbitRadius=140000;
	double q=2.8;
	double minRadius=3e-11;
	double maxRadius=3e-10;
//	double tau=0.42;
	double eValue=2e-10;
	double iValue=3e-8;
	int outputInterval=1;
	int stepMultiple=1;
	double timeStep=6.28e-3*stepMultiple;
	double particleDensitygPercm3=0.6;
	double tau=0.5;
	int step=0;

//	omp_set_num_threads(4);
	

/***** Boundary Setup ********/
	typedef SlidingBrick Boundary;
//	typedef PeriodicWithPhiShift Boundary;
//	typedef FixedPeriodic Boundary;
	Boundary bc(minx,maxx,miny,maxy);

//	typedef SingleOrbitAzimuthal Boundary;
//	Boundary bc(minx,maxx,miny);

//	typedef ParallelPeriodicWithPhiShift<CollForcing> Boundary;
//	Boundary bc(minx,maxx,miny,maxy,pc,collForce);

//	typedef ParallelSingleOrbitAzimuthal<CollForcing> Boundary;
//	Boundary bc(minx,maxx,miny,pc,collForce);

/***** Forcing Setup ********/
//	typedef FixedGridCollisionHash Hash;
//	typedef VariableGridCollisionHash Hash;
//	typedef CollisionForcing<Hash> CollForcing;
	typedef TreeCollisionForcing<GravCollTree<Boundary> > CollForcing;
	typedef DoubleForce<GravCollTree<Boundary>,CollForcing> Forcing;
//	typedef DoubleForce<KDGravTree,Forcing1> Forcing;
//	typedef DoubleForce<ParticleMoonForcing,CollForcing> Forcing;

	GravCollTree<Boundary> gt(0.3,bc);
	CollForcing collForce(gt);
//	Forcing force;
//	MoonForcing moon(moonMass,moonE,moonPhi);
//	Forcing1 force1(moon,collForce);
//	ParticleMoonForcing pmf(1e-7);
//	Forcing force(pmf,force1);
//	Forcing force(pmf,collForce);
	Forcing force(gt,collForce);


/***** Output Setup ********/
//	typedef VelocityEllipsoidOutput Output;
//	Output output;

	typedef BinaryDumpOutput<CartCoords> Output;
	Output output(outputInterval/stepMultiple,step);

//	typedef Fixed2DBinnedOutput Output2;
//	Output2 output2(outputInterval,1000, 1,minx,maxx);

//	typedef Particle2DBinnedOutput Output2;
//	Output2 output2(outputInterval,300,1);

//	typedef ParallelBinaryDumpOutput Output;
//	Output output(outputInterval,pc);

//	typedef DoubleOutput<Output1,Output2> Output;
//	Output output(output1,output2);


/***** Population Setup ********/
	typedef GCPopulation<Boundary,FullLinearFinder<GCCoords>,Output,GCCoords, En_0p5_Et_0p5> Pop;
	StandardMass massFunc(moonOrbitRadius,particleDensitygPercm3);
	Pop pop(bc,output,timeStep,moonOrbitRadius,massFunc);


/***** Particle Distribution Setup ********/
//	RadiusDistrib rd(singleRadius);
	RadiusDistrib rd(minRadius,maxRadius,q);
//	RadiusDistrib rd(2);
//	rd.setBin(0,rad1,frac1);
//	rd.setBin(1,rad2,frac2);
//	RandomSquareEIOrbits distrib(numBodies,bc.getMinX(),bc.getMaxX(),bc.getMinY(),bc.getMaxY(),eValue,iValue,rd);
//	RandomSquareEIOrbitsWithCheck<Boundary> distrib(numBodies,bc.getMinX(),bc.getMaxX(),bc.getMinY(),bc.getMaxY(),eValue,iValue,rd,bc);
	// TauRandomSquareEIOrbitsWithCheck<Boundary, GCCoords> distrib(tau,bc.getMinX(),bc.getMaxX(),bc.getMinY(),bc.getMaxY(),eValue,iValue,rd,bc);
//	SigmaRandomSquareEIOrbitsWithCheck<Boundary,GCCoords> distrib(sigma,particleDensitygPercm3,moonOrbitRadius,bc.getMinX(),bc.getMaxX(),bc.getMinY(),bc.getMaxY(),eValue,iValue,rd,bc);
//	TauRandomGaussianEIOrbitsWithCheck<Boundary> distrib(tau,bc.getMinX(),bc.getMaxX(),bc.getMinY(),bc.getMaxY(),eValue,iValue,rd,bc);
	FileRecoverAll distrib(step);
	pop.randomDistribution(distrib);
	for (int i = 0; i<pop.getNumBodies(); i++){
		ParticleIndex pi={i};
		printf("%d %e %e %e %e %e %e\n", i, pop.getx(pi), pop.gety(pi), pop.getz(pi), pop.getvx(pi), pop.getvy(pi), pop.getvz(pi));
	}
	
//	pop.addSingleParticleGCandClear((bc.getMaxX()+bc.getMinX())*0.5,(bc.getMaxY()+bc.getMinY())*0.5,0.0,0.0,0.0,0.0,1e-8);

/***** Do Simulation ********/
	System<Pop,Forcing> sys(pop,force);

	printf("Start Simulation with %d particles.\n",pop.getNumBodies());
	for(int i=0; i<5000/stepMultiple+1; ++i) {
		step=i;
		printf("Step %d\n",i);
		sys.advance();
	}
	

	return 0;
}
