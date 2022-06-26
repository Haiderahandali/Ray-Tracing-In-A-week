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

Internal INLINE f32 ClampValueBetween(f32 Value, f32 Min, f32 Max)
{
    if(Value < Min) return Min;
    if(Value > Max) return Max;
    return Value;
}

Internal INLINE f32 Inner(v3 U, v3 V)
{
    return (U.X * V.X + U.Y * V.Y +  U.Z * V.Z );
}

Internal INLINE v3 NOZ(v3 U)
{
    f32 Domen = sqrtf(Inner(U,U));

    return 
    {
        U.X / Domen,
        U.Y / Domen,
        U.Z / Domen
    };
}

Internal INLINE
f32 MagnitudeSqaured(v3 V)
{
    return (V.X * V.X +   V.Y * V.Y + V.Z * V.Z);
}

Internal INLINE v3 Cross(v3 V1, v3 V2)
{
    v3 Result = {};

    Result.X = (V1.Y * V2.Z) - (V1.Z * V2.Y);
    Result.Y = (V1.Z * V2.X) - (V1.X * V2.Z);
    Result.Z = (V1.X * V2.Y) - (V1.Y * V2.X);
    return (Result);
}

Internal INLINE
v3 Hadamard(v3 V, v3 U)
{
    return 
    {
        V.X * U.X,
        V.Y * U.Y,
        V.Z * U.Z,
    };
}


Internal INLINE 
v3 RandomVec(f32 Min, f32 Max)
{
    return {RandomInterval(Min,Max), RandomInterval(Min,Max), RandomInterval(Min,Max)};
}


Internal INLINE 
v3 RandomPointOnUnitSphere(void)
{
    v3 Point = {2*Random() - 1, 2*Random() - 1, 2*Random() - 1}; 
    while(((Point.X * Point.X + Point.Y * Point.Y + Point.Z * Point.Z) >= 1))
    {

        Point = {2*Random() - 1, 2*Random() - 1, 2*Random() - 1};
        
    }
    return Point;
}

Internal INLINE
v3 RandomUnitVec(void)
{
    return NOZ(RandomPointOnUnitSphere());
}



Internal INLINE v3 RandomPointOnHemisphere(v3* HemiSphereNormal)
{
    v3 UnitVec = RandomUnitVec();
    return Inner(UnitVec, *HemiSphereNormal) > 0 ? UnitVec : -UnitVec;
}


Internal INLINE bool VecNearZero(v3* V)
{
    //Check this later, because it might be different and it doesn't work
    return  ( ( (f64)V->X * (f64)V->X ) + ( (f64)V->Y * (f64)V->Y ) + ( (f64)V->Z * (f64)V->Z) )  <   ((f64)3.0 * EPSILON64);
    // return (fabsf(V->X) < EPSILON) && (fabsf(V->Y) < E)
}

Internal INLINE
v3 VecReflectOverNormal(v3 Vec, v3 Normal)
{
    return Vec - (2*Inner(Vec, Normal) * Normal);
}

Internal INLINE

v3 Refract(v3 RayDir, v3 Normal, f32 RelativeRefractedIndex) 
{

    f32 CosAngle  = fminf(Inner(-RayDir, Normal), 1.0f);
    //caluclate the output ray as a sum of a perpendicular to the surface normal vector + a parallel to the surface normal vector

    v3 PerpendicularToNormal =  RelativeRefractedIndex * (RayDir + (CosAngle * Normal));

    v3 ParallelToNormal      = -sqrtf(fabsf(1.0f - MagnitudeSqaured(PerpendicularToNormal))) * Normal;
    return (PerpendicularToNormal + ParallelToNormal);
}
Internal INLINE
f32 Reflectance(f32 CosAngle, f32 RelativeRefractedIndex) 
{
    // Use Schlick's approximation for reflectance.
    f32 R0 = (1 - RelativeRefractedIndex) / (1+ RelativeRefractedIndex);
    R0 = R0*R0;
    return R0 + (1 - R0) * powf((1 - CosAngle),5);
}


Internal INLINE void WriteColor(std::ostream& OutStream, v3 Color, u32 SamplesPerPixel)
{

    f32 C = 1.0f/(f32)SamplesPerPixel;

    OutStream << (s32) ( ClampValueBetween(sqrtf(Color.X * C), 0.0f, 0.999f) * 255.999f) << ' '
    << (s32) ( ClampValueBetween(sqrtf(Color.Y * C), 0.0f, 0.999f) * 255.999f) << ' '
    << (s32) ( ClampValueBetween(sqrtf(Color.Z * C), 0.0f, 0.999f) * 255.999f) << '\n';

}




#define TMax 100000.0f 
#define TMin 0.001f

Internal INLINE f32 RayIntersectSphere(ray* Ray, sphere* Sphere)
{
    f32 T = TMax;

    v3 X = Ray->Origin - Sphere->Center;

    f32 B = Inner(X, NOZ(Ray->Dir));
    f32 C = Inner(X,X) - (Sphere->R * Sphere->R);


    f32 Disc = B*B - C;

    if(Disc < 0.0f) 
    {
        return T;
    }
    T = -B - sqrtf(Disc);
    if (T < TMin)
    {
        T = -B + sqrtf(Disc);
    }

    return T;
}

