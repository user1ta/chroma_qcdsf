// -*- C++ -*-
/* \file
 * \brief Stout params
 */

#ifndef _stout_fermstate_params_h_
#define _stout_fermstate_params_h_

#include "chromabase.h"

namespace Chroma
{
  //! Params for stout-links
  /*! \ingroup fermstates */
  struct StoutFermStateParams
  {
    //! Default constructor
    StoutFermStateParams();
    StoutFermStateParams(XMLReader& in, const std::string& path);

    multi2d<Real>  rho;
    multi1d<bool>  smear_in_this_dirP;

    int            n_smear;
    Real           sm_fact;
  };

  void read(XMLReader& xml, const std::string& path, StoutFermStateParams& p);
  void write(XMLWriter& xml, const std::string& path, const StoutFermStateParams& p);

  //! Params for stout-links
  /*! \ingroup fermstates momentum */
  struct MomentumFermStateParams
  {
    //! Default constructor
    MomentumFermStateParams();
    MomentumFermStateParams(XMLReader& in, const std::string& path);

    multi2d<Real>  rho;
    multi1d<bool>  smear_in_this_dirP;

    int            n_smear;
    Real           sm_fact;
    multi1d<float>   momk;
  };

  void read(XMLReader& xml, const std::string& path, MomentumFermStateParams& p);
  void write(XMLWriter& xml, const std::string& path, const MomentumFermStateParams& p);
}

#endif
