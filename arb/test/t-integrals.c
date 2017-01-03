#include "abel_jacobi.h"

/* check with gp */
/*
 x = [5, 8 , -9, 8, 1 , 7, -4 , 3 , -5, 0, 0 , -9, 4 , 4, 4, 7 , 10, 4, -3, -9];
 y = [9, -5, 1 , 0, -4, 8, -10, -8, 0 , 6, -8, -9, -6, 0, 5, -7, 4 , 7, 4 , 5];
 u = vector(#x,k,(x[k]+I*y[k])/sqrt(2));
 { ref = concat(vector(8,d,vector((d+1)\2,i,
     intnum(t=[-1,-1/2],[1,-1/2],t^(i-1)/(sqrt(1-t^2)*prod(k=1,d,sqrt(t-u[k]))))
     ))) }
 apply(z->printf("\"%f\",\n",real(z)),ref)
 apply(z->printf("\"%f\",\n",imag(z)),ref)
 */
#define dmax 20
#define nref 20
int x[dmax] = {5, 8 , -9, 8, 1 , 7, -4 , 3 , -5, 0, 0 , -9, 4 , 4, 4, 7 , 10, 4, -3, -9};
int y[dmax] = {9, -5, 1 , 0, -4, 8, -10, -8, 0 , 6, -8, -9, -6, 0, 5, -7, 4 , 7, 4 , 5};

const char * ref_r[nref] = {
    "0.592339228028183807171534310995523858113327659308939964892",
    "0.437420121778404811616784013987239655951163497307180422045",
    "0.175231908231066551777805786969019725145393923334108545631",
    "0.00112380081724733126156439114362059190087029292674953208446",
    "-0.0148413715211417475574573418710946293451063884857438117265",
    "-0.00185936463862594746837689254295532684684401766499124498137",
    "-0.0386589420469707561445913065600072747987030458956008725564",
    "-0.00146350720863848941329380324473485967851978770256555126879",
    "-0.0191974109186227319526395925075778867560135729024455387117",
    "0.000849877142108395011506993778960167172675025231149818384234",
    "0.000744520227802420622573174599316886287075308119258810234174",
    "0.000477386150251483346388571581986706626474613444323541758680",
    "-0.00296103015383763087701825733040738975347234002609509955372",
    "0.000109764336961937303048123069696718180739036332313204024045",
    "-0.00145694294607117663733987736881944712127014874883057748201",
    "0.0000824770081936928634922922289707916743248110098663930504262",
    "-0.00232139594928423603610628021816974319927736153926231705930",
    "-0.000206413505214404505444988944572223976160919032049013595837",
    "-0.00115214572895628268979845265969423951051291311587870122486",
    "-0.000153762870341653204706503062659451283843109742010069402740"
};

const char * ref_i[nref] = {
    "0.999867701926114250164226874546329505028599718651444395662",
    "-0.113067678269683819622201153546299168505476071079623537598",
    "-0.0348370488950480271551085857028179172171431387886861582553",
    "-0.00283602601409151217530409047783814570233277521212177347002",
    "-0.0741582327331738378264035865455320735113097112403289483787",
    "-0.00378650249533001777719121683974835593189948760128367989690",
    "-0.0202783101922206406766534217057000345950366173297359858386",
    "-0.00405791663967395646375641603767976124908454118962734781580",
    "-0.0102811784368502960244293548074588652159343746560986727976",
    "-0.0159970804964055413282773715527637548959648319590776649809",
    "-0.00147224396576860883469047091914236287376509100944748752628",
    "-0.00800095371410142738832455532337076833250091726726776813743",
    "-0.00494495659879824506372682698318810371682881453070545310986",
    "-0.000620998942318701475153172555382115223572297651904716490680",
    "-0.00247695643501778907021030609179624989075374542211055856551",
    "-0.000464033864857812799899428471903021643703781992012310368704",
    "-0.000181913228365818229962612516748414485400119272128768632491",
    "-0.000272661884209949348063238230435560371821319044595307737284",
    "-0.000106296662482134164442088320766896966355752123699304388536",
    "-0.000204067584319916096880806648301376368600530202218080800442"
};

int main() {

    slong prec;

    flint_rand_t state;

    flint_printf("integrals...");
    fflush(stdout);
    flint_randinit(state);

    for (prec = 60; prec < 150; prec *= 2)
    {
        /* start with m = 2 only */
        slong j, l, d; /*, m = 2;*/
        acb_ptr u, ref;
        arb_t s2;
        u = _acb_vec_init(dmax);
        ref = _acb_vec_init(dmax);

        arb_init(s2);
        arb_set_si(s2, 2);
        arb_sqrt(s2, s2, prec);

        for (j = 0; j < dmax; j++)
        {
            arb_set_si(acb_realref(u + j), x[j]);
            arb_set_si(acb_imagref(u + j), y[j]);
            acb_div_arb(u + j, u + j, s2, prec);
        }
        arb_clear(s2);

        for (j = 0; j < nref; j++)
        {
            if (arb_set_str(acb_realref(ref + j), ref_r[j], prec) ||
                    arb_set_str(acb_imagref(ref + j), ref_i[j], prec) )
            {
                flint_printf("error while setting ref[%ld] <- %s+I*%s\n",
                        j, ref_r[j], ref_i[j]);
                abort();
            }

        }

        for (l = 0, d = 1; d < 4; d ++)
        {
            slong g = (d + 1) / 2;
            slong n, j;
            acb_ptr res;

            res = _acb_vec_init(g);

            n = gc_params(u, d, -1, g - 1, prec);

            /* take d1 = d ie sqrt(x-u[i]) for all */
            gc_integrals(res, u, d, d, g, n, prec);

            for (j = 0; j < g && l < nref; j++, l++)
            {
                if (!acb_overlaps(res + j, ref + l))
                {
                    flint_printf("FAIL:\n\n");
                    flint_printf("d = %ld, i = %ld, prec = %ld\n", d, j, prec);
                    flint_printf("\nref = ");
                    acb_printd(ref + l, 20);
                    flint_printf("\ngc_integral = ");
                    acb_printd(res + j, 20);
                    flint_printf("\n\n");
                    abort();
                }
            }

            _acb_vec_clear(res, g);
        }
        _acb_vec_clear(u, dmax);
        _acb_vec_clear(ref, nref);
    }

    flint_randclear(state);
    flint_cleanup();
    printf("PASS\n");
    return 0;
}