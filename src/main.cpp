#include "Ray.hpp"


#include <iostream>
#include <cstdio>
#include <fstream>
#include <cmath>




#define BREAKPOINT \
asm("int $3")


Internal INLINE std::ostream& operator<<(std::ostream& OutStream, v3* V)
{
    return OutStream << V->X << ' ' << V->Y << ' ' << V->Z;
}


//returns a number between 0 and 0.05
f32 Random()
{
    s32 RandomNumber = std::rand()%1000;

    return (f32)RandomNumber/10000.0f;
}


Internal INLINE f32 Inner(v3 U, v3 V)
{
    return (U.X * V.X + U.Y * V.Y +  U.Z * V.Z );
}

v3 NOZ(v3 U)
{
    f32 Domen = sqrtf(Inner(U,U));

    return 
    {
        U.X / Domen,
        U.Y / Domen,
        U.Z / Domen
    };
}

v3 Cross(v3 V1, v3 V2)
{
    v3 Result = {};

    Result.X = (V1.Y * V2.Z) - (V1.Z * V2.Y);
    Result.Y = (V1.Z * V2.X) - (V1.X * V2.Z);
    Result.Z = (V1.X * V2.Y) - (V1.Y * V2.X);
    return (Result);
}




void WriteColor(std::ostream& OutStream, v3 Color)
{
    OutStream << (s32) (Color.X * 255.999f) << ' '
    << (s32) (Color.Y * 255.999f) << ' '
    << (s32) (Color.Z * 255.999f) << '\n';

}





#define Tolerance 0.001f
#define TMax 100000.0f 
f32 RayIntersectSphere(ray* Ray, sphere* Sphere)
{

    f32 T = TMax;

    v3 X = Ray->Origin - Sphere->Center;

    f32 B = Inner(X, Ray->Dir);
    f32 C = Inner(X,X) - (Sphere->R * Sphere->R);


    f32 Disc = B*B - C;


    if(Disc < 0.0f) 
    {
        return T;
    }
    T = -B - sqrtf(Disc);
    if (T < Tolerance)
    {
        T = -B + sqrtf(Disc);
    }


    return T;

}


v3 RayColor(ray* Ray) 
{
    v3 Dir = NOZ(Ray->Dir);
    f32 t = 0.5f * ((Dir.Y) + 1.0f);
    return ((1.0f-t)* v3{1.0f, 1.0f, 1.0f}) + (t * v3{0.5f, 0.7f, 1.0f});
}


int main(void)
{

    // f32 AspectRatio = (f32) 16.0f / (f32) 9.0f;

#if 1
    s32 ImageWidth   =  512;
    s32 ImageHeight  =  512;
#else 
    s32 ImageWidth = 400;
    f32 AspectRatio = (f32) 16.0f / (f32) 9.0f;

    s32 ImageHeight = (s32) ((f32) ImageWidth / AspectRatio);

#endif
    //------------- View Port and Camera -------------//

    v3 WorldUpVector = {0.0f, 1.0f, 0.0f};

    //Right Hand Coordinates System, Camera Pointing in the Negative Z-Axis

    camera Camera = {};
    Camera.Origin = {0.0f, 0.0f, 0.0f};
    Camera.DirZ = NOZ({0.0f ,0.0f , -1.0f});
    Camera.DirX = NOZ(Cross(Camera.DirZ, WorldUpVector));
    Camera.DirY = NOZ(Cross(Camera.DirX, Camera.DirZ));

    //----------- View Port ----------//
    film Film   = {};
    Film.H      = 2;
    Film.W      = 2;
    Film.HalfW  = Film.W * 0.5f ;
    Film.HalfH  = Film.H * 0.5f ;
    //---- camera is facing the negative Z Axis ----//
    Film.Dist = 1;



    //--------- Creating a sphere -------------//
    sphere Sphere  = {6.0f, {10, 30 , -40.0f}};


    //writing the ppm image header
    std::cout<< "P3\n" << ImageWidth << ' ' << ImageHeight <<"\n255\n";


    for(s32 Y = ImageHeight - 1; Y >= 0; --Y)
    {
        std::cerr << (u32)(100 * (1- ((f32)Y/ (f32)(ImageHeight -1)))) << "%" << ' ' << std::flush;


        Film.Y = (2 * (f32) Y / (f32) (ImageHeight - 1) ) - 1;


        for(s32 X = 0; X < ImageWidth; ++X)
        {
            Film.X = (2* (f32) X / (f32) (ImageWidth -1)) - 1;

            ray Ray = {};

            Ray.Origin = Camera.Origin;
            Ray.Dir = Film.Dist * Camera.DirZ;
            Ray.Dir += Film.Y * Film.HalfH * Camera.DirY;
            Ray.Dir += Film.X * Film.HalfW * Camera.DirX;
            Ray.Dir = NOZ(Ray.Dir);


            v3 Color;
            f32 T = RayIntersectSphere(&Ray, &Sphere);
            if( (T < TMax)) 
            {

                #if 0 // With Shading

                v3 P = (Ray.Dir*T) + Ray.Origin;
                v3 PNormal = (P - Sphere.Center) / Sphere.R;

                v3 LightDir = NOZ(P - Camera.Origin);

                f32 CosAngle = Inner(-LightDir, PNormal);

                Color = CosAngle *  v3{1.0f, 0.0f, 0.0f};    
                #else //No shading

                Color = {1.0f, 0.0f, 0.0f};
                #endif 

            }
            else
            {
                Color =  {0.0f, 1.0f, 0.0f};
            }
            
            WriteColor(std::cout, Color);
        }
    }

    std::cerr << "\n-- Done --\n";

    return(0);
}
