
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

struct Domain {
  public:
    int* nodelist(int idx) { return &m_nodelist[8*idx]; }
    int& fx(int idx)       { return m_fx[idx]; }
    int& fy(int idx)       { return m_fy[idx]; }
    int& fz(int idx)       { return m_fz[idx]; }
  private:
    std::vector<int> m_nodelist;     /* elemToNode connectivity */
    std::vector<int> m_fx;  /* forces */
    std::vector<int> m_fy;
    std::vector<int> m_fz;
} domain;

void IntegrateStressForElems(int numElem,double *sigxx,double *sigyy,double *sigzz,double *determ)
{
  int lnode_nom_5;
  int lnode_nom_4;
  int k_nom_3;
// shape function derivatives
  double B[3UL][8UL];
  double x_local[8UL];
  double y_local[8UL];
  double z_local[8UL];
  double fx_local[8UL];
  double fy_local[8UL];
  double fz_local[8UL];
  // loop over all elements
  for( int k=0 ; k<numElem ; ++k )
  {
    int* elemNodes = domain.nodelist(k);

    // get nodal coordinates from global arrays and copy into local arrays.
    for( int lnode=0 ; lnode<8 ; ++lnode )
    {
      int gnode = elemNodes[lnode];
      x_local[lnode] = domain.fx(gnode);
      y_local[lnode] = domain.fy(gnode);
      z_local[lnode] = domain.fz(gnode);
    }

    // copy nodal force contributions to global force arrray.
    for( int lnode=0 ; lnode<8 ; ++lnode )
    {
      int gnode = elemNodes[lnode];
      domain.fx(gnode) += fx_local[lnode];
      domain.fy(gnode) += fy_local[lnode];
      domain.fz(gnode) += fz_local[lnode];
    }
  }
}
