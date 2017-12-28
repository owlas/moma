//////////////////////////////////
// CONVERGENCE TESTS            //
//                              //
// TASK 3                       //
// Single trajectory (unstable) //
//////////////////////////////////

#include "../../include/integrators.hpp"
#include "../../include/rng.hpp"
#include "../../include/io.hpp"
#include <functional>
#include <iostream>
#include <stdio.h>
#include <cmath>

void task3() {

    //                                  aout    bout    j_a     j_b       x_in           t_a           t_b
    using sde_jac = std::function<void(double*,double*,double*,double*,const double*,const double,const double)>;
    //                              drift   diff       state           t
    using sde = std::function<void(double*,double*,const double*,const double)>;

    ////////////////////////////////////////////////////
    // STOCHASTIC DIFFERENTIAL EQUATION               //
    // dX(t) = aX(t) dt + bX(t) dW(t)  [stratonovich] //
    ////////////////////////////////////////////////////

    double a=1.0, b=0.1; // unstable
    size_t n_dim=1, w_dim=1;


    // SDE FUNCTION - drift and diffusion
    sde test_sde = [a,b](double *drift, double *diff, const double*in, const double)
        {
            drift[0] = a * in[0];
            diff[0] = b * in[0];
        };


    // SDE AND JACOBIANS - needed for implicit solver
    sde_jac test_sde_jac = [a,b](double *aout, double *bout, double *ja,
                                 double *jb, const double *xin,
                                 const double, const double)
        {
            aout[0] = a * xin[0];
            bout[0] = b * xin[0];
            ja[0] = a;
            jb[0] = b;
        };


    // INITIAL CONDITION
    double x0[1] = { 1.0 };
    double t0 = 0.0;


    // SDE SOLUTION - analytic solution
    auto solution = [a, b, x0]( double t, double Wt )
        { return x0[0] * std::exp( a * t + b * Wt ); };


    // STEP SIZE
    size_t n_steps=100;
    double dt = 1e-7;


    // RANDOM NUMBER GENERATOR
    const long seed = 1001;
    RngMtNorm rng( seed, std::sqrt(dt) );
    double *dw;


    // IMPLICIT SCHEME PARAMS
    double eps = 1e-9;
    size_t max_iter = 1000;


    // ALLOCATE MEM FOR RESULTS
    double *x = new double[n_steps+1]; // +1 for the initial condition
    char fname[100];


    // SIMULATE SOLUTION
    std::cout << std::endl;
    std::cout << "Executing task 3" << std::endl;
    std::cout << "Simulating with time step: " << dt << std::endl;
    std::cout << "For number of steps: " << n_steps << std::endl;

    dw = new double[n_steps];
    rng = RngMtNorm( seed, 1.0 );
    for( size_t n=0; n<n_steps; n++ )
        dw[n] = rng.get();


    // IMPLICIT MIDPOINT
    driver::implicit_midpoint( x, x0, dw, test_sde_jac, n_dim, w_dim,
                               n_steps, t0, dt, eps, max_iter );
    io::write_array( "output/task3/implicit", x, n_steps + 1 );


    // HEUN SCHEME
    driver::heun( x, x0, dw, test_sde, n_steps,
                  n_dim, w_dim, dt );
    io::write_array( "output/task3/heun", x, n_steps + 1);
    delete[] dw;

    // ANALYTIC SOLUTION
    double Wt=0;
    rng = RngMtNorm( seed, std::sqrt( dt ) );
    x[0]=x0[0];
    for( unsigned int n=1; n<n_steps+1; n++ )
    {
        Wt += rng.get();
        x[n] = solution( dt*n, Wt );
    }
    io::write_array( "output/task3/true", x, n_steps + 1 );

    delete[] x;
}
