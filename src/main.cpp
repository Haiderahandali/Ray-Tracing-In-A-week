#include "Ray.hpp"


#include <iostream>
#include <cstdio>
#include <fstream>
#include <cmath>





Internal INLINE std::ostream& operator<<(std::ostream& OutStream, v3* V)
{
    return OutStream << V->X << ' ' << V->Y << ' ' << V->Z;
}


#define BREAKPOINT \
asm("int $3")

typedef FILE* image_file;


#define COLOR_VALUE 255
#define EPSILON 0.001f
#define MIN_HIT_DISTANCE 0.001f


#define ImageWidth  256
#define ImageHeight 256

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


#if 0

void ConvertU8ToCString(u8 Num, char* Buffer)
{
    u8 Power = 100;
    while(Power)
    {
        *Buffer++ = '0' + (s8)(Num/Power);
        Num = Num%Power;
        Power /= 10;
    }
    *Buffer = ' '; //space is required to convert the number   
}

void ConvertWidthOrHeightToCString(u32 WOrH, char* Buffer)
{
    Assert(WOrH < 10000); //any thing that is greater than 10K is useless for us
    u32 Power = 1000; 
    while(Power)
    {
        *Buffer++ = '0' + (s8)(WOrH/Power);
        WOrH = WOrH%Power;
        Power /= 10;
    }
    *Buffer = ' '; //space is required to convert the number
}


void WritePPMImageHeader(image_file ImageFile, image* Image)
{

    char const* PPMHeader = "P3\n";
    fwrite(PPMHeader, 1, 3, ImageFile);

    char Buffer[5];
    
    
    ConvertWidthOrHeightToCString(Image->W, Buffer);
    fwrite(Buffer, 1, 5,ImageFile);
    


    
    ConvertWidthOrHeightToCString(Image->H, Buffer);
    fwrite(Buffer, 1, 5,ImageFile);

    
    ConvertU8ToCString((u8)(Image->ColorValue), Buffer);
    Buffer[3] = '\n';
    fwrite(Buffer, 1, 4,ImageFile);
    
}


void WritePPMImage(image_file ImageFile, image* Image)
{
    WritePPMImageHeader(ImageFile, Image);

    u8* Pixels = Image->ColorBuffer;

    for(u32 i = 0; i < Image->H;  ++i)
    {
        for(u32 j = 0; j < Image->W; ++j)
        {
            char Buffer[12];          
            ConvertU8ToCString(*Pixels++, Buffer);
            ConvertU8ToCString(*Pixels++, &Buffer[4]);
            ConvertU8ToCString(*Pixels++, &Buffer[8]);
                // Buffer[12] = 0;
            fwrite(Buffer,1,12,ImageFile);
                // printf("%.*s",12,Buffer);

        }
    }

    char const* ENDOFFILE = "\n";
    fwrite(ENDOFFILE,1,2,ImageFile);

}



void CreatePPMImage(image* Image,u32 Width, u32 Height )
{
    Image->W = Width;
    Image->H = Height;
    Image->BPP = 3;
    Image->ColorValue = COLOR_VALUE;
    Image->ColorBuffer =(u8*)malloc(Image->BPP * Image->W * Image->H);
}


u32 ColorPackV3ToU32(v3 Color)
{
    Color = Color *  255.0f;


    u32 Result = {};
    if(Color.X <0) Color.X = 0;
    if(Color.Y <0) Color.Y = 0;
    if(Color.Z <0) Color.Z = 0;
    Result = (u32) (( ((u8)Color.X) << 24) | ( ((u8)Color.Y) << 16) | ( ((u8)Color.Z) << 8) | 0XFF);
    return(Result);

}

v3 ColorUnPackU32ToV3(u32 Color)
{
    v3 Result = {};
    Result.Z = ((u8)(Color>>8))  ;
    Result.Y = ((u8)(Color>>16)) ;
    Result.X = ((u8)(Color>>24)) ;

    return (Result / 255.0f);
}


void ClearImage(image* Image, u32 Color)

{  
    // u8 A = (u8)(Color);
    u8 B = (u8)(Color>>8);
    u8 G = (u8)(Color>>16);
    u8 R = (u8)(Color>>24);

    u8* Pixels = Image->ColorBuffer;

    for(u32 Row = 0; Row < Image->W; ++Row)
    {
        for(u32 Col = 0; Col < Image->H; ++Col)
        {
            *Pixels++ = R;
            *Pixels++ = G;
            *Pixels++ = B;
        }
    }
}

void WritePPMPixel(u32 X, u32 Y, image* Image, u32 Color)
{
    v3 RGBColor = 255.0f * ColorUnPackU32ToV3(Color);

    u8* Pixels = Image->ColorBuffer;
    u8 BytesPerPixel = Image->BPP;
    u32 Pitch  = BytesPerPixel * Image->W;

    Pixels += (Pitch * Y) + (X * BytesPerPixel);

    *Pixels++ = (u8)RGBColor.X;
    *Pixels++ = (u8)RGBColor.Y;
    *Pixels++ = (u8)RGBColor.Z;
}

#endif


int main(void)
{

    //my way of rendering
    {
    #if 0
    
    
        // image_file RayImageFile = fopen("image.ppm", "wb");
        // image RayImage;
        // CreatePPMImage(&RayImage,512, 512);
        // ClearImage(&RayImage, 0X00FFFFFF);



        // WritePPMImage(RayImageFile, &RayImage);
        // fclose(RayImageFile);   
    #endif
    }

    std::cout<< "P3\n" << ImageWidth << ' ' << ImageHeight <<"\n255\n";
    
    for(s32 Y = ImageHeight - 1; Y >= 0; --Y)
    {
        std::cerr << (u32)(100 * (1- ((f32)Y/ (f32)(ImageHeight -1)))) << "%" << ' ' << std::flush;

        for(s32 X = 0; X < ImageWidth; ++X)
        {
            v3 Color = { (f32) X / (f32) (ImageWidth -1) , (f32) Y / (f32) (ImageHeight - 1) , 0.25f } ;

            WriteColor(std::cout, Color);
        }
    }

    std::cerr << "\n-- Done --\n";

    return(0);
}
