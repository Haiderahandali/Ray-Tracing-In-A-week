#ifndef RAY_HPP
#define RAY_HPP

#define MAX_SPHERE_COUNT 8
#define MAX_PLANE_COUNT  8



#define KB(Value) (1024 * (Value)) 
#define MB(Value) (1024 * KB(Value))
#define GB(Value) (1024 * MB(Value))

#define Assert(Expr) if(!(Expr)) { __builtin_trap(); }

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

typedef float f32;

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

struct ray
{
    v3 Origin;
    v3 Dir;
};


struct sphere
{
    f32 R;
    v3  Center;
};

struct plane
{
    f32 D;
    v3  N;

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
    f32 W;      
    f32 H;      
    f32 HalfW;  
    f32 HalfH;  
    f32 X;      // Film Width  in Normalized Coordinates -1 to 1
    f32 Y;      // Film Height in Normalized Coordinates -1 to 1
    f32 Dist;   // Distance in Negative Z direction

    v3 Center;
};

struct camera
{
    v3 Origin;
    v3 DirX;
    v3 DirY;
    v3 DirZ;
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
