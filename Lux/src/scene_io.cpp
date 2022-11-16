#include "scene.hpp"
#include "scene_io.hpp"
#include "fimage.hpp"
#include "uimage.hpp"
#include "vector_field.hpp"
#include <fstream>
#include <sstream>

using json = nlohmann::json;

scene_reader::scene_reader( scene& s_init, std::string( filename ) ) : s( s_init ) {
    // read a JSON file
    std::ifstream in_stream(filename);
    json j = json::parse(in_stream); 

    // scene fields
    if( j.contains( "name" ) ) j[ "name" ].get_to( s.name ); else s.name = "Unnamed";
    std::cout << "scene_reader constructor: name = " << s.name << std::endl;
    if( j.contains( "size" ) ) s.size = read_vec2i( j[ "size" ] ); else s.size = { 1024, 1024 };
    std::cout << "scene_reader constructor: size = " << s.size.x << " " << s.size.y << std::endl;
    if( j.contains( "images" ) )    for( auto& jimg :   j[ "images" ] )   read_image( jimg );
    std::cout << "scene_reader constructor: images read\n";
    // effects - TBI
    //if( j.contains( "effects" ) ) for( auto& jeff : j[ "effects" ] ) read_effect( jeff );
    if( j.contains( "elements" ) )  for( auto& jelem :  j[ "elements"  ] ) read_element(  jelem  );  
    std::cout << "scene_reader constructor: elements read\n";
    if( j.contains( "functions" ) ) for( auto& jfunc :  j[ "functions" ] ) read_function( jfunc  );
    std::cout << "scene_reader constructor: functions read\n";
    if( j.contains( "clusters" ) )  for( auto& jclust : j[ "clusters"  ] ) read_cluster(  jclust ); // if no clusters, blank scene
    std::cout << "scene_reader constructor: clusters read\n";
    //auto wiggle_damper = j.get<log_fn>();
    //std::cout << *(wiggle_damper.scale) << " " << *(wiggle_damper.shift) << std::endl; 
}

vec2f scene_reader::read_vec2f( const json& j ) { return vec2f( { j[0], j[1] } ); }
vec2i scene_reader::read_vec2i( const json& j ) { return vec2i( { j[0], j[1] } ); }
frgb  scene_reader::read_frgb(  const json& j ) { return frgb( j[0], j[1], j[2] ); }
bb2f  scene_reader::read_bb2f(  const json& j ) { return bb2f( read_vec2f( j[0] ), read_vec2f( j[1]) ); }
bb2i  scene_reader::read_bb2i(  const json& j ) { return bb2i( read_vec2i( j[0] ), read_vec2i( j[1]) ); }

// ucolor represented as hexidecimal string
ucolor scene_reader::read_ucolor(   const json& j ) { 
    std::string color;
    ucolor u;

    j.get_to( color );
    std::istringstream( color ) >> std::hex >> u;
    return u;
}

void scene_reader::read_image( const json& j ) {
    std::string type, name, filename;

    if( j.contains( "type") ) j[ "type" ].get_to( type );
    else {
        std::cout << "scene_reader::read_image error - image type missing\n";
        return;
    }

    if( j.contains( "filename" ) ) j[ "filename" ].get_to( filename );
    else {
        std::cout << "scene_reader::read_image error - image filename missing\n";
        return;
    }

    if( j.contains( "name" ) ) j[ "name" ].get_to( name );
    else name = filename;


    if( type == "fimage" ) {
        std::shared_ptr< fimage > fimage_ptr( new fimage( filename ) );
        s.images[ name ] = fimage_ptr;
    }

    if( type == "uimage" ) {
        std::shared_ptr< uimage > uimage_ptr( new uimage( filename ) );
        s.images[ name ] = uimage_ptr;
    }
    // future: binary image format, which will support fimage, uimage, and additionally vector_field
}

