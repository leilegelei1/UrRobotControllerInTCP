//
// File: ForceController.cpp
//
// Code generated for Simulink model 'ForceController'.
//
// Model version                  : 1.146
// Simulink Coder version         : 8.11 (R2016b) 25-Aug-2016
// C/C++ source code generated on : Mon Apr 24 16:46:02 2017
//
// Target selection: ert.tlc
// Embedded hardware selection: 32-bit Generic
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "ForceController.h"
#include "ForceController_private.h"

// Exported block signals
real_T force_error[6];                 // '<Root>/Sum2'

// Exported block parameters
real_T PID_D[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } ;// Variable: D
                                                    //  Referenced by: '<S1>/Derivative Gain'


real_T PID_I[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } ;// Variable: I
                                                    //  Referenced by: '<S1>/Integral Gain'


real_T PID_N[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } ;// Variable: N
                                                    //  Referenced by: '<S1>/Filter Coefficient'


real_T PID_P[6] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 } ;// Variable: P
                                                    //  Referenced by: '<S1>/Proportional Gain'


static void rate_scheduler(RT_MODEL_ForceController_T *const ForceController_M);

//
//   This function updates active task flag for each subrate.
// The function is called at model base rate, hence the
// generated code self-manages all its subrates.
//
static void rate_scheduler(RT_MODEL_ForceController_T *const ForceController_M)
{
  // Compute which subrates run during the next base time step.  Subrates
  //  are an integer multiple of the base rate counter.  Therefore, the subtask
  //  counter is reset when it reaches its limit (zero means run).

  (ForceController_M->Timing.TaskCounters.TID[1])++;
  if ((ForceController_M->Timing.TaskCounters.TID[1]) > 3) {// Sample time: [0.008s, 0.0s] 
    ForceController_M->Timing.TaskCounters.TID[1] = 0;
  }
}

// Model step function
void ForceControllerClass::step(const real_T (&force_ee)[6], const real_T
  (&gravity_ee)[6], real_T (&poseOffset_ee)[6])
{
  int32_T i;
  real_T rtb_Gain;
  real_T rtb_FilterCoefficient;
  if ((&ForceController_M)->Timing.TaskCounters.TID[1] == 0) {
    for (i = 0; i < 6; i++) {
      // Sum: '<Root>/Sum2' incorporates:
      //   Inport: '<Root>/force_ee'
      //   Inport: '<Root>/gravity_ee'

      force_error[i] = force_ee[i] - gravity_ee[i];

      // Gain: '<Root>/Gain'
      rtb_Gain = 0.001 * force_error[i];

      // Gain: '<S1>/Filter Coefficient' incorporates:
      //   DiscreteIntegrator: '<S1>/Filter'
      //   Gain: '<S1>/Derivative Gain'
      //   Sum: '<S1>/SumD'

      rtb_FilterCoefficient = (PID_D[i] * rtb_Gain -
        ForceController_DW.Filter_DSTATE[i]) * PID_N[i];

      // Outport: '<Root>/poseOffset_ee' incorporates:
      //   DiscreteIntegrator: '<S1>/Integrator'
      //   Gain: '<S1>/Proportional Gain'
      //   Sum: '<S1>/Sum'

      ForceController_Y.poseOffset_ee[i] = (PID_P[i] * rtb_Gain +
        ForceController_DW.Integrator_DSTATE[i]) + rtb_FilterCoefficient;

      // Update for DiscreteIntegrator: '<S1>/Integrator' incorporates:
      //   Gain: '<S1>/Integral Gain'

      ForceController_DW.Integrator_DSTATE[i] += PID_I[i] * rtb_Gain * 0.008;

      // Update for DiscreteIntegrator: '<S1>/Filter'
      ForceController_DW.Filter_DSTATE[i] += 0.008 * rtb_FilterCoefficient;
    }
  }

  rate_scheduler((&ForceController_M));

  // Copy value for root outport '<Root>/poseOffset_ee' since it is accessed globally 
  {
    int32_T i;
    for (i = 0; i < 6; i++)
      poseOffset_ee[i] = ForceController_Y.poseOffset_ee[i];
  }
}

// Model initialize function
void ForceControllerClass::initialize()
{
  // Registration code

  // initialize real-time model
  (void) memset((void *)(&ForceController_M), 0,
                sizeof(RT_MODEL_ForceController_T));

  // block I/O

  // exported global signals
  {
    int32_T i;
    for (i = 0; i < 6; i++) {
      force_error[i] = 0.0;
    }
  }

  // states (dwork)
  (void) memset((void *)&ForceController_DW, 0,
                sizeof(DW_ForceController_T));
}

// Model terminate function
void ForceControllerClass::terminate()
{
  // (no terminate code required)
}

// Constructor
ForceControllerClass::ForceControllerClass()
{
}

// Destructor
ForceControllerClass::~ForceControllerClass()
{
  // Currently there is no destructor body generated.
}

// Real-Time Model get method
RT_MODEL_ForceController_T * ForceControllerClass::getRTM()
{
  return (&ForceController_M);
}

//
// File trailer for generated code.
//
// [EOF]
//
