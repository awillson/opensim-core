// VectorFunctionForActuators.cpp
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
* Copyright (c)  2005, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*  
 * Author: Frank C. Anderson 
 */

// INCLUDES
#include "VectorFunctionForActuators.h"
#include "Model.h"
#include "AbstractActuator.h"

using namespace OpenSim;
using namespace std;

//=============================================================================
// DESTRUCTOR AND CONSTRUCTORS
//=============================================================================
//_____________________________________________________________________________
/**
 * Destructor.
 */
VectorFunctionForActuators::~VectorFunctionForActuators()
{
}
//_____________________________________________________________________________
/**
 * Constructor.
 */
VectorFunctionForActuators::
VectorFunctionForActuators(ModelIntegrand *aIntegrand) :
	VectorFunctionUncoupledNxN(aIntegrand->getModel()->getNumControls()),
	_f(0.0)
{
	setNull();
	_integrand = aIntegrand;
	_integrator = new IntegRKF(aIntegrand,5.0e-6,1.0e-7);
	_integrator->setMaxDT(1.0e-3);
	_f.setSize(getNX());
}
//_____________________________________________________________________________
/**
 * Copy constructor.
 *
 * @param aVectorFunction Function to copy.
 */
VectorFunctionForActuators::
VectorFunctionForActuators(const VectorFunctionForActuators &aVectorFunction) :
	VectorFunctionUncoupledNxN(aVectorFunction),
	_f(0.0)
{
	setNull();

	// ASSIGN
	setEqual(aVectorFunction);
}
//_____________________________________________________________________________
/**
 * Copy this object.
 *
 * @return Pointer to a copy of this object.
 */
Object* VectorFunctionForActuators::
copy() const
{
	VectorFunctionForActuators *func =
		new VectorFunctionForActuators(*this);
	return(func);
}


//=============================================================================
// CONSTRUCTION
//=============================================================================
//_____________________________________________________________________________
/**
 * Set all member variables to NULL values.
 */
void VectorFunctionForActuators::
setNull()
{
	setType("VectorFunctionForActuators");
	_ti = 0.0;
	_tf = 0.0;
	_integrand = NULL;
	_integrator = NULL;
}

//_____________________________________________________________________________
/**
 * Set all member variables equal to the members of another object.
 * Note that this method is private.  It is only meant for copying the data
 * members defined in this class.  It does not, for example, make any changes
 * to data members of base classes.
 */
void VectorFunctionForActuators::
setEqual(const VectorFunctionForActuators &aVectorFunction)
{
}

//=============================================================================
// OPERATORS
//=============================================================================
//_____________________________________________________________________________
/**
 * Assignment operator.
 *
 * @return Reference to this object.
 */
VectorFunctionForActuators& VectorFunctionForActuators::
operator=(const VectorFunctionForActuators &aVectorFunction)
{
	// BASE CLASS
	VectorFunctionUncoupledNxN::operator=(aVectorFunction);

	// DATA
	setEqual(aVectorFunction);

	return(*this);
}


//=============================================================================
// SET & GET
//=============================================================================
//-----------------------------------------------------------------------------
// INTIAL TIME
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Set the initial time of the simulation.
 *
 * @param aTI Initial time.
 */
void VectorFunctionForActuators::
setInitialTime(double aTI)
{
	_ti = aTI;
}
//_____________________________________________________________________________
/**
 * Get the initial time of the simulation.
 *
 * @return Initial time.
 */
double VectorFunctionForActuators::
getInitialTime() const
{
	return(_ti);
}

//-----------------------------------------------------------------------------
// FINAL TIME
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Set the final time of the simulation.
 *
 * @param aTF Final time.
 */
void VectorFunctionForActuators::
setFinalTime(double aTF)
{
	_tf = aTF;
}
//_____________________________________________________________________________
/**
 * Get the final time of the simulation.
 *
 * @return Final time.
 */
double VectorFunctionForActuators::
getFinalTime() const
{
	return(_tf);
}

//-----------------------------------------------------------------------------
// TARGET FORCES
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Set the target actuator forces.
 *
 * @param aF Array of target forces.
 */
void VectorFunctionForActuators::
setTargetForces(const double *aF)
{
	int i,N=getNX();
	for(i=0;i<N;i++) _f[i] = aF[i];
}
//_____________________________________________________________________________
/**
 * Get the target actuator forces.
 *
 * @param rF Array of target forces.
 */
void VectorFunctionForActuators::
getTargetForces(double *rF) const
{
	int i,N=getNX();
	for(i=0;i<N;i++) rF[i] = _f[i];
}


//-----------------------------------------------------------------------------
// INTEGRAND
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Get the integrand.
 *
 * @return Integrand.
 */
ModelIntegrand* VectorFunctionForActuators::
getIntegrand()
{
	return(_integrand);
}



//=============================================================================
// EVALUATE
//=============================================================================
//_____________________________________________________________________________
/**
 * Evaluate the vector function.
 *
 * @param aX Array of controls.
 * @param aF Array of actuator force differences.
 */
void VectorFunctionForActuators::
evaluate(const double *aX,double *rF)
{
	int i;
	int N = getNX();
	Model *model = _integrand->getModel();
	int nyModel = model->getNumStates();

	// Controls
	ControlSet *controlSet = _integrand->getControlSet();
	controlSet->setControlValues(_tf,aX);

	// States
	Array<double> y(0.0,_integrand->getSize());
	_integrand->getInitialStates(&y[0]);

	// Integration
	_integrator->integrate(_ti,_tf,&y[0],0.000001);

	// Actuator forces
	Array<double> yModel(0.0,nyModel);
	_integrand->convertStatesIntegrandToModel(_tf,&y[0],&yModel[0]);
	// Need to set the controls here because the integrator would have last evaluated the controls at some
	// intermediate time < _tf so they won't necessarily be at their desired value.
	model->setControls(aX);
	model->setStates(&yModel[0]);
	ActuatorSet *actuatorSet = model->getActuatorSet();
	actuatorSet->computeActuation();

	// Vector function values
	for(i=0;i<N;i++) {
		rF[i] = (*actuatorSet)[i]->getForce() - _f[i];
	}
}
//_____________________________________________________________________________
/**
 * Evaluate the vector function.
 *
 * @param aX Array of controls.
 * @param aF Array of actuator force differences.
 */
void VectorFunctionForActuators::
evaluate(const Array<double> &aX,Array<double> &rF)
{
	evaluate(&aX[0],&rF[0]);
}
//_____________________________________________________________________________
/**
 * Evaluate the vector function.
 *
 * @param aX Array of controls.
 * @param aF Array of actuator force differences.
 */
void VectorFunctionForActuators::
evaluate(const Array<double> &aX,Array<double> &rF,
			const Array<int> &aDerivWRT)
{
	cout<<"\n\nVectorFunctionForActuators.evaluate:  ";
	cout<<"Stupid Head! What are you trying to do?\n\n";
}