void scene_reader::read_element( const json& j ) {
    std::string name;
    vec2f position; 		    // coordinates of element center (relative to parent cluster)
    float scale; 			    // radius of element
    float rotation; 	        // image rotation in degrees
    float orientation;          // direction of motion relative to vector field ( or function )
    bool  orientation_lock;     // is rotation relative to orientation?
    int index;                  // index of element within cluster
    std::string mask_mode;      // how will mask be applied to splat and backround?

    std::string img;  // If no image, element not rendered, serves as placeholder
    std::string mask;

/*
    if( j.contains( "filename" ) ) { 
        std::string filename;    // Make sure file doesn't reference itself to prevent infinite loop
        j[ "filename" ].get_to( filename );
        std::ifstream subfile_stream(filename);
        json subfile = json::parse(subfile_stream); 
        read_element( subfile );
        return;
    }
*/
    if( j.contains( "name" ) ) j[ "name" ].get_to( name );  else throw std::runtime_error( "Element name missing\n" );
    std::cout << "reading element " << name << std::endl;
    s.elements[ name ] = std::make_shared< element >();
    element& elem = *(s.elements[ name ]);

    if( j.contains( "position" ) ) elem.position = read_vec2f( j[ "position" ] );
    std::cout << "  position = " << elem.position.x << " " << elem.position.y << std::endl;
    if( j.contains( "scale" ) )       j[ "scale"       ].get_to( elem.scale );
    std::cout << "  scale = " << elem.scale << std::endl;
    if( j.contains( "rotation" ) )    j[ "rotation"    ].get_to( elem.rotation );
    std::cout << "  rotation = " << elem.rotation << std::endl;
    if( j.contains( "orientation" ) ) j[ "orientation" ].get_to( elem.orientation );
    std::cout << "  orientation = " << elem.orientation << std::endl;
    if( j.contains( "orientation_lock" ) ) j[ "orientation_lock" ].get_to( elem.orientation_lock );
    std::cout << "  orientation_lock = " << elem.orientation_lock << std::endl;
    if( j.contains( "mask_mode" ) )   j[ "mask_mode"   ].get_to( mask_mode );
    std::cout << "  mask_mode = " << mask_mode << std::endl;
    
    if( mask_mode == "additive" ) elem.mmode = MASK_ADDITIVE;
    if( mask_mode == "trim"     ) elem.mmode = MASK_TRIM;
    if( mask_mode == "opacity"  ) elem.mmode = MASK_OPACITY;
    if( mask_mode == "blend"    ) elem.mmode = MASK_BLEND;

    // image, mask, tint
    if( j.contains( "image" ) ) {
        j[ "image" ].get_to( img );
        std::cout << "  image: " << img << std::endl;
        elem.img = s.images[ img ];
    }

    if( j.contains( "mask" ) ) {
        j[ "mask" ].get_to( mask );
        std::cout << "  mask: " << mask << std::endl;
        elem.mask = s.images[ mask ];
    }

    if( j.contains( "tint" ) ) {
        auto k = j[ "tint" ];
        if( k.contains( "frgb"   ) ) elem.tint = read_frgb(   k[ "frgb" ]   );
        if( k.contains( "ucolor" ) ) elem.tint = read_ucolor( k[ "ucolor" ] );
        if( k.contains( "vec2f"  ) ) elem.tint = read_vec2f(  k[ "vec2f" ]  );
    }
}

// forward references not implemented
template< class T > void scene_reader::read_harness( std::string& field_name, const json& j, harness< T >& h, std::map< std::string, std::function< T ( T&, element_context& ) > >& harness_fns ) {
    if( j.contains( field_name ) ) {
        json k = j[ field_name ];
        if( k.is_object() ) {
            if( k.contains( "value" ) ) read( h.val, k );
            // need list of harness functions by type (map of maps?)
            if( k.contains( "functions" ) ) for( std::string name : k ) h.functions.push_back( harness_fns[ name ] );
        }
        else read( h.val, k );
    }    
}

