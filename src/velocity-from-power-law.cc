//
// Copyright (C) 2012 LAAS-CNRS
//
// Author: Florent Lamiraux, 
//         Mehdi Benallegue <mehdi@benallegue.com>
//

#include "velocity-from-power-law.hh"
#include <dynamic-graph/command-bind.h>

namespace dynamicgraph {
  namespace sot {
    using command::makeDirectSetter;
    using command::docDirectSetter;
    using command::docDirectGetter;
    using command::makeDirectGetter;
    namespace tools {
      VelocityFromPowerLaw::VelocityFromPowerLaw (const std::string name,
                                                  double radiusx,
                                                  double radiusy,
                                                  double gamma,
                                                  double beta,
                                                  double zerokapparadius):
        Entity(name),
        vectorSoutSOUT_( boost::bind(&VelocityFromPowerLaw::computeVelocitySignal,this,_1,_2),
                         sotNOSIGNAL,"VelocityFromPowerLaw("+name+")::output(vector)::velocity" ),
        LeftFootCurrentPosSIN(NULL,"VelocityFromPowerLaw("+name+")::input(homogeneousmatrix)::leftfootcurrentpos"),
        RightFootCurrentPosSIN(NULL,"VelocityFromPowerLaw("+name+")::input(homogeneousmatrix)::rightfootcurrentpos"),
        WaistCurrentPosSIN(NULL,"VelocityFromPowerLaw("+name+")::input(vector)::waist"),
        hp_(radiusx,radiusy,gamma,beta,zerokapparadius),
        powerLawGeneration_(this,hp_)
      {
        vectorSoutSOUT_.addDependency(LeftFootCurrentPosSIN );
        vectorSoutSOUT_.addDependency(RightFootCurrentPosSIN);
        vectorSoutSOUT_.addDependency(WaistCurrentPosSIN    );
        signalRegistration(vectorSoutSOUT_);

        signalRegistration(LeftFootCurrentPosSIN <<
                           RightFootCurrentPosSIN <<
                           WaistCurrentPosSIN);

        initializeCommand();
      }

      VelocityFromPowerLaw::VelocityFromPowerLaw (const std::string name):
        Entity(name),
        vectorSoutSOUT_( boost::bind(&VelocityFromPowerLaw::computeVelocitySignal,this,_1,_2),
                         sotNOSIGNAL,"VelocityFromPowerLaw("+name+")::output(vector)::velocity" ),
        LeftFootCurrentPosSIN(NULL,"VelocityFromPowerLaw("+name+")::input(homogeneousmatrix)::leftfootcurrentpos"),
        RightFootCurrentPosSIN(NULL,"VelocityFromPowerLaw("+name+")::input(homogeneousmatrix)::rightfootcurrentpos"),
        WaistCurrentPosSIN(NULL,"VelocityFromPowerLaw("+name+")::input(vector)::waist"),
        hp_(0.0, 0.0, 0.0, 0.0, 0.0),
        // HopfParameters(radiusx, radiusy, gamma, beta, zerokapparadius)
        powerLawGeneration_(this,hp_)
      {
        vectorSoutSOUT_.addDependency(LeftFootCurrentPosSIN );
        vectorSoutSOUT_.addDependency(RightFootCurrentPosSIN);
        vectorSoutSOUT_.addDependency(WaistCurrentPosSIN    );
        signalRegistration(vectorSoutSOUT_);

        signalRegistration(LeftFootCurrentPosSIN <<
                           RightFootCurrentPosSIN <<
                           WaistCurrentPosSIN);

        initializeCommand();
      }

      void VelocityFromPowerLaw::initializeCommand()
      {
        std::string docstring = "    \n"
        "    Set ellipse parameters\n"
        "      Input:\n"
        "        - a floating point number: the X half axe,\n"
        "        - a floating point number: the Y half axe,\n"
        "        - a floating point number: the gamma of the power law,\n"
        "        - a floating point number: the beta of the power law,\n"
        "    \n";
        addCommand("initializePowerLaw",
           dynamicgraph::command::makeCommandVoid4(
                     *this,&VelocityFromPowerLaw::initializeEllipse,docstring)
                   );
      }

      void VelocityFromPowerLaw::initializeEllipse (const double& radiusx,
                                                    const double& radiusy,
                                                    const double& gamma,
                                                    const double& beta)
      {
        hp_.~HopfParameters();
        hp_ = HopfParameters(radiusx,
                           radiusy,
                           gamma,
                           beta,
                           2.0);
        hp_.save();
        powerLawGeneration_.~PowerLawGeneration();
        powerLawGeneration_ = PowerLawGeneration(this,hp_);

        return ;
      }

      dynamicgraph::Vector& VelocityFromPowerLaw::computeVelocitySignal (
          dynamicgraph::Vector& vsout, const int& time)
      {
        vsout.resize(3,true) ;
        MatrixHomogeneous rf ;
        MatrixHomogeneous lf ;
        dynamicgraph::Vector waist;
        Eigen::Vector3d velocity ;
        try{
          rf=RightFootCurrentPosSIN(time);
          lf=LeftFootCurrentPosSIN(time);
          waist=WaistCurrentPosSIN(time);
//          std::cout << "lfx = " << lf(0,3) << std::endl;
//          std::cout << "lfy = " << lf(1,3) << std::endl;
//          std::cout << "rfx = " << rf(0,3) << std::endl;
//          std::cout << "rfy = " << rf(1,3) << std::endl;
//          std::cout << "ctheta = " << waist(5) << std::endl;
          velocity = powerLawGeneration_.generateVelocityFromPowerLawVectorField(
                time*0.005,waist(5),
                lf(0,3),lf(1,3),
                rf(0,3),rf(1,3));
        }catch(...)
        {
          velocity << 0.0001 , 0.0 , 0.0 ;
        }

        for (unsigned i=0 ; i<3 ; ++i )
        {
          if(std::abs(velocity(i))<0.0001)
            vsout(i)=0.0001;
          else
            vsout(i)=velocity(i);
        }

        return vsout;
      }

      DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(VelocityFromPowerLaw, "VelocityFromPowerLaw");

    } // namespace tools
  } // namespace sot
} // namespace dynamicgraph