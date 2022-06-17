#include "Ray.hpp"

#include <iostream>
#include <fstream>
#include <cmath>




#define BREAKPOINT asm("int $3")


Internal INLINE std::ostream& operator<<(std::ostream& OutStream, v3* V)
{
    return OutStream << V->X << ' ' << V->Y << ' ' << V->Z;
}


//returns a number [0 , 1)
Internal INLINE f32 Random(void)
{
    return  (f32)std::rand() / ((f32)RAND_MAX + 1.0f);
}


//returns a number between [IntervalMin , IntervalMax)
Internal INLINE f32 RandomInterval(f32 IntervalMin, f32 IntervalMax)
{
    return (IntervalMin + (IntervalMax - IntervalMin)*Random());
}   

f32 ClampValueBetween(f32 Value, f32 Min, f32 Max)
{
    if(Value < Min) return Min;
    if(Value > Max) return Max;
    return Value;
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



v3 RandomVec(f32 Min, f32 Max)
{
    return {RandomInterval(Min,Max), RandomInterval(Min,Max), RandomInterval(Min,Max)};
}

v3 RandomPointOnUnitSphere(void)
{
    v3 Point = {2*Random() - 1, 2*Random() - 1, 2*Random() - 1}; 
    while(((Point.X * Point.X + Point.Y * Point.Y + Point.Z * Point.Z) >= 1))
    {
        
        Point = {2*Random() - 1, 2*Random() - 1, 2*Random() - 1};
        
    }
    

    return Point;
}


void WriteColor(std::ostream& OutStream, v3 Color, u32 SamplesPerPixel)
{

    f32 C = 1.0f/(f32)SamplesPerPixel;



    OutStream << (s32) ( ClampValueBetween(sqrtf(Color.X * C), 0.0f, 0.999f) * 255.999f) << ' '
              << (s32) ( ClampValueBetween(sqrtf(Color.Y * C), 0.0f, 0.999f) * 255.999f) << ' '
              << (s32) ( ClampValueBetween(sqrtf(Color.Z * C), 0.0f, 0.999f) * 255.999f) << '\n';

}


#define Tolerance 0.001f
#define TMax 100000.0f 
#define TMin 0.001f
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

v3 RayCast(ray* Ray, world* World, s32 RecursionDepth) 
{
    if(!RecursionDepth) 
    {
        return {0.0f, 0.0f, 0.0f};
    }

    v3 ResultColor = {};
    hit_info HitInfo = {};
    f32 Tclosest = TMax;
    for(u32 SphereIndex = 0; SphereIndex < World->SphereCount; ++SphereIndex)
    {
        sphere* Sphere = &World->Sphere[SphereIndex];
        f32 T = RayIntersectSphere(Ray, Sphere);
            if( (T > TMin) && (T < Tclosest)) 
            {

                Tclosest = T;

                HitInfo.HitPoint = Ray->Dir*T + Ray->Origin;
                HitInfo.Normal   = (HitInfo.HitPoint - Sphere->Center) / Sphere->R;
                HitInfo.T        =  T;
                HitInfo.RayIsOutward = Inner(HitInfo.Normal, Ray->Dir) > 0;
                if(HitInfo.RayIsOutward) HitInfo.Normal = -HitInfo.Normal;


            }
            
    }


    for(u32 PlaneIndex = 0; PlaneIndex < World->PlaneCount; ++PlaneIndex)
    {
        ;
    }


    if(Tclosest < TMax)
    {
        // should this be for a sphere only?

        v3 Target = HitInfo.HitPoint + HitInfo.Normal + RandomPointOnUnitSphere();
        ray NewRay = {HitInfo.HitPoint, v3{Target - HitInfo.HitPoint}};
        return  0.5f *  RayCast(&NewRay ,World, RecursionDepth-1);
         // return 0.5f * (HitInfo.Normal + 1);
    }
    else
    {

        // ResultColor = RayColor(Ray);
        
        f32 t = 0.5f * (NOZ(Ray->Dir).Y + 1.0f) ;
        return ((1.0f-t)* v3{1.0f, 1.0f, 1.0f}) + (t * v3{0.5f, 0.7f, 1.0f});
       

        // v3 Dir = NOZ(Ray->Dir);
        // f32 t = 0.5f * Dir.Y + 1.0f;
        // return ((1.0f-t)* v3{1.0f, 1.0f, 1.0f}) + (t * v3{0.5f, 0.7f, 1.0f});
    }

    return (ResultColor);
}

int main(void)
{


    //------------- View Port and Camera -------------//

    v3 WorldUpVector = {0.0f, 1.0f, 0.0f};

    //Right Hand Coordinates System, Camera Pointing in the Negative Z-Axis

    camera Camera = {};
    Camera.AspectRatio = 16.0f / 9.0f;
    Camera.Origin = {0.0f, 0.0f, 0.0f};
    Camera.DirZ = NOZ({0.0f ,0.0f , -1.0f});
    Camera.DirX = NOZ(Cross(Camera.DirZ, WorldUpVector));
    Camera.DirY = NOZ(Cross(Camera.DirX, Camera.DirZ));

    s32 ImageWidth  = 400;
    s32 ImageHeight = (s32) ( (f32)ImageWidth / Camera.AspectRatio ) ;

    //----------- View Port ----------//
    film Film   = {};
    Film.H      = 2;
    Film.W      = Film.H * Camera.AspectRatio;
    Film.HalfW  = Film.W * 0.5f ;
    Film.HalfH  = Film.H * 0.5f ;
    
    Film.Dist = 1;
    //---- camera is facing the negative Z Axis ----//


    //--------- Creating WorldObject sphere -------------//
    world World = {};
    sphere Sphere0  = {0.5f, {0.0f, 0.0f, -1.0f}};
    sphere Sphere1  = {100.0f, {0, -100.5f , -1.0f}};
    

    Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere0;

    // Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere1;

    //writing the ppm image header
    std::cout<< "P3\n" << ImageWidth << ' ' << ImageHeight <<"\n255\n";


    u32 SamplesCount = 100;
    for(s32 Y = ImageHeight - 1; Y >= 0; --Y)
    {
        std::cerr << (u32)(100 * (1- ((f32)Y/ (f32)(ImageHeight -1)))) << "%" << ' ' << std::flush;

        for(s32 X = 0; X < ImageWidth; ++X)
        {
            v3 Color = {};
            for( u32 SampleIndex = 0; SampleIndex < SamplesCount; ++SampleIndex)
            {
                Film.Y = (2 * ((f32) Y + Random()) / (f32) (ImageHeight - 1) ) - 1;
                Film.X = (2 * ((f32) X + Random()) / (f32) (ImageWidth  - 1) ) - 1;

                ray Ray = {};
                Ray.Origin = Camera.Origin;
                Ray.Dir = Film.Dist * Camera.DirZ;
                Ray.Dir += Film.Y * Film.HalfH * Camera.DirY;
                Ray.Dir += Film.X * Film.HalfW * Camera.DirX;
                Ray.Dir = NOZ(Ray.Dir);
                //------ super sampling ray tracing----//;
                Color = Color + RayCast(&Ray, &World, 50);
            }            
            
            WriteColor(std::cout, Color, SamplesCount);
        }
    }

    std::cerr << "\n-- Done --\n";

    return(0);
}
