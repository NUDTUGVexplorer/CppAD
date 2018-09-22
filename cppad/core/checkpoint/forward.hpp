# ifndef CPPAD_CORE_CHECKPOINT_FORWARD_HPP
# define CPPAD_CORE_CHECKPOINT_FORWARD_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

namespace CppAD { // BEGIN_CPPAD_NAMESPACE

# if ! CPPAD_MULTI_THREAD_TMB
template <class Base>
bool checkpoint<Base>::forward(
	size_t                    p ,
	size_t                    q ,
	const vector<bool>&      vx ,
	      vector<bool>&      vy ,
	const vector<Base>&      tx ,
	      vector<Base>&      ty )
{	// make sure member_ is allocated for this thread
	size_t thread = thread_alloc::thread_num();
	allocate_member(thread);
	//
	size_t n = member_[thread]->f_.Domain();
	size_t m = member_[thread]->f_.Range();
	//
	CPPAD_ASSERT_UNKNOWN( member_[thread]->f_.size_var() > 0 );
	CPPAD_ASSERT_UNKNOWN( tx.size() % (q+1) == 0 );
	CPPAD_ASSERT_UNKNOWN( ty.size() % (q+1) == 0 );
	CPPAD_ASSERT_UNKNOWN( n == tx.size() / (q+1) );
	CPPAD_ASSERT_UNKNOWN( m == ty.size() / (q+1) );
	bool ok  = true;
	//
	if( vx.size() == 0 )
	{	// during user forward mode
		if( member_[thread]->jac_sparse_set_.n_set() != 0 )
			member_[thread]->jac_sparse_set_.resize(0,0);
		if( member_[thread]->jac_sparse_bool_.size() != 0 )
			member_[thread]->jac_sparse_bool_.clear();
		//
		if( member_[thread]->hes_sparse_set_.n_set() != 0 )
			member_[thread]->hes_sparse_set_.resize(0,0);
		if( member_[thread]->hes_sparse_bool_.size() != 0 )
			member_[thread]->hes_sparse_bool_.clear();
	}
	if( vx.size() > 0 )
	{	// need Jacobian sparsity pattern to determine variable relation
		// during user recording using checkpoint functions
		if( sparsity() == atomic_base<Base>::set_sparsity_enum )
		{	if( member_[thread]->jac_sparse_set_.n_set() == 0 )
				set_jac_sparse_set();
			CPPAD_ASSERT_UNKNOWN( member_[thread]->jac_sparse_set_.n_set() == m );
			CPPAD_ASSERT_UNKNOWN( member_[thread]->jac_sparse_set_.end()   == n );
			//
			for(size_t i = 0; i < m; i++)
			{	vy[i] = false;
				local::sparse_list::const_iterator set_itr(
					member_[thread]->jac_sparse_set_, i
				);
				size_t j = *set_itr;
				while(j < n )
				{	// y[i] depends on the value of x[j]
					// cast avoid Microsoft warning (should not be needed)
					vy[i] |= static_cast<bool>( vx[j] );
					j = *(++set_itr);
				}
			}
		}
		else
		{	if( member_[thread]->jac_sparse_set_.n_set() != 0 )
				member_[thread]->jac_sparse_set_.resize(0, 0);
			if( member_[thread]->jac_sparse_bool_.size() == 0 )
				set_jac_sparse_bool();
			CPPAD_ASSERT_UNKNOWN( member_[thread]->jac_sparse_set_.n_set() == 0 );
			CPPAD_ASSERT_UNKNOWN( member_[thread]->jac_sparse_bool_.size() == m * n );
			//
			for(size_t i = 0; i < m; i++)
			{	vy[i] = false;
				for(size_t j = 0; j < n; j++)
				{	if( member_[thread]->jac_sparse_bool_[ i * n + j ] )
					{	// y[i] depends on the value of x[j]
						// cast avoid Microsoft warning
						vy[i] |= static_cast<bool>( vx[j] );
					}
				}
			}
		}
	}
	// compute forward results for orders zero through q
	ty = member_[thread]->f_.Forward(q, tx);

	// no longer need the Taylor coefficients in f_
	// (have to reconstruct them every time)
	// Hold onto sparsity pattern because it is always good.
	size_t c = 0;
	size_t r = 0;
	member_[thread]->f_.capacity_order(c, r);
	return ok;
}
# else // CPPAD_MULTI_THREAD_TMB
# define THREAD omp_get_thread_num()
template <class Base>
bool checkpoint<Base>::forward(
	size_t                    p ,
	size_t                    q ,
	const vector<bool>&      vx ,
	      vector<bool>&      vy ,
	const vector<Base>&      tx ,
	      vector<Base>&      ty )
{	size_t n = f_[THREAD].Domain();
	size_t m = f_[THREAD].Range();
	//
	CPPAD_ASSERT_UNKNOWN( f_[THREAD].size_var() > 0 );
	CPPAD_ASSERT_UNKNOWN( tx.size() % (q+1) == 0 );
	CPPAD_ASSERT_UNKNOWN( ty.size() % (q+1) == 0 );
	CPPAD_ASSERT_UNKNOWN( n == tx.size() / (q+1) );
	CPPAD_ASSERT_UNKNOWN( m == ty.size() / (q+1) );
	bool ok  = true;
	//
	if( vx.size() == 0 )
	{	// during user forward mode
		if( jac_sparse_set_[THREAD].n_set() != 0 )
			jac_sparse_set_[THREAD].resize(0,0);
		if( jac_sparse_bool_[THREAD].size() != 0 )
			jac_sparse_bool_[THREAD].clear();
		//
		if( hes_sparse_set_[THREAD].n_set() != 0 )
			hes_sparse_set_[THREAD].resize(0,0);
		if( hes_sparse_bool_[THREAD].size() != 0 )
			hes_sparse_bool_[THREAD].clear();
	}
	if( vx.size() > 0 )
	{	// need Jacobian sparsity pattern to determine variable relation
		// during user recording using checkpoint functions
		if( sparsity() == atomic_base<Base>::set_sparsity_enum )
		{	if( jac_sparse_set_[THREAD].n_set() == 0 )
				set_jac_sparse_set();
			CPPAD_ASSERT_UNKNOWN( jac_sparse_set_[THREAD].n_set() == m );
			CPPAD_ASSERT_UNKNOWN( jac_sparse_set_[THREAD].end()   == n );
			//
			for(size_t i = 0; i < m; i++)
			{	vy[i] = false;
				local::sparse_list::const_iterator set_itr(
					jac_sparse_set_[THREAD], i
				);
				size_t j = *set_itr;
				while(j < n )
				{	// y[i] depends on the value of x[j]
					// cast avoid Microsoft warning (should not be needed)
					vy[i] |= static_cast<bool>( vx[j] );
					j = *(++set_itr);
				}
			}
		}
		else
		{	if( jac_sparse_set_[THREAD].n_set() != 0 )
				jac_sparse_set_[THREAD].resize(0, 0);
			if( jac_sparse_bool_[THREAD].size() == 0 )
				set_jac_sparse_bool();
			CPPAD_ASSERT_UNKNOWN( jac_sparse_set_[THREAD].n_set() == 0 );
			CPPAD_ASSERT_UNKNOWN( jac_sparse_bool_[THREAD].size() == m * n );
			//
			for(size_t i = 0; i < m; i++)
			{	vy[i] = false;
				for(size_t j = 0; j < n; j++)
				{	if( jac_sparse_bool_[THREAD][ i * n + j ] )
					{	// y[i] depends on the value of x[j]
						// cast avoid Microsoft warning
						vy[i] |= static_cast<bool>( vx[j] );
					}
				}
			}
		}
	}
	// compute forward results for orders zero through q
	ty = f_[THREAD].Forward(q, tx);

	// no longer need the Taylor coefficients in f_
	// (have to reconstruct them every time)
	// Hold onto sparsity pattern because it is always good.
	size_t c = 0;
	size_t r = 0;
	f_[THREAD].capacity_order(c, r);
	return ok;
}
# undef THREAD
# endif //  CPPAD_MULTI_THREAD_TMB

} // END_CPPAD_NAMESPACE
# endif