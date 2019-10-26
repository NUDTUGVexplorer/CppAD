/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-19 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */
/*
$begin graph_sum_op.cpp$$
$spell
    sum
$$

$section C++ AD Graph sum Operator: Example and Test$$

$head Source Code$$
$srcfile%test_more/graph/sum_op.cpp%0%// BEGIN C++%// END C++%1%$$

$end
*/
// BEGIN C++
# include <cppad/cppad.hpp>

bool sum_op(void)
{   bool ok = true;
    using CppAD::vector;
    using CppAD::AD;
    using std::string;
    typedef CppAD::local::graph::graph_op_struct graph_op_struct;
    //
    // AD graph example
    // node_1 : p[0]
    // node_2 : p[1]
    // node_3 : p[2]
    // node_4 : x[0]
    // node_5 : p[0] + p[1] + p[2]
    // node_6 : x[0] + p[0] + p[1] + p[2]
    // y[0]   = x[0] + p[0] + p[1] + p[2]
    //
    // initialize vectors as empty
    vector<string>          atomic_name_vec;
    vector<double>          constant_vec;
    vector<graph_op_struct> operator_vec;
    vector<size_t>          operator_arg;
    vector<size_t>          dependent_vec;
    //
    // structure corresponding to one operator
    graph_op_struct         op_usage;
    //
    // size_t value that is not used
    size_t not_used = std::numeric_limits<size_t>::max();
    //
    // set scalars
    string function_name = "sum_op example";
    size_t n_dynamic_ind = 3;
    size_t n_independent = 1;
    //
    // node_5 : p[0] + p[1] + p[2]
    op_usage.op_enum          = CppAD::local::graph::sum_graph_op;
    op_usage.n_result         = 1;
    op_usage.n_arg            = 3;
    op_usage.start_arg        = operator_arg.size();
    op_usage.extra            = not_used;
    operator_vec.push_back(op_usage);
    operator_arg.push_back(1);
    operator_arg.push_back(2);
    operator_arg.push_back(3);
    //
    // node_6 : x[0] + p[0] + p[1] + p[2]
    op_usage.n_arg            = 2;
    op_usage.start_arg        = operator_arg.size();
    operator_vec.push_back(op_usage);
    operator_arg.push_back(4);
    operator_arg.push_back(5);
    //
    // y[0]   = x[0] + p[0] + p[1] + p[2]
    dependent_vec.push_back(6);
    //
    // f(x, p) = x_0 + p_0 + p_1 + p_2
    CppAD::ADFun<double> f;
    f.from_graph(
        function_name,
        atomic_name_vec,
        n_dynamic_ind,
        n_independent,
        constant_vec,
        operator_vec,
        operator_arg,
        dependent_vec
    );
    //
    ok &= f.Domain() == 1;
    ok &= f.Range() == 1;
    ok &= f.size_dyn_ind() == 3;
    //
    // set independent variables and parameters
    vector<double> p(3), x(1);
    for(size_t j = 0; j < 3; ++j)
        p[j] = double(j + 1);
    x[0] = 5.0;
    //
    // compute y = f(x, p)
    f.new_dynamic(p);
    vector<double> y = f.Forward(0, x);
    //
    // check result
    ok &= y[0] == x[0] + p[0] + p[1] + p[2];
    // -----------------------------------------------------------------------
    // Convert to Graph graph and back again
    f.to_graph(
        function_name,
        atomic_name_vec,
        n_dynamic_ind,
        n_independent,
        constant_vec,
        operator_vec,
        operator_arg,
        dependent_vec
    );
    // std::cout << "json = " << json;
    f.from_graph(
        function_name,
        atomic_name_vec,
        n_dynamic_ind,
        n_independent,
        constant_vec,
        operator_vec,
        operator_arg,
        dependent_vec
    );
    // -----------------------------------------------------------------------
    ok &= f.Domain() == 1;
    ok &= f.Range() == 1;
    ok &= f.size_dyn_ind() == 3;
    //
    // set independent variables and parameters
    for(size_t j = 0; j < 3; ++j)
        p[j] = double(j + 1);
    x[0] = 5.0;
    //
    // compute y = f(x, p)
    f.new_dynamic(p);
    y = f.Forward(0, x);
    //
    // check result
    ok &= y[0] == x[0] + p[0] + p[1] + p[2];
    //
    return ok;
}
// END C++