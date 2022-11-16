#ifndef __SCENE_HPP
#define __SCENE_HPP

#include <variant>
#include <map>
#include <optional>
#include "image.hpp"
#include "any_image.hpp"
#include "any_function.hpp"
#include "effect.hpp"

//template< class T > struct effect;
struct element;
struct cluster;

struct element_context {
    element& el;
    cluster& cl;
    any_image_ptr& img;  // image being rendered upon
    float t;  // time
    // scene scn;
    // derivative?

    element_context( element& el_init, cluster& cl_init, any_image_ptr& img_init, const float& t_init = 0.0f ) :
        el( el_init ), cl( cl_init ), img( img_init ), t( t_init ) {}
};

// An element object contains all the data members needed to create an image splat
struct element {
    // std::vector< effect< T >& > effects; // should effects be generative functions?
    
    vec2f position; 		    // coordinates of element center (relative to parent cluster)
    float scale; 			    // radius of element
    float rotation; 	        // image rotation in degrees
    float orientation;          // direction of motion relative to vector field ( or function )
    bool  orientation_lock;     // is rotation relative to orientation?
    int index;                  // index of element within cluster
    mask_mode mmode;            // how will mask be applied to splat and backround?

    any_image_ptr img;  // If no image, element not rendered, serves as placeholder
    any_image_ptr mask;
    std::optional< any_pixel > tint;	// change the color of element

    // approximate absolute derivative used to calculate angle of branches
    // calculated from delta since last position or directly (for instance in the case of a circle)
    vec2f derivative;   // move to element_context?
    bool derivative_lock;

    void render( any_image_ptr& in, const float& t );

    // render into a buffer pair. Rendering modifies image directly - does not require buffer swap.
    // in this case the element serves as an effect functor 

    //void operator () ( any_buffer_pair buf, const float& t ); // render into a buffer pair. Rendering does not require buffer swap.

    element(    const vec2f& position_init =  { 0.0f, 0.0f },
                const float& scale_init = 1.0f,
                const float& rotation_init = 0.0f,
                const float& orientation_init = 0.0f,
                any_image_ptr img_init = null_fimage_ptr,
                any_image_ptr mask_init = null_fimage_ptr,
                const std::optional< any_pixel > tint_init = std::nullopt,
                const mask_mode mmode_init = MASK_BLEND           
            ) 
        : position( position_init ),
            scale( scale_init ),
            rotation( rotation_init ),
            orientation( orientation_init ),
            img( img_init ),
            mask( mask_init ),
            tint( tint_init ),
            mmode( mmode_init ),
            index( 0 ) {}
};

struct next_element;

// A default cluster with root set to default element with a single image should produce a full frame image, e.g. for background
struct cluster {
    element root_elem;       // initial element in cluster
    next_element& next_elem; // Functor to recursively generate elements in cluster

    int max_n;          // limit to number of elements
    int depth;          // counter to keep track of depth in tree
    int max_depth;      // prevent infinite recursion
    float min_scale;    // approximately one pixel

    std::optional< bb2f > bounds;   // Optionally, cluster will stop generating if it goes out of bounds

    // Recursively generate branches and render elements
    void render( any_image_ptr& img, const float& t = 0.0f );

    // change root element parameters for branching cluster
    void set_root( element& el );

    // render into a buffer pair. Rendering modifies image directly - does not require buffer swap.
    // in this case the cluster serves as an effect functor (the effect being rendering)
    // need generic buffer pair
    //void operator () ( buffer_pair< T >& buf, const float& t );

    cluster( const element& el,  
             next_element& next_elem_init, 
             const int& max_n_init = 1,
             const int& depth_init = 0,
             const int& max_depth_init = 10,
             const float& min_scale_init = 0.001f,
             const std::optional< bb2f >& bounds_init = std::nullopt
            )
        : root_elem( el ),
          next_elem( next_elem_init ),
          max_n( max_n_init ),
          depth( depth_init ),
          max_depth( max_depth_init ),
          min_scale( min_scale_init ),
          bounds( bounds_init ) 
          {}
};

struct scene {
    // scene owns clusters, elements, images, effects, and functions
    std::string name;
    std::map< std::string, any_image_ptr > images;
    //std::map< std::string, any_eff_fn_ptr > effects;
    std::map< std::string, std::shared_ptr< element > > elements;

    // need list of harness functions by type (map of maps?)
    std::map< std::string, any_float_fn_ptr > float_fn_ptrs;
    std::map< std::string, float_fn > float_fns;    
    std::map< std::string, any_int_fn_ptr > int_fn_ptrs;
    std::map< std::string, int_fn > int_fns;    
    std::map< std::string, any_vec2f_fn_ptr > vec2f_fn_ptrs;
    std::map< std::string, vec2f_fn > vec2f_fns;    
    std::map< std::string, any_vec2i_fn_ptr > vec2i_fn_ptrs;
    std::map< std::string, vec2i_fn > vec2i_fns;

    std::map< std::string, any_gen_fn_ptr > gen_fn_ptrs;
    std::map< std::string, gen_fn > gen_fns;

    std::map< std::string, std::shared_ptr< next_element > > next_elements; // next element functions tagged with cluster names
    std::map< std::string, std::shared_ptr< cluster > > clusters; // scene defined as a set of clusters
    std::vector< std::string > tlc;   // list of top level cluster names in rendering order
    
    vec2i size; // size of output image
    
    scene( std::string filename, vec2i size_init = { 1024, 1024 } );   // Load scene file (JSON)

    void set_size( vec2i size_init );

    // Add template - T is image type (fimage, uimage, vfield, etc.)
    void render( std::string filename, float time = 0.0f );

    template< class T > void animate( std::string basename, int nframes = 100 );
};

#endif // __SCENE_HPP