/******************************************************************************

 Copyright (C) 2016 Pascal Molin

 ******************************************************************************/

#include "abel_jacobi.h"
#include "complex_extras.h"

#define LOG2 log(2)
#define PARAMS 0

double
distance_ellipse_d(double x, double y, double a)
{
    const double eps = 1.e-10;
    double b, t, ft, st, ct;
    b = sqrt(a*a-1);
    t = atan(a*y/(b*x));

    do
    {
        double fpt;
        st = sin(t);
        ct = cos(t);
        ft = ct*st - x*a*st + y*b*ct;
        fpt = ct*(ct-x*a) - st*(st + y*b);
        t -= ft / fpt;
    }
    while (fabs(ft) > eps);

    x -= a * ct;
    y -= b * st;

    return sqrt(x*x + y*y);
}

double
constant_m_d(cdouble * w, slong len, double r, slong d)
{
    /* 2*Pi*r^d/sqrt(prod d(w_i,e_r)) */
    slong k;
    double p;
    const double eps = 1.e-5;
    p = 1;
    for (k = 0; k < len; k++)
    {
        double d = distance_ellipse_d(creal(w[k]), cimag(w[k]), r);
        p *= (d < eps) ? r : d;
    }
    return 2 * PI * pow(r, d) / sqrt(p);
}

void
ab_points_worst(cdouble * w, tree_t tree, sec_t c)
{
    slong k, l, i, j;
    cdouble a, b;

    /* small approx of points */
    for (k = 0; k < c.d; k++)
        w[k] = acb_get_cdouble(c.roots + k);

    /* get worst edge */
    k = tree->min;

    i = tree->e[k].a;
    j = tree->e[k].b;

    a = w[i];
    b = w[j];

    for (k = 0, l = 0; k < c.d; k++)
    {
        if (k == i || k == j)
            continue;
        else
            w[l++] = (2*w[k]-a-b)/(b-a);
    }
}

slong
gc_int_params(const tree_t tree, sec_t c, slong prec)
{
    slong n;
    double r;
    double M, A, B, rho;
    double mult = .25;
    cdouble * w;

    r = tree->r;

    if (r <= 1)
    {
        flint_printf("gc int: r must be > 1 (r = %lf)\n", r);
        abort();
    }

    w = malloc(c.d * sizeof(cdouble));
    ab_points_worst(w, tree, c);

#if PARAMS
    {
        /* zero estimate */
        double r1 = (1 + r) / 2;
        M = constant_m_d(w, c.d - 2, r1, c.d);
        A = prec*LOG2 + log(2*PI*M) + 1;
        rho = r1 + sqrt(r1*r1 - 1);
        B = 2*log(rho);
        printf("r = %lf -> n0 = %lf [M = %lf, A = %lf, B = %lf]\n", r1, A/B, M, A, B);
    }
#endif

    /* first estimate */
    M = constant_m_d(w, c.d - 2, r, c.d);
    A = prec*LOG2 + log(2*PI*M) + 1;
    rho = r + sqrt(r*r-1);
    B = 2*log(rho);
#if PARAMS
    printf("r = %lf -> n1 = %lf [M = %lf, A = %lf, B = %lf]\n", r, A/B, M, A, B);
#endif
    n = ceil(A / B);

    /* second = should be exact */
    r *= 1-mult/n;
    M = constant_m_d(w, c.d - 2, r, c.d);
    A = prec*LOG2 + log(2*PI*M) + 1;
    rho = r + sqrt(r*r-1);
    B = 2*log(rho);
#if PARAMS
    printf("r = %lf -> n2 = %lf [M = %lf, A = %lf, B = %lf]\n", r, A/B, M, A, B);
#endif
    n = ceil(A / B);

    free(w);

    return n;
}

void
gc_distance_ellipse(mag_t d, const acb_t p, mag_t r)
{
    return;
}

void
gc_constant_M(mag_t M, acb_srcptr w, slong len, slong d, mag_t r)
{
    slong i;
    mag_t di, p;
    /* 2*Pi*r^d/sqrt(prod d(w_i,e_r)) */
    mag_const_pi(M);
    mag_mul_2exp_si(M, M, 1);
    mag_pow_ui(p, r, d);
    mag_mul(M, M, p);
    mag_one(p);
    for (i = 0; i < len; i++)
    {
        gc_distance_ellipse(di, w + i, r);
        mag_mul(p, p, di);
    }
    mag_rsqrt(p, p);
    mag_mul(M, M, p);
}

/* error for computation with n points */
void
gc_error(mag_t e, slong n, acb_srcptr w, slong d, mag_t r)
{
    return;
}