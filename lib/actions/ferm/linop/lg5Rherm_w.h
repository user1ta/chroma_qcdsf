// -*- C++ -*-
// $Id: lg5Rherm_w.h,v 1.1 2005-07-15 11:06:11 bjoo Exp $

#ifndef __lg5Rherm_h__
#define __lg5Rherm_h__

#include "handle.h"
#include "linearop.h"


namespace Chroma 
{ 
//! Gamma(5) R hermitian linear operator
/*!
 * \ingroup linop
 *
 * This routine is specific to DWF fermions!
 *
 * This operator evaluates g5 R D
 * where D the input DWF operator
 * and R is the reflection matrix R_{s,s'}=\delta_{s, Ls-1-s'}
 *
 * = ( 0 ...... 1 )
 *   ( 0 .....1 0 )
 *   ( 0 ...1 0 0 )
 *   ( 0 ..     0 )
 *   ( 0 1 ...  0 )
 *   ( 1 0 0 ...0 )
 */

template<typename T>
class lg5RHerm {};              // Empty class -- I want instantiations 
                                // of this to break the compiler
 
template<typename T>
class lg5RHerm< multi1d<T> > : public LinearOperator< multi1d<T> >
{
public:
  //! Initialize pointer with existing pointer
  /*! Requires that the pointer p is a return value of new */
  lg5RHerm(const LinearOperator< multi1d<T> >* p) : D(p) {}

  //! Copy pointer (one more owner)
  lg5RHerm(Handle<const LinearOperator< multi1d<T> > > p): D(p) {}

  //! Destructor
  ~lg5RHerm() {}

  //! Subset comes from underlying operator
  inline const OrderedSubset& subset() const {return D->subset();}

  //! The size of the 5D operator
  inline int size() const { return D->size(); }

  //! Apply the operator onto a source vector
  /*! For this operator, the sign is ignored */
  inline void operator() (multi1d<T>& chi, const multi1d<T>& psi, enum PlusMinus isign) const
  {
    multi1d<T>  tmp(D->size());  moveToFastMemoryHint(tmp);
    const OrderedSubset& sub = (*D).subset();
    int N5 = (*D).size();
    // Operator is hermitian so can ignore isign

    // tmp = D psi
    (*D)(tmp, psi, PLUS);

    // tmp = R tmp
    for(int s=0; s < N5; s++) { 
      tmp[s][sub] = tmp[N5-1-s];
    }

    // chi = G_5 tmp
    for(int s=0; s < N5; s++) { 
      chi[s][sub] = GammaConst<Ns,Ns*Ns-1>()*tmp[s];
    }
    
    // tmp disappears here...
  }

private:
  const Handle< const LinearOperator< multi1d<T> > > D;
};



}; // End Namespace Chroma


#endif