void scene_reader::read_function( const json& j ) {
    std::string name, type, fn_name;

    if( j.contains( "name" ) ) j[ "name" ].get_to( name );  else throw std::runtime_error( "Function name missing\n" );
    if( j.contains( "type" ) ) j[ "type" ].get_to( type );  else throw std::runtime_error( "Function type missing\n" );

    std::cout << "read_function name = " << name << " type = " << type << std::endl;
    // example of expanded macro
    /* if( type == "orientation_gen_fn" ) { 
        auto fn_ptr = std::make_shared< orientation_gen_fn >();
        s.functions[ name ] = fn_ptr;
        if( j.contains( "orientation" ) ) read_harness< float >( "orientation", j[ "orientation" ], fn_ptr->orientation );
    } */

    #define GEN_FN( _T_ )  if( type == #_T_ ) { std::cout << "type = " << type << std::endl; auto fn_ptr = std::make_shared< _T_ >(); s.gen_fn_ptrs[ name ] = fn_ptr; s.gen_fns[ name ] = *fn_ptr; 
    #define FLOAT_FN( _T_ )  if( type == #_T_ ) { auto fn_ptr = std::make_shared< _T_ >(); s.float_fn_ptrs[ name ] = fn_ptr; s.float_fns[ name ] = *fn_ptr;
    #define HARNESS( _T_, _U_ ) if( j.contains( #_T_ ) ) read_any_harness( #_T_, j[ #_T_ ], fn_ptr->_T_, _U_ );
    #define READ( _T_ ) if( j.contains( #_T_ ) ) read( fn_ptr->_T_, j[ #_T_ ] );
    #define FLOAT_PARAM if( j.contains( "fn" ) ) { j[ "fn" ].get_to( fn_name); fn_ptr->fn = s.float_fns[ fn_name ]; }
    #define END }

    // harness float functions
    FLOAT_FN( adder_float ) HARNESS( r, s.float_fns ) END
    FLOAT_FN( ratio_float ) HARNESS( r, s.float_fns ) END

    // parameter functions
    FLOAT_FN( index_param ) FLOAT_PARAM END
    FLOAT_FN( scale_param ) FLOAT_PARAM END
    FLOAT_FN( time_param )  FLOAT_PARAM END

    // single field modifiers
    GEN_FN( orientation_gen_fn ) HARNESS( orientation, s.float_fns ) END
    GEN_FN( scale_gen_fn ) HARNESS( scale, s.float_fns ) END

    // generalized functions (alphabetical order)
    GEN_FN( advect_element ) HARNESS( flow, s.vec2f_fns ) HARNESS( step, s.float_fns ) READ( proportional ) READ( smooth ) END
    GEN_FN( angle_branch ) READ( interval ) READ( offset ) READ( mirror_offset) HARNESS( size_prop, s.float_fns ) HARNESS( branch_ang, s.float_fns ) HARNESS( branch_dist, s.float_fns ) END
    GEN_FN( curly ) HARNESS( curliness, s.float_fns ) END
    // position_list should go here - figure out how to work the vector of positions
}

void scene_reader::read_cluster( const json& j ) {
    std::string name;
    element root_elem;  // initial element in cluster
    std::string root_elem_name;
    int max_depth;      // prevent infinite recursion
    float min_scale;    // approximately one pixel
    bb2f bounds;        // Optionally, cluster will stop generating if it goes out of bounds
    bool tlc;

/*
    if( j.contains( "filename" ) ) { 
        std::string filename;    // Make sure file doesn't reference itself to prevent infinite loop
        j[ "filename" ].get_to( filename );
        std::ifstream subfile_stream(filename);
        json subfile = json::parse(subfile_stream); 
        read_cluster( subfile );
        return;
    }
    */

    // Required fields
    if( j.contains( "name" ) )          j[ "name" ].get_to( name );  else throw std::runtime_error( "Cluster name missing\n" );
    // Check for unique name. Future - make sure duplicate clusters refer to the same cluster

    if( s.clusters.contains( name ) )   throw std::runtime_error( "Cluster name collision\n" ); 
    //else throw( "Cluster next_element missing" );
    std::cout << " reading cluster " << name << std::endl;         
    if( j.contains( "element" ) )     j[ "element" ].get_to( root_elem_name ); else throw std::runtime_error( "Cluster root_elem missing\n" );
    std::cout << "  root element " << root_elem_name << std::endl;

    // create cluster object
    s.next_elements[ name ] = std::make_shared< next_element >();
    s.clusters[ name ] = std::make_shared< cluster >( *( s.elements[ root_elem_name ] ), *( s.next_elements[ name ] ) );
    cluster& clust = *(s.clusters[ name ]);   // reference to cluster object
    std::cout << "created cluster object\n";

    // optional fields
    if( j.contains( "max_n" ) )         j[ "max_n" ].get_to( clust.max_n );
    if( j.contains( "max_depth" ) )     j[ "max_depth" ].get_to( clust.max_depth );
    if( j.contains( "min_scale" ) )     j[ "min_scale" ].get_to( clust.min_scale );
    if( j.contains( "bounds" ) )        clust.bounds = read_bb2f( j[ "bounds" ] );

    std::cout << "About to read next_element\n";
    if( j.contains( "next_element" ) )  for( std::string fname : j[ "next_element" ] ) 
    {
        std::cout << "  Reading next element function " << fname << std::endl;
        clust.next_elem.add_function( s.gen_fns[ fname ] ); // Empty next_element is allowed - useful for single element clusters
    }
    clust.next_elem.max_index = clust.max_n;    // Set limit to the number of elements in cluster
    std::cout << "About to read tlc\n";
    if( j.contains( "tlc" ) )  { 
        j[ "tlc" ].get_to( tlc );
        if( tlc ) s.tlc.push_back( name );
    }
    std::cout << "cluster " << name << " completed - tlc = " << tlc << std::endl;
}