Internal INLINE 
void ScatterDiffuse(material* MaterialPtr, hit_info* HitInfo, v3* Attentuation, ray* ScatteredRay )
{
    v3 ScatterDir = HitInfo->Normal + RandomUnitVec();

    if(VecNearZero(&ScatterDir)) 
    {
        ScatterDir = HitInfo->Normal;
    }

    *ScatteredRay = ray {HitInfo->HitPoint, ScatterDir};
    *Attentuation = MaterialPtr->AlbedoColor;
}

Internal INLINE
void ScatterDielectric(material* MaterialPtr, ray* Ray, hit_info* HitInfo, ray* ScatteredRay)
{

    f32 RelativeRefractedIndex = !HitInfo->RayIsOutward? (1.0f /MaterialPtr->RefractedIndex) : MaterialPtr->RefractedIndex;
    v3 NOZDir = NOZ(Ray->Dir); 

    f32 CosAngle = Inner(-NOZDir, HitInfo->Normal);
    f32 SinAngle = sqrtf( 1 - (CosAngle * CosAngle));


    v3 RefractedRay;
    if((SinAngle * RelativeRefractedIndex > 1.0f) || Reflectance(CosAngle, RelativeRefractedIndex) > Random())
    {
    //Total Internal Refraction case
        RefractedRay= VecReflectOverNormal(NOZDir, HitInfo->Normal);
    }
    else
    {
    //Normal Refraction Case
        RefractedRay= Refract(NOZDir, HitInfo->Normal, RelativeRefractedIndex);
    }

    *ScatteredRay = ray{HitInfo->HitPoint, RefractedRay};
}

Internal INLINE 
b32 ScatterMetalic(material* MaterialPtr, ray* Ray, hit_info* HitInfo, v3* Attentuation, ray* ScatteredRay )
{
    v3 ReflectedRay = VecReflectOverNormal(NOZ(Ray->Dir), HitInfo->Normal);
    *ScatteredRay   = {HitInfo->HitPoint ,  (MaterialPtr->Fuzz * RandomUnitVec()) + ReflectedRay};
    *Attentuation   = MaterialPtr->AlbedoColor;

    return (Inner(ScatteredRay->Dir, HitInfo->Normal) > 0);
}


Internal INLINE v3 RayColor(ray* Ray) 
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
    u32 MaterialIndex = 0;

    // v3 ResultColor = {};
    hit_info HitInfo = {};
    f32 Tclosest = TMax;
    for(u32 SphereIndex = 0; SphereIndex < World->SphereCount; ++SphereIndex)
    {
        sphere* Sphere = &World->Sphere[SphereIndex];
        f32 T = RayIntersectSphere(Ray, Sphere);
        if( (T > TMin) && (T < Tclosest)) 
        {

            Tclosest = T;

            HitInfo.MaterialPtr  = &World->Material[Sphere->MaterialIndex]; 
            HitInfo.HitPoint     = NOZ(Ray->Dir)*T + Ray->Origin;
            HitInfo.Normal       = (HitInfo.HitPoint - Sphere->Center) / Sphere->R;
            HitInfo.T            =  T;
            HitInfo.RayIsOutward = Inner(HitInfo.Normal, NOZ(Ray->Dir)) > 0;
            if(HitInfo.RayIsOutward)
            {
                HitInfo.Normal = -HitInfo.Normal;  
            } 
            MaterialIndex = Sphere->MaterialIndex;
        }
    }


    switch (MaterialIndex)
    {

        case 1: //diffuse mateiral
        case 2: 
        {
            v3 Attentuation;
            ray ScatteredRay;
            ScatterDiffuse(HitInfo.MaterialPtr, &HitInfo, &Attentuation, &ScatteredRay);
            return Hadamard(Attentuation , RayCast(&ScatteredRay, World, RecursionDepth-1)); 

        } break;    


        case 3: //Dielectric Material 
        {
            ray ScatteredRay;
            ScatterDielectric(HitInfo.MaterialPtr, Ray, &HitInfo, &ScatteredRay);
            return Hadamard(v3{1.0f, 1.0f, 1.0f}, RayCast(&ScatteredRay, World, RecursionDepth-1)) ; 

        } break;

        case 4: // Metalic Material
        {
            v3 Attentuation;
            ray ScatteredRay;

            if(ScatterMetalic(HitInfo.MaterialPtr, Ray, &HitInfo, &Attentuation, &ScatteredRay))
            {
                return Hadamard(Attentuation, RayCast(&ScatteredRay, World, RecursionDepth-1)); 
            }
            return {0.0f, 0.0f, 0.0f};

        } break;

        case 0:
        default:
        {
            f32 t = 0.5f * (NOZ(Ray->Dir).Y + 1.0f) ;
            return ((1.0f-t)* v3{1.0f, 1.0f, 1.0f}) + (t * v3{0.5f, 0.7f, 1.0f});
        } break;
    }
    // if(Tclosest < TMax)
    // {
    //     // should this be for a sphere only?

    //     v3 Target = HitInfo.HitPoint  + HitInfo.Normal + NOZ(RandomPointOnUnitSphere());
    //     ray NewRay = {HitInfo.HitPoint, v3{Target - HitInfo.HitPoint}};
    //     return  0.5f *  RayCast(&NewRay ,World, RecursionDepth-1);
    //      // return 0.5f * (HitInfo.Normal + 1);
    // }
    // else
    // {
    //     // ResultColor = RayColor(Ray);
    //     f32 t = 0.5f * (NOZ(Ray->Dir).Y + 1.0f) ;
    //     return ((1.0f-t)* v3{1.0f, 1.0f, 1.0f}) + (t * v3{0.5f, 0.7f, 1.0f});
    // }
}

