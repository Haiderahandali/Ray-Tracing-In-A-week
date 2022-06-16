#ifndef RAY_HPP
#define RAY_HPP

#define MAX_SPHERE_COUNT 8
#define MAX_PLANE_COUNT  8

#include <cstdint>

#define KB(Value) (1024 * (Value)) 
#define MB(Value) (1024 * KB(Value))
#define GB(Value) (1024 * MB(Value))

#define Assert(Expr) if(!(Expr)) { __builtin_trap(); }

#define INLINE __attribute__((always_inline)) inline

#define Internal static

typedef int64_t  s64;
typedef int32_t  s32;
typedef int8_t   s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t  u8;

typedef s32 bool32;
typedef s32 b32;

typedef float f32;




union v2
{
    struct
    {
        f32 X;
        f32 Y;
    };
    struct
    {
        f32 U;
        f32 V;
    };
};




struct v3
{
    f32 X;
    f32 Y;
    f32 Z;
};

struct v4
{
    f32 X;
    f32 Y;
    f32 Z;
    f32 W;
};

struct material
{
    v3 Color;
};

struct sphere
{
    f32 R;
    v3  Center;

    u32 MaterialIndex;
};

struct plane
{
    f32 D;
    v3  N;

    u32 MaterialIndex;
};


struct world
{
    v3 O;
    sphere Sphere[MAX_SPHERE_COUNT] ;
    plane  Plane[MAX_PLANE_COUNT]  ;
    u32 SphereCount;
    u32 PlaneCount;
};

struct film
{
    // Right Hand Coordinates System 

    u32 FilmW; // Film Width  In Pixels 
    u32 FilmH; // Film Height In Pixels
    u32 HalfFilmW;
    u32 HalfFilmH;
    f32 FilmX; // Film Width   in Normalized Coordinates -1 to 1
    f32 FilmY; // Film Height  in Normalized Coordinates -1 to 1
    f32 FilmDist; // Distance in Negative Z direction

    v3 FilmCenter;
};

struct image
{
    u8* ColorBuffer;
    u32 W;
    u32 H;
    u8 BPP;
    u8 ColorValue;
};


struct polygon
{
    v3 Vertices[20];
    u32  Count;

    u32 MaterialIndex;
};

struct polygon_2d
{
    v2 Vertices[20];
    u32 Count;

    u32 MaterialIndex;
};

struct triangle
{
    v3 Vertices[3];
    u32 Count;

    u32 MaterialIndex;
};

struct quadrilateral
{
    v3 Vertices[4];
    u32 Count;

    u32 MaterialIndex;

};

typedef quadrilateral quad;



Internal INLINE v2 operator- (v2& U, v2& V)
{
    return {U.X - V.X, U.Y - V.Y};
}


Internal INLINE v2 operator- (v2&& U, v2& V)
{
    return {U.X - V.X, U.Y - V.Y};
}



Internal INLINE v3 operator+ (v3 U, f32 C)
   {
    return {U.X + C ,
        U.Y + C ,
        U.Z + C 
    };   
}

Internal INLINE v3 operator+ ( f32 C, v3 U)
{
    return {U.X + C ,
        U.Y + C ,
        U.Z + C 
    };   
}

Internal INLINE v3 operator+ (v3 U, v3 V)
{
    return {U.X + V.X ,
        U.Y + V.Y ,
        U.Z + V.Z 
    };
}

Internal INLINE v3 operator- (v3 U, v3 V)
{
    return {U.X - V.X ,
        U.Y - V.Y ,
        U.Z - V.Z 
    };
}

Internal INLINE v3 operator- (v3 U, f32 C)
{
    return {U.X - C ,
        U.Y - C ,
        U.Z - C 
    };   
}

Internal INLINE v3 operator- (v3 U)
{
    return {-U.X,
            -U.Y,
            -U.Z  
            };   
}

Internal INLINE v3 operator- ( f32 C, v3 U)
{
    return {U.X - C ,
        U.Y - C ,
        U.Z - C 
    };   
}

Internal INLINE v3 operator* (v3 U, f32 C)
{
    return {U.X * C ,
            U.Y * C ,
            U.Z * C 
    };
}

Internal INLINE v3 operator* (f32 C, v3 U)
{
    return {U.X * C ,
            U.Y * C ,
            U.Z * C 
            };
}

Internal INLINE v3 operator/ (v3 U, f32 C)
{
    return {U.X/C,
            U.Y/C,
            U.Z/C};
}

Internal INLINE v3 operator/ (f32 C, v3 U)
{
    return {U.X/C,
            U.Y/C,
            U.Z/C};
}

// v3 operator += (v3 U, f32 C)
// {
//     return {U + C};
// }
// v3 operator += (v3 U, v3 V)
// {
//     return {U + V};
// }

// v3 operator -= (v3 U, f32 C)
// {
//     return {U - C};
// }
// v3 operator -= (v3 U, v3 V)
// {
//     return {U - V};
// }

//Those are evil for now
// v3 operator *= (v3 U, f32 C)
// {
//     v3 Result = {U*C};
//     return (Result);
// }

// v3 operator /= (v3 U, f32 C)
// {
//     return {U / C};
// }



#endif
