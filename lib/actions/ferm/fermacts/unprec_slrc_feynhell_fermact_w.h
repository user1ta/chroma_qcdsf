// -*- C++ -*-
// $Id: unprec_slrc_feynhell_fermact_w.h,v 3.2 2006-10-19 16:01:29 bglaessle Exp $
/*! \file
 *  \brief Unpreconditioned SLRC fermion action
 */

#ifndef __unprec_slrc_feynhell_fermact_w_h__
#define __unprec_slrc_feynhell_fermact_w_h__

#include "unprec_wilstype_fermact_w.h"
#include "actions/ferm/linop/lgherm_w.h"
#include "actions/ferm/fermacts/clover_fermact_params_w.h"
#include "actions/ferm/fermacts/slrc_feynhell_fermact_params_w.h"

namespace Chroma
{
  //! Name and registration
  /*! \ingroup fermacts */
  namespace UnprecSLRCFeynHellFermActEnv
  {
    extern const std::string name;
    bool registerAll();
  }

  //! Unpreconditioned Clover fermion action
  /*! \ingroup fermacts
   *
   * Unpreconditioned clover fermion action
   */
  class UnprecSLRCFeynHellFermAct : public UnprecWilsonTypeFermAct<LatticeFermion, 
			      multi1d<LatticeColorMatrix>, multi1d<LatticeColorMatrix> >
  {
  public:
    // Typedefs to save typing
    typedef LatticeFermion               T;
    typedef multi1d<LatticeColorMatrix>  P;
    typedef multi1d<LatticeColorMatrix>  Q;

    //! General FermBC
    /*! Isotropic action */
    UnprecSLRCFeynHellFermAct(Handle< CreateFermState<T,P,Q> > cfs_,
			      const CloverFermActParams& param_,
			      const SLRCFeynHellFermActParams& fhparam_) : 
      cfs(cfs_), param(param_), fhparam(fhparam_) {}

    //! Copy constructor
    UnprecSLRCFeynHellFermAct(const UnprecSLRCFeynHellFermAct& a) : 
      cfs(a.cfs), param(a.param), fhparam(a.fhparam) {}

    //! Produce a linear operator for this action
    UnprecLinearOperator<T,P,Q>* linOp(Handle< FermState<T,P,Q> > state) const;

    LinearOperator<T>* hermitianLinOp(Handle< FermState<T,P,Q> > state) const 
      { 
	return new lgherm<T>(linOp(state));
      }

    //! Destructor is automatic
    ~UnprecSLRCFeynHellFermAct() {}

  protected:
    //! Return the fermion BC object for this action
    const CreateFermState<T,P,Q>& getCreateState() const {return *cfs;}

    // Hide partial constructor
    UnprecSLRCFeynHellFermAct() {}

    //! Assignment
    void operator=(const UnprecSLRCFeynHellFermAct& a) {}

  private:
    Handle< CreateFermState<T,P,Q> > cfs;
    CloverFermActParams param;
    SLRCFeynHellFermActParams fhparam;
  };

}

#endif
