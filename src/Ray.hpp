#ifndef RAY_HPP
#define RAY_HPP

#define MAX_SPHERE_COUNT   20
#define MAX_PLANE_COUNT    20
#define MAX_MATERIAL_COUNT 10

#define KB(Value) (1024 * (Value)) 
#define MB(Value) (1024 * KB(Value))
#define GB(Value) (1024 * MB(Value))

#define PI32 3.141592f
#define PI64 3.141592653589793

#define EPSILON32 0.000001f
#define EPSILON64 0.00000000001

#define ArrayCount(Array) sizeof(Array)/sizeof(Array[0])

#define DegreeToRad(Angle) ((f32) ((Angle) * PI32 / 180.0f))

#if DEBUG_BUILD 
    #define Assert(Expr) if(!(Expr)) { __builtin_trap(); }
#else 
    #define Assert(Expr)
#endif 

#define INLINE __attribute__((always_inline)) inline

#define Internal static

#include <cstdint>

typedef int64_t  s64;
typedef int32_t  s32;
typedef int8_t   s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t  u8;

typedef s32 bool32;
typedef s32 b32;

typedef float  f32;
typedef double f64;


struct v2
{
    f32 X;
    f32 Y;
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

enum Material_Tpye
{
    BACKGROUND = 0,
    DIFFUSE    = 1,
    METALIC    = 2,
};


struct material
{
    v3 AlbedoColor;
    f32 RefractedIndex;
    f32 Fuzz;
};

struct ray
{
    v3 Origin;
    v3 Dir;
};

struct sphere
{
    v3  Center;
    f32 R;
    u32 MaterialIndex;
};

struct plane
{
    f32 D;
    v3  N;

};

struct world
{
    sphere Sphere[MAX_SPHERE_COUNT]      ;
    plane  Plane[MAX_PLANE_COUNT]        ;
    material Material[MAX_MATERIAL_COUNT];

    u32 SphereCount;
    u32 PlaneCount;
    u32 MaterialCount;
};

struct film
{
    v3 Center;
    f32 W;      
    f32 H;      
    f32 HalfW;  
    f32 HalfH;  
    f32 X;      // Film Width  in Normalized Coordinates -1 to 1
    f32 Y;      // Film Height in Normalized Coordinates -1 to 1
    f32 Dist;   // Distance in Negative Z direction

};

struct camera
{
    v3 Origin;
    v3 DirX;
    v3 DirY;
    v3 DirZ;

    f32 AspectRatio;
    f32 VFOV; // vertical FOV

};


struct hit_info
{
    v3 HitPoint;
    v3 Normal;
    material* MaterialPtr; 
    f32 T; // ths is paramter of a ray equation given by: Ray(t) =RayOrigin +  (RayDirection * t)
    b32 RayIsOutward; //if the ray hitted an object, was pointing in the same direction on not relative to the normal of the surface it did hit
};



//--------------------- Operator Overloading for vector operations BEGIN ------------------//
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


v3& operator += (v3& U, v3 V)
{
    
        U.X += V.X;  
        U.Y += V.Y;  
        U.Z += V.Z;

        return U;
}

//--------------------- Operator Overloading for vector operations END ------------------//

#endif
