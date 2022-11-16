#ifndef __ANY_FUNCTION
#define __ANY_FUNCTION

#include <variant>
#include "next_element.hpp"

typedef std::variant < 
    // harness functions
    std::shared_ptr< identity_float >,
    std::shared_ptr< adder_float >,
    std::shared_ptr< log_fn >,
    std::shared_ptr< ratio_float >,
    std::shared_ptr< wiggle >,

    // parameter functions
    std::shared_ptr< index_param >,
    std::shared_ptr< scale_param >,
    std::shared_ptr< time_param >

> any_float_fn_ptr;

typedef std::variant <
    // harness functions
    std::shared_ptr< identity_int >,
    std::shared_ptr< adder_int >
> any_int_fn_ptr;

typedef std::variant <
    // harness functions
    std::shared_ptr< identity_vec2f >,
    std::shared_ptr< adder_vec2f >,
    std::shared_ptr< ratio_vec2f >
> any_vec2f_fn_ptr;

typedef std::variant <
    // harness functions
    std::shared_ptr< identity_vec2i >,
    std::shared_ptr< adder_vec2i >
> any_vec2i_fn_ptr;

typedef std::variant < 
    // single field modifiers
    std::shared_ptr< orientation_gen_fn >,
    std::shared_ptr< scale_gen_fn >,

    // generalized functions (alphabetical order)
    std::shared_ptr< advect_element >,
    std::shared_ptr< angle_branch >,
    std::shared_ptr< curly >,
    std::shared_ptr< position_list >

> any_gen_fn_ptr;

template< class T > gen_fn   to_gen_fn(   std::shared_ptr< T > ptr );
gen_fn resolve_gen_fn( any_gen_fn_ptr any_fn_ptr);

template< class T > float_fn to_float_fn( std::shared_ptr< T > ptr );
float_fn resolve_float_fn( any_float_fn_ptr any_fn_ptr);

template< class T > int_fn   to_int_fn( std::shared_ptr< T > ptr );
int_fn resolve_int_fn( any_int_fn_ptr any_fn_ptr);

template< class T > vec2f_fn to_vec2f_fn( std::shared_ptr< T > ptr );
vec2f_fn resolve_vec2f_fn( any_vec2f_fn_ptr any_fn_ptr);

template< class T > vec2i_fn to_vec2i_fn( std::shared_ptr< T > ptr );
vec2i_fn resolve_vec2i_fn( any_vec2i_fn_ptr any_fn_ptr);

#endif // __ANY_FUNCTION