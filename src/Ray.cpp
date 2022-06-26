#include "Ray.hpp"

#include <iostream>
#include <fstream>
#include <cmath>

#define BREAKPOINT asm("int $3")

Internal INLINE std::ostream& operator<<(std::ostream& OutStream, v3* V)
{
    return OutStream << V->X << ' ' << V->Y << ' ' << V->Z;
}

Internal INLINE 
f32 ClampValueBetween(f32 Value, f32 Min, f32 Max)
{
    if(Value < Min) return Min;
    if(Value > Max) return Max;
    return Value;
}

Internal INLINE 
f32 Inner(v3 U, v3 V)
{
    return (U.X * V.X + U.Y * V.Y +  U.Z * V.Z );
}


Internal INLINE
f32 MagnitudeSqaured(v3 V)
{
    return (V.X * V.X +   V.Y * V.Y + V.Z * V.Z);
}

Internal INLINE 
f32 Vec3Length(v3 V)
{
    return sqrtf(MagnitudeSqaured(V));
}


Internal INLINE 
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


Internal INLINE 
v3 Cross(v3 V1, v3 V2)
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

//returns a number [0 , 1)
Internal INLINE 
f32 Random(void)
{
    return  (f32)std::rand() / ((f32)RAND_MAX + 1.0f);
}


//returns a number between [IntervalMin , IntervalMax)
Internal INLINE 
f32 RandomInterval(f32 IntervalMin, f32 IntervalMax)
{
    return (IntervalMin + (IntervalMax - IntervalMin) * Random());
}   

Internal INLINE 
v3 RandomVecBetweenInterval(f32 Min, f32 Max)
{
    return {RandomInterval(Min,Max), RandomInterval(Min,Max), RandomInterval(Min,Max)};
}

Internal INLINE 
v3 RandomVec(void)
{
    return {Random(), Random(),Random()};
}


Internal INLINE 
v3 RandomPointOnUnitSphere(void)
{
    v3 Point = {2*Random() - 1, 2*Random() - 1, 2*Random() - 1}; //this givs a random point in a cube with sides from -1 to 1
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

Internal INLINE 
v3  RandomPointOnHemisphere(v3* HemiSphereNormal)
{
    v3 UnitVec = RandomUnitVec();
    return Inner(UnitVec, *HemiSphereNormal) > 0 ? UnitVec : -UnitVec;
}

Internal INLINE 
v3 RandomOnUnitDisk(void)
{
    v3 Result = {};
    do
    {
        Result = v3{RandomInterval(-1,1), RandomInterval(-1,1), 0.0f};

    } while(MagnitudeSqaured(Result) >= 1);
    return Result;
}

Internal INLINE 
b32 VecNearZero(v3* V)
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

Internal INLINE 
void WriteColor(std::ostream& OutStream, v3 Color, u32 SamplesPerPixel)
{

    f32 C = 1.0f/(f32)SamplesPerPixel;

    OutStream << (s32) ( ClampValueBetween(sqrtf(Color.X * C), 0.0f, 0.999f) * 255.999f) << ' '
    << (s32) ( ClampValueBetween(sqrtf(Color.Y * C), 0.0f, 0.999f) * 255.999f) << ' '
    << (s32) ( ClampValueBetween(sqrtf(Color.Z * C), 0.0f, 0.999f) * 255.999f) << '\n';

}


#define TMax 100000.0f 
#define TMin 0.001f

Internal INLINE 
f32 RayIntersectSphere(ray* Ray, sphere* Sphere)
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


Internal INLINE 
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
    u32 MaterialIndex = 0;

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


    switch (World->Material[MaterialIndex].Type)
    {
        case MaterialDiffuse:
        {
            v3 Attentuation;
            ray ScatteredRay;
            ScatterDiffuse(HitInfo.MaterialPtr, &HitInfo, &Attentuation, &ScatteredRay);
            return Hadamard(Attentuation , RayCast(&ScatteredRay, World, RecursionDepth-1)); 

        } break;    


        case MaterialDielectric:
        {
            ray ScatteredRay;
            ScatterDielectric(HitInfo.MaterialPtr, Ray, &HitInfo, &ScatteredRay);
            return Hadamard(v3{1.0f, 1.0f, 1.0f}, RayCast(&ScatteredRay, World, RecursionDepth-1)) ; 

        } break;

        case MaterialMetal: 
        {
            v3 Attentuation;
            ray ScatteredRay;

            if(ScatterMetalic(HitInfo.MaterialPtr, Ray, &HitInfo, &Attentuation, &ScatteredRay))
            {
                return Hadamard(Attentuation, RayCast(&ScatteredRay, World, RecursionDepth-1)); 
            }
            return {0.0f, 0.0f, 0.0f};

        } break;

        default:
        {
            //-----Back Ground Color-------//
            f32 t = 0.5f * (NOZ(Ray->Dir).Y + 1.0f) ;
            return ((1.0f-t)* v3{1.0f, 1.0f, 1.0f}) + (t * v3{0.5f, 0.7f, 1.0f});
        } break;
    }
}