int main(void)
{

    //------------- View Port and Camera -------------//

    v3 WorldUpVector = {0.0f, 1.0f, 0.0f};

    //Right Hand Coordinates System, Camera Pointing in the Negative Z-Axis
    camera Camera = {};
    Camera.AspectRatio = 16.0f / 9.0f;
    Camera.VFOV = DegreeToRad(90.0f);

    Camera.Origin = {-0.0f, 0.0f, 0.0f};
    v3 LookAt = {0.0f ,0.0f, -1.0f};

    Camera.DirZ = NOZ(LookAt - Camera.Origin);
    Camera.DirX = NOZ(Cross(Camera.DirZ, WorldUpVector));
    Camera.DirY = NOZ(Cross(Camera.DirX, Camera.DirZ));

    s32 ImageWidth  = 400;
    s32 ImageHeight = (s32) ( (f32)ImageWidth / Camera.AspectRatio ) ;

    //----------- View Port ----------//
    f32 H       = tanf(Camera.VFOV/2.0f);
    film Film   = {};
    Film.H      = 2 * H;
    Film.W      = Film.H * Camera.AspectRatio;
    Film.HalfW  = Film.W * 0.5f ;
    Film.HalfH  = Film.H * 0.5f ;
    
    Film.Dist = 1;
    //---- camera is facing the negative Z Axis ----//


    //--------- Creating WorldObject sphere -------------//
    world World = {};    


    
    material MaterialBackGround = {{0.0f, 0.0f, 0.0f}, 1.0f, 1.0f};

    material MaterialGround = {{0.8f, 0.8f, 0.0f}, 1.0f, 1.0f};
    material MaterialCenter = {{0.1f, 0.2f, 0.5f}, 1.0f, 1.0f}; 
    material MaterialLeft   = {{0.8f, 0.8f, 0.8f}, 1.5f, 0.3f};
    material MaterialRight  = {{0.8f, 0.6f, 0.2f}, 1.0f, 1.0f};

    sphere Sphere1 {{ 0.0f, -100.5f, -1.0f}, 100.0f, 1};
    sphere Sphere2 {{ 0.0f,    0.0f, -1.0f},   0.5f, 2};
    sphere Sphere3 {{-1.0f,    0.0f, -1.0f},   0.5f, 3};
    sphere Sphere4 {{-1.0f,    0.0f, -1.0f},   0.4f, 3};
    sphere Sphere5 {{ 1.0f,    0.0f, -1.0f},   0.5f, 4};

    /*Material Index from 1-2 are diffuse, 3-4 are metailic, 5 is*/

    
    Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere1;

    Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere2;    

    Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere3;    

    Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere4;    

    Assert(World.SphereCount < MAX_SPHERE_COUNT);
    World.Sphere[World.SphereCount++] = Sphere5;    



    World.Material[0] = MaterialBackGround;     World.MaterialCount++;
    World.Material[1] = MaterialGround;         World.MaterialCount++;
    World.Material[2] = MaterialCenter;         World.MaterialCount++;
    World.Material[3] = MaterialLeft;           World.MaterialCount++;
    World.Material[4] = MaterialRight;          World.MaterialCount++;


     

    //writing the ppm image header
    std::cout<< "P3\n" << ImageWidth << ' ' << ImageHeight <<"\n255\n";


    u32 SamplesCount = 100;
    for(s32 Y = ImageHeight - 1; Y >= 0; --Y)
    {
        // if(Y & 0X20)  //every 32 iteration
        //     {
        //         std::cerr << "\rFinished: "<<(u32)(100 * (1- ((f32)Y/ (f32)(ImageHeight -1)))) << "%" << ' ' << std::flush;
        //     }
        std::cerr << "\rScanlines remaining: " << Y << ' ' << std::flush;

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
                Ray.Dir = Ray.Dir;
                // Ray.Dir = NOZ(Ray.Dir);
                //------ super sampling ray tracing----//;
                Color = Color + RayCast(&Ray, &World, 50);
            }            
            
            WriteColor(std::cout, Color, SamplesCount);
        }
    }

    std::cerr << "\n-- Done --\n";

    return(0);
}
