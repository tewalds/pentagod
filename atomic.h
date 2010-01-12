
#ifndef _ATOMIC_H_
#define _ATOMIC_H_

// http://gcc.gnu.org/onlinedocs/gcc/Atomic-Builtins.html

#if defined(_OPENMP) || MAX_THREADS > 1

#define CASv(var, old, new) __sync_bool_compare_and_swap(&(var), old, new)
#define CASp(ptr, old, new) __sync_bool_compare_and_swap(ptr, old, new)
#define INCR(var)           __sync_add_and_fetch(&(var), 1)
#define PLUS(var, val)      __sync_add_and_fetch(&(var), val)

#else

#define CASv(var, old, new) ((var) == (old) ? (var) = (new) : (false));
//if((var) == (old)) { (var) = (new); }
#define CASp(ptr, old, new) if(*(ptr) == (old)) { *(ptr) = (new); }
#define INCR(var)           (++(var))
#define PLUS(var, val)      ((var) += (val))

/*
template <class T> bool CASv(T& var, const T& old, const T& val) {
	if (var == old){
		var = val;
		return true;
	}
	return false;
}
//*/
#endif

#endif