world CreateWorld(void)
{

    world World = {};
    material MaterialBackGround = {{0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, BackGround};
    World.Material[0] = MaterialBackGround;


    material MaterialGround = {{0.5f, 0.5f, 0.5f}, 1.0f, 0.0f, MaterialDiffuse };
    World.Material[1] = MaterialGround;
    sphere Sphere1          = {{0.0f, -1000.0f, 0.0f}, 1000.0f, 1};
    World.Sphere[World.SphereCount++] = Sphere1;

    u32 SphereIndex = 1; // World.MaterialCount = 2
    for (int a = -11; a < 11; a++) 
    {
        for (int b = -11; b < 11; b++) 
        {
            f32 Mat = Random(); // material randomly
            v3 Center = {a + 0.9f * Random(), 0.2f, b + 0.9f * Random()};

            if ( Vec3Length(Center -  v3{4.0f, 0.2f, 0.0f}) > 0.9f) 
            {
                material SphereMaterial;

                if (Mat < 0.8) 
                {
                    // diffuse
                    v3 Albedo = Hadamard(RandomVec() , RandomVec());
                    SphereMaterial = material{Albedo, 1.0f, 0.0f, MaterialDiffuse};
                    World.Material[SphereIndex] = SphereMaterial;
                    World.Sphere[World.SphereCount++] = {Center, 0.2f, SphereIndex};
                    ++SphereIndex; 
                } 
                else if (Mat < 0.95) 
                {
                    // metal
                    v3 Albedo = RandomVecBetweenInterval(0.5f, 1.0f);
                    f32 Fuzz = RandomInterval(0.0f, 0.5f);
                    SphereMaterial = material{Albedo, 1.0f, Fuzz, MaterialMetal};

                    World.Material[SphereIndex] = SphereMaterial;
                    World.Sphere[World.SphereCount++] = {Center, 0.2f, SphereIndex};
                    ++SphereIndex; 
                    
                } 
                else 
                {
                    // glass
                    SphereMaterial = {{1.0f, 1.0f, 1.0f}, 1.5f, 0.0f, MaterialDielectric};

                    World.Material[SphereIndex] = SphereMaterial;
                    World.Sphere[World.SphereCount++] = {Center, 0.2f, SphereIndex};
                    ++SphereIndex; 
                }
            }
        }
    }



    material Material1 = {{1.0f, 1.0f, 1.0f}, 1.5f, 1.0f, MaterialDielectric};
    World.Material[SphereIndex] = Material1;
    World.Sphere[World.SphereCount++] = {{0.0f, 1.0f, 0.0f}, 1.0f, SphereIndex};
    ++SphereIndex; 
    

    material Material2 = {{0.4f, 0.2f, 0.1f}, 1.0f, 1.0f, MaterialDiffuse};
    World.Material[SphereIndex] = Material2;
    World.Sphere[World.SphereCount++] = {{-4.0f, 1.0f, 0.0f}, 1.0f, SphereIndex};
    ++SphereIndex; 

    material Material3 = {{0.7f, 0.6f, 0.5f}, 1.0f, 0.0f, MaterialMetal};
    World.Material[SphereIndex] = Material3;
    World.Sphere[World.SphereCount++] = {{4.0f, 1.0f, 0.0f}, 1.0, SphereIndex};

    
    return World;
}


int main(void)
    {

    //-----------Creating The World---------//
    world World = CreateWorld();


    //Right Hand Coordinates System, Camera Pointing in the Negative Z-Axis
    //------------- View Port and Camera -------------//
    v3 WorldUpVector = {0.0f, 1.0f, 0.0f};

    camera Camera = {};
    Camera.AspectRatio = 3.0f / 2.0f;
    Camera.Aperture = 0.1f;
    Camera.VFOV = DegreeToRad(20.0f); 
    
    Camera.LensRadius = (Camera.Aperture/2.0f);
    
    
    v3 LookAt = {0.0f, 0.0f, 0.0f};
    Camera.Origin = {13.0f, 2.0f, 3.0f};
    Camera.DirZ = NOZ(LookAt - Camera.Origin);
    Camera.DirX = NOZ(Cross(Camera.DirZ, WorldUpVector));
    Camera.DirY = NOZ(Cross(Camera.DirX, Camera.DirZ));

    Camera.FocusDist = 10.0f;

    //----------- View Port ----------//
    f32 H       = tanf(Camera.VFOV/2.0f);
    film Film   = {};
    Film.H      = 2 * H;
    Film.W      = Film.H * Camera.AspectRatio;
    Film.HalfW  = Film.W * 0.5f ;
    Film.HalfH  = Film.H * 0.5f ;
    
    Film.Dist = 1;


//------------- Image ------------//
    s32 ImageWidth  = 1200;
    s32 ImageHeight = (s32) ( (f32)ImageWidth / Camera.AspectRatio ) ;

    //writing the ppm image header
    std::cout<< "P3\n" << ImageWidth << ' ' << ImageHeight <<"\n255\n";

    u32 SamplesCount = 500;
    for(s32 Y = ImageHeight - 1; Y >= 0; --Y)
    {
        std::cerr << "\rScanlines remaining: " << Y << ' ' << std::flush;

        for(s32 X = 0; X < ImageWidth; ++X)
        {
            v3 Color = {};
            for( u32 SampleIndex = 0; SampleIndex < SamplesCount; ++SampleIndex)
            {
                Film.Y = (2 * ((f32) Y + Random()) / (f32) (ImageHeight - 1) ) - 1;
                Film.X = (2 * ((f32) X + Random()) / (f32) (ImageWidth  - 1) ) - 1;

                ray Ray = {};
                v3 RandomOnLens  = Camera.LensRadius * RandomOnUnitDisk(); // a point on the camera lens, Lens Radius = Aperture/2
                v3 RayOffset     = (RandomOnLens.X * Camera.DirX) + (RandomOnLens.Y * Camera.DirY);
                Ray.Origin       = Camera.Origin + RayOffset;

                Ray.Dir = Camera.FocusDist * Camera.DirZ;
                Ray.Dir += Film.Y * Film.HalfH * Camera.DirY * Camera.FocusDist;
                Ray.Dir += Film.X * Film.HalfW * Camera.DirX * Camera.FocusDist;
                Ray.Dir = Ray.Dir - RayOffset;

                Color = Color + RayCast(&Ray, &World, 50);
            }            
            
            WriteColor(std::cout, Color, SamplesCount);
        }
    }

    std::cerr << "\n-- Done --\n";

    return(0);
}
