#include "next_element.hpp"
#include "scene.hpp"

float wiggle::operator () ( float& val, element_context& context  )
{
    wavelength( context ); amplitude( context ); phase( context ); wiggliness( context );
    if( *wavelength != 0.0f ) 
        return *amplitude * sin( ( val / *wavelength + *phase + *wiggliness * context.t ) * TAU );
    else return 0.0f; 
}

float index_param::operator () ( float& val, element_context& context ) {
    float findex = context.el.index * 1.0f;    
    return fn( findex, context );
}

float scale_param::operator () ( float& val, element_context& context ) {    
    return fn( context.el.scale, context );
}

float time_param::operator () ( float& val, element_context& context ) {    
    return fn( context.t, context );
}

bool orientation_gen_fn::operator () ( element_context& context ) 
    { context.el.orientation = *orientation; return true; }

bool scale_gen_fn::operator () ( element_context& context ) 
    { context.el.scale = *scale; return true; }

bool advect_element::operator () ( element_context& context ) { 
    flow( context ); step( context );
    element& el = context.el;
    if( proportional ) el.position += *flow * *step * el.scale; 
    else               el.position = *flow * *step;
    return true;
}

void angle_branch::render_branch( const float& ang, element_context& context )
{
    cluster cl( context.cl ); 
    cl.set_root( context.el );
    element& el = cl.root_elem;
    cl.depth++;
    el.scale *= *size_prop;
    if( ( el.scale >= cl.min_scale ) && ( cl.depth <= cl.max_depth ) ) {
        // change branch rule or other cluster params here
        el.orientation += ang;
        el.derivative = rot_deg( el.derivative, ang );
        el.index = 0;
        // calculate new position - relative to angle of motion if relevant
        //float ang = vtoa( el.derivative ) + branch_ang;
        el.position += *branch_dist * ( linalg::normalize( el.derivative ) * ( context.el.scale + el.scale ) );
        // render branch
        cl.render( context.img, context.t );
    }
}

bool angle_branch::operator () ( element_context& context ) {
    size_prop( context ); branch_ang( context ); branch_dist( context );
    element& el = context.el;
    if( !( ( el.index + offset ) % interval ) ) render_branch( *branch_ang, context );
    if( mirror_offset.has_value() ) {
        if( !( ( el.index + *mirror_offset ) % interval ) ) render_branch( -*branch_ang, context );
    }
    return true;
}

bool curly::operator () ( element_context& context ) {
    curliness( context );
    element& el = context.el;
    if( el.scale > 0.0f ) el.orientation += *curliness / el.scale;
    return true;
}

bool position_list::operator () ( element_context& context ) {
    positions( context );
    element& el = context.el;
    if( el.index < (*positions).size() ) el.position = (*positions)[ el.index ];
    return true;
}

bool next_element::operator () ( element_context& context ) { 
    element& el = context.el;
    el.index++;
    std::cout << "next_element - functions.size() = " << functions.size() << "\n";
    std::cout << "max_index = " << max_index << "\n";
    if( functions.size() == 0 ) return false;   // if no functions, no next element
    if( el.index >= max_index ) return false;
    vec2f p = el.position;
    for( auto fn : functions ) {
        std::cout << "executing function\n";
        if( !fn( context ) ) return false;
    }
    if( el.scale < context.cl.min_scale ) return false;
    el.derivative = el.position - p;
    // bounds check
    if( bounds.has_value() ) {
        bounds->pad( el.scale );
        if( !( bounds->in_bounds_pad( el.position ) ) ) return false;
    }
    return true; 
}
   
