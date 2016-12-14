/******************************************************************************
 
 Copyright (C) 2016 Pascal Molin
 
 ******************************************************************************/

#include "arb.h"
#include "arb_mat.h"
#include "acb.h"
#include "acb_mat.h"

/******************************************************************************

  data structures

 ******************************************************************************/

typedef struct
{
    /* y^m = prod_{i=1}^d x - roots[i] */
    slong m;             /* degree in y */
    slong d;             /* degree in x */
    acb_ptr roots;       /* branch points */
    slong g;             /* genus, 2g = (m-1)(d-1) - gcd(N,d) + 1 */

}
superelliptic_curve;

typedef superelliptic_curve sec_t;

typedef struct
{
    /*
    double tau;
    double h;
    */
    ulong n;
    slong prec;
    arb_t factor;
    arb_ptr x;
    arb_ptr dx;
}
de_integration_struct;
typedef de_integration_struct de_int_t[1];

typedef struct
{
    double tau;
    slong a;
    slong b;
} edge_t;

typedef struct
{
    slong n;
    double tau;
    edge_t * e;
} tree_struct;
typedef tree_struct tree_t[1];

/* intersection between two edges of the tree */
typedef struct
{
    slong coeff; /* intersection 0, 1 or -1 */
    slong shift; /* number of sheets to move to */
    slong dir;   /* up or down */
} inter_t;

typedef inter_t ** inter_mat;

/* represents the loop_t(coeff * zeta^shift * tree[index]) */
typedef struct
{
    slong coeff;
    slong shift;
    slong index;
} gamma_t;

typedef struct
{
    slong n; /* length at most 2*g */
    gamma_t * l;
} loop_t;

/* homology basis */
typedef loop_t * homol_t;

/* integer matrix */
typedef slong ** si_mat_t;

/* differential form */
typedef struct
{
    slong x; /* power of x */
    slong y; /* power of y */
} dform_t;

/* cohomology basis */
typedef dform_t * cohom_t;

typedef struct
{
    /* curve */
    sec_t c;

    /* differentials */
    cohom_t dz;

    /* homology using tree */
    tree_t tree;         /* choice of best loops */
    inter_mat inter;     /* intersections in tree, size d-1 * d-1 */

    /* A,B symplectic basis as sums of shifted gamma loops */
    homol_t loop_a;
    homol_t loop_b;

    /* periods  */
    acb_mat_t omega0;
    acb_mat_t omega1;      /* on A,B basis */
    acb_mat_t tau;         /* tau = Omega1^{-1}Omega0 */

    /* for Abel-Jacobi map, choice of a base point */
    slong p0;            /* base point */
    arb_mat_t proj;

}
abel_jacobi_struct;

typedef abel_jacobi_struct abel_jacobi_t[1];

/******************************************************************************

  functions

 ******************************************************************************/

void abel_jacobi_init_roots(abel_jacobi_t aj, slong m, acb_srcptr x, slong d);
void abel_jacobi_init_poly(abel_jacobi_t aj, slong m, acb_srcptr f, slong len, slong prec);
void abel_jacobi_compute(abel_jacobi_t aj, slong prec);
void abel_jacobi_clear(abel_jacobi_t aj);

void de_int_params(double * h, ulong *n, double tau, double M1, double M2, slong prec);
void de_int_init(de_int_t de, double h, ulong n, slong prec);
void de_int_clear(de_int_t de);

/* compute maximum spanning tree */
void tree_init(tree_t tree, slong d);
void tree_clear(tree_t tree);
void spanning_tree(tree_t tree, acb_srcptr x, slong d);

/* compute local intersections between tree edges */
/* -> (d-1)*(d-1) intersection matrix */
void intersection_tree(inter_mat inter, tree_t tree, acb_srcptr x, slong d);

/* find g+g symplectic homology basis from tree */
/* two lists of g loops */
void symplectic_reduction(si_mat_t p, si_mat_t m, slong g, slong len);
void symplectic_basis(homol_t loop_a, homol_t loop_b, inter_mat inter, sec_t c);

/* find basis of holomorphic differentials */
/* g elementary differentials */
void holomorphic_differentials(cohom_t dz, slong d, slong m);

/* numerically compute d-1 integrals along tree edges */
/* (d-1)*(g-1) matrix, tree edges on lines */
void integrals_tree(acb_mat_t integrals, sec_t c, const tree_t tree, const cohom_t dz, slong prec);

/* get all periods on a, b basis */
/* two g*g matrices */
void period_matrix(acb_mat_t omega, const homol_t basis, const acb_mat_t integrals, slong g, slong d, slong prec);

/* get tau reduced matrix */
void tau_matrix(acb_mat_t tau, const acb_mat_t omega0, const acb_mat_t omega1, slong prec);

/* core functions */
void nth_root_pol_def(acb_t y, acb_srcptr u, const arb_t x, slong d, slong m, slong prec);
void nth_root_pol_prod(acb_t y, acb_srcptr u, const arb_t x, slong d, slong m, slong prec);
