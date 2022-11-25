#ifndef DOUBLE_COMPARE_HPP_INCLUDED
#define DOUBLE_COMPARE_HPP_INCLUDED


//--------------------------------------------------


#define EPSILON 0.001


//--------------------------------------------------


bool  double_equal  (double first, double second);


//--------------------------------------------------


bool  double_equal  (double first, double second) {

    assert ( (isnan(first) || isnan(second)) == false);

    if (fabs (first - second) < EPSILON) { return true ; }
    else                                 { return false; }
}


//--------------------------------------------------


#endif