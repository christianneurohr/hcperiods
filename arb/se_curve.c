/******************************************************************************

 Copyright (C) 2016 Pascal Molin

 ******************************************************************************/

#include "abel_jacobi.h"

void
abel_jacobi_init_roots(abel_jacobi_t aj, slong m, acb_srcptr x, slong d)
{
    slong k, g;

    g = ((m-1)*(d-1) - n_gcd(m,d) + 1)/ 2;
    aj->c.m = m;
    aj->c.d = d;
    aj->c.g = g;
    aj->c.roots = _acb_vec_init(d);
    for (k = 0; k < d; k++)
        acb_set(aj->c.roots + k, x + k);

    tree_init(aj->tree, d);
    aj->dz = malloc(g * sizeof(dform_t));

    aj->inter = malloc( (d-1) * sizeof (inter_t *) );
    for (k = 0; k < d-1; k++)
        aj->inter[k] = malloc( (d-1) * sizeof(inter_t));

    aj->loop_a = malloc(g * sizeof(loop_t));
    aj->loop_b = malloc(g * sizeof(loop_t));
    /*
    for (k = 0; k < g; k++)
    {
        aj->loop_a[k] = malloc( (d-1) * sizeof(int_tree));
        aj->loop_b[k] = malloc( (d-1) * sizeof(int_tree));
    }
    */

    acb_mat_init(aj->omega0, g, g);
    acb_mat_init(aj->omega1, g, g);
    acb_mat_init(aj->tau, g, g);

    arb_mat_init(aj->proj, 2*g, 2*g);
}

void
abel_jacobi_init_poly(abel_jacobi_t aj, slong n, acb_srcptr f, slong len, slong prec)
{
    slong d = len - 1;
    acb_ptr x;
    x = _acb_vec_init(d);
    /* isolate roots */
    if (_acb_poly_find_roots(x, f, NULL, len, 0, prec) < d)
    {
        flint_printf("missing roots, abort.\n");
        abort();
    }
    abel_jacobi_init_roots(aj, n, x, d);
    _acb_vec_clear(x, d);
}

void
abel_jacobi_clear(abel_jacobi_t aj)
{
    _acb_vec_clear(aj->c.roots, aj->c.d);
    acb_mat_clear(aj->omega0);
    acb_mat_clear(aj->omega1);
    acb_mat_clear(aj->tau);
    arb_mat_clear(aj->proj);
    tree_clear(aj->tree);
    free(aj->loop_a);
    free(aj->loop_b);
    free(aj->inter);
    free(aj->dz);
}

void
abel_jacobi_compute(abel_jacobi_t aj, slong prec)
{
    sec_t c = aj->c;
    acb_mat_t integrals;

    /* homology */
    spanning_tree(aj->tree, c.roots, c.d);
    intersection_tree(aj->inter, aj->tree, c.roots, c.d);
    symplectic_basis(aj->loop_a, aj->loop_b, aj->inter, c);

    /* cohomology */
    holomorphic_differentials(aj->dz, c.d, c.m);

    /* integration */
    acb_mat_init(integrals, c.d-1, c.g);
    integrals_tree(integrals, c, aj->tree, aj->dz, prec);

    /* period matrices */
    period_matrix(aj->omega0, aj->loop_a, integrals, c.g, c.d, prec);
    period_matrix(aj->omega1, aj->loop_b, integrals, c.g, c.d, prec);

    acb_mat_clear(integrals);

    tau_matrix(aj->tau, aj->omega0, aj->omega1, prec);

    /* abel-jacobi map */
    aj->p0 = 0;
}