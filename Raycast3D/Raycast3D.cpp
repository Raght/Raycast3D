#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <iostream>
#include <string>
#include <math.h>
using namespace std;

const double const PI = 3.141592653589793238462643383279;
const int const FORWARD = 0;
const int const RIGHT = 1;
const int const BACK = 2;
const int const LEFT = 3;


struct Tile
{
    bool isSolid = false;
    float height = 1.0f;
};

struct Rect
{
    olc::Pixel pColor;
    float fHeight;
};

struct Map
{
    string tiles;
    int width, height;
};



class Engine : public olc::PixelGameEngine
{
public:
    Engine()
    {
        sAppName = "Raycast3D Demo";
    }

private:
    float Cap(float n, float a, float b)
    {
        if (n < a)
            return a;
        else if (n > b)
            return b;
        return n;
    }

    int Cap(int n, int a, int b)
    {
        if (n < a)
            return a;
        else if (n > b)
            return b;
        return n;
    }

    bool PointVsRect(olc::vf2d point, olc::vf2d rectangle_position, olc::vf2d rectangle_size)
    {
        return (rectangle_position.x <= point.x && point.x <= rectangle_position.x + rectangle_size.x &&
                rectangle_position.y <= point.y && point.y <= rectangle_position.y + rectangle_size.y);
    }

    bool PointVsTile(olc::vf2d point, olc::vi2d tile)
    {
        return PointVsRect(point, tile, olc::vf2d(1.0f, 1.0f));
    }

    float Radians(float eulerAngle)
    {
        return PI * eulerAngle / 180.0f;
    }

    float Degrees(float radians)
    {
        return radians / PI * 180.0f;
    }

    float AngleBetween(olc::vf2d v1, olc::vf2d v2)
    {
        return acosf((v1.dot(v2)) / (v1.mag() * v2.mag()));
    }

    olc::vf2d Rotate(olc::vf2d v, float eulerAngle)
    {
        olc::vf2d vRotated = v;

        vRotated.x = v.x * cosf(Radians(eulerAngle)) - v.y * sinf(Radians(eulerAngle));
        vRotated.y = v.x * sinf(Radians(eulerAngle)) + v.y * cosf(Radians(eulerAngle));

        return vRotated;
    }

    int iMapWidth;
    int iMapHeight;
    float fMapWidth;
    float fMapHeight;

    float* heightMap;

    Map map;
    Map map1 = { "########"
                 ".......#"
                 "#...##.#"
                 "#..##..#"
                 "#......#"
                 "#.#.#..#"
                 "#...#..#"
                 "########", 8, 8};

    Map map2 = { "################"
                 "################"
                 "..............##"
                 "..............##"
                 "##......####..##"
                 "##......####..##"
                 "##....####....##"
                 "##....####....##"
                 "##............##"
                 "##............##"
                 "##..##..##....##"
                 "##..##..##....##"
                 "##......##....##"
                 "##......##....##"
                 "################"
                 "################", 16, 16 };

    Map map3 = { "########"
                 "#......#"
                 "#......#"
                 "#......#"
                 "#......#"
                 "#......#"
                 "#......#"
                 "########", 8, 8 };

    Map map4 = { "########"
                 "#......#"
                 "#......#"
                 "#..##..#"
                 "#..##..#"
                 "#......#"
                 "#......#"
                 "########", 8, 8 };


    float fWallHeight = 3.6f;

    olc::vf2d vMoveDirection = { 0.0f, 0.0f };
    float fPlayerSpeed = 2.0f;
    float fPlayerSprintSpeed = 2.5f;

    float fHitPointDistance;
    float fDrawDistance;
    olc::vf2d vCameraDirection = { 1.0f, 0.0f };
    olc::vf2d vCameraLeftRayDirection;

    olc::vf2d vPlayerPos;

    bool bDrawMap = true;

    //float fNearZ = 0.5f;
    float fNearZ;

    float fCameraHeight = 1.8f;
    float fHFovAngle = 90.0f;
    float fHFovHalf = fHFovAngle / 2.0f;
    
    float fRotationSpeed = 120.0f;

    bool debugMode = false;


    

    bool OutOfBounds(olc::vf2d point)
    {
        return !PointVsRect(point, { 0.0f, 0.0f }, { fMapWidth, fMapHeight });
    }

    Rect ShootRay(olc::vi2d vPixelPos)
    {
        //float fAngle = float(vPixelPos.x) / ScreenWidth() * fHFovAngle;
        //if (fAngle > fHFovHalf)
        //{
        //    fAngle -= fHFovHalf;
        //    fAngle = -fAngle;
        //}
        //else
        //    fAngle = abs(fAngle - fHFovHalf);
        //float fAngle = float(vPixelPos.x) / ScreenWidth() * fHFovAngle - fHFovAngle / 2;
        float fAngle = atanf((vPixelPos.x - ScreenWidth() / 2) / fNearZ);
        olc::vf2d vRayDirection = Rotate(vCameraDirection, Degrees(fAngle)).norm();

        bool bHitSuccessful = false;
        float fRayTravelledDistance = 0.0f;

        olc::vf2d vPoint;
        olc::vi2d vTilePos;
        float distance_step = 0.025f;
        olc::Pixel pColor = olc::WHITE;
        float height = 0;
        fRayTravelledDistance = 0;
        for (; fRayTravelledDistance <= fDrawDistance; fRayTravelledDistance += distance_step)
        {
            vPoint = vPlayerPos + vRayDirection * float(fRayTravelledDistance);
            if (OutOfBounds(vPoint))
            {
                bHitSuccessful = true;
                break;
            }
            vTilePos = { int(Cap(vPoint.x, 0.0f, fMapWidth - 1)), int(Cap(vPoint.y, 0.0f, fMapHeight - 1)) };

            if (map.tiles[vTilePos.y * iMapWidth + vTilePos.x] == '#')
            {
                bHitSuccessful = true;
                break;
            }
        }

        fHitPointDistance = fRayTravelledDistance;
        fHitPointDistance *= cosf(fAngle);

        if (OutOfBounds(vPoint))
        {
            pColor = olc::BLACK;
            height = fWallHeight;
        }
        else if (bHitSuccessful)
        {
            pColor = olc::WHITE;
            float fBrightnessLoss = fRayTravelledDistance / fDrawDistance;
            for (int i = 0; i < 3; i++)
                pColor[i] -= 255 * fBrightnessLoss;
            
            height = heightMap[vTilePos.y * iMapWidth + vTilePos.x];
        }
        else
        {
            pColor = olc::BLACK;
            height = 0.0f;
        }

        return Rect{ pColor, height };
    }


public:
    bool OnUserCreate() override
    {
        fNearZ = ScreenWidth() / 2 / tanf(Radians(fHFovHalf));

        map = map2;

        iMapWidth = map.width;
        iMapHeight = map.height;
        fMapWidth = (float)iMapWidth;
        fMapHeight = (float)iMapHeight;

        vPlayerPos = { 2.0f, 4.0f };
        fDrawDistance = max(fMapWidth, fMapHeight);

        vCameraDirection = vCameraDirection.norm();

        heightMap = new float[iMapWidth * iMapHeight]
        { 1,    1.5f, 1.5f, 1.5f, 1.25f, 1,  0.75f, 0.75f,
          1,    1,    1,    1,    1,     1,  1,     0.5f, 
          2,    1,    1,    1,    1,     1,  1,     1,   
          2.5f, 1,    1,    1,    1,     1,  1,     1,   
          1,    1,    1,    1,    1,     1,  1,     1,   
          1,    1,    1,    1,    1,     1,  1,     1,   
          1,    1,    1,    1,    1,     1,  1,     1,   
          1,    1,    1,    1,    1,     1,  1,     1    };

        for (int j = 0; j < iMapHeight; j++)
        {
            for (int i = 0; i < iMapWidth; i++)
            {
                heightMap[j * iMapWidth + i] = fWallHeight;
            }
        }
        //for (int j = 0; j < iMapHeight; j += 4)
        //{
        //    for (int i = 1; i < iMapWidth - 2; i++)
        //    {
        //        heightMap[j * iMapWidth + i] = fWallHeight * 1.5f;
        //    }
        //    for (int i = iMapWidth; i < iMapWidth; i++)
        //    {
        //        heightMap[j * iMapWidth + i] = fWallHeight * 0.75f;
        //    }
        //}
        heightMap[3 * 8 + 3] = fCameraHeight * 3;

        

        return true;
    }

    bool OnUserDestroy() override
    {
        delete[] heightMap;

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::BLACK);

        if (GetKey(olc::LEFT).bHeld)
            vCameraDirection = Rotate(vCameraDirection, -fRotationSpeed * fElapsedTime);
        if (GetKey(olc::RIGHT).bHeld)
            vCameraDirection = Rotate(vCameraDirection, fRotationSpeed * fElapsedTime);

        vMoveDirection = { 0.0f, 0.0f };
        if (GetKey(olc::W).bHeld)
            vMoveDirection += vCameraDirection;
        if (GetKey(olc::D).bHeld)
            vMoveDirection += Rotate(vCameraDirection, 90.0f);
        if (GetKey(olc::S).bHeld)
            vMoveDirection -= vCameraDirection;
        if (GetKey(olc::A).bHeld)
            vMoveDirection += Rotate(vCameraDirection, -90.0f);
        vMoveDirection = vMoveDirection.norm();

        vPlayerPos = { Cap(vPlayerPos.x, 0.0f, fMapWidth), Cap(vPlayerPos.y, 0.0f, fMapHeight) };

        olc::vf2d move = vMoveDirection * fElapsedTime;
        if (GetKey(olc::SHIFT).bHeld)
            move *= fPlayerSprintSpeed;
        else
            move *= fPlayerSpeed;
        olc::vf2d vTempPos = vPlayerPos + move;
        olc::vi2d vTilePos = { int(vTempPos.x), int(vTempPos.y) };
        if (!(map.tiles[vTilePos.y * iMapWidth + vTilePos.x] == '#'))
        {
            vPlayerPos = vTempPos;
        }


        

        // Draw floor
        float fLightIntensityBegin = 1.0f;
        float fLightIntensityEnd = 0.25f;
        float fLightIntensity = fLightIntensityBegin;
        float fLightIntensitySpan = fLightIntensityBegin - fLightIntensityEnd;
        float fFloorYBegin = ScreenHeight() - 1;
        float fFloorYEnd = ScreenHeight() / 2;
        for (int y = fFloorYBegin; y > fFloorYEnd; y--)
        {
            //float fLightIntensity = (y - ScreenHeight() / 2) / float(ScreenHeight() / 2);
            int dy = fFloorYBegin - y;

            float fLightIntensity = fLightIntensityBegin - dy / float(fFloorYEnd) * fLightIntensitySpan;

            olc::Pixel p = olc::WHITE;
            for (int i = 0; i < 3; i++)
                p[i] *= fLightIntensity;

            DrawLine(olc::vi2d{ 0, y }, olc::vi2d{ScreenWidth() - 1, y}, p);
        }


        // Raycasting
        for (int x = 0; x < ScreenWidth(); x++)
        {
            Rect rect = ShootRay(olc::vi2d{ x, ScreenHeight() / 2 });

            if (rect.fHeight == 0.0f)
            {
                continue;
            }

            //float scale = 1 - fHitPointDistance / fDrawDistance;
            //int size = ScreenHeight() * scale;

            float yWorldLower = 0;
            float yWorldHigher = rect.fHeight;

            float dyCameraToLower = fCameraHeight - yWorldLower;
            float dyCameraToHigher = yWorldHigher - fCameraHeight;

            //int dyLower = roundf(dyCameraToLower * fNearZ / fHitPointDistance);
            //int dyHigher = roundf(dyCameraToHigher * fNearZ / fHitPointDistance);

            int dyLower = roundf(dyCameraToLower * fNearZ / fHitPointDistance);
            int dyHigher = roundf(dyCameraToHigher * fNearZ / fHitPointDistance);

            //int constant = 10;
            //dyLower *= constant;
            //dyHigher *= constant;

            int yCamera = ScreenHeight() / 2;
            int yLower = yCamera + dyLower;
            int yHigher = yCamera - dyHigher;
            yLower = Cap(yLower, 0, ScreenHeight() - 1);
            yHigher = Cap(yHigher, 0, ScreenHeight() - 1);
            
            //float height_projected = height * z / fHitPointDistance;
            //
            //int yLower = (ScreenHeight() + size) / 2;
            //int yHigher = ScreenHeight() / 2 - size * (rect.fHeight - 0.5f);

            DrawLine(olc::vi2d{ x, yLower }, olc::vi2d{ x, yHigher }, rect.pColor);
        }

        
        if (bDrawMap)
        {
            // Draw map
            for (int y = 0; y < iMapHeight; y++)
                for (int x = 0; x < iMapWidth; x++)
                    DrawChar(olc::vi2d{ x * 8, y * 8 }, map.tiles[y * iMapWidth + x], olc::RED, 1U);
        
            // Draw Player position
            FillCircle(olc::vi2d{ int(vPlayerPos.x * 8) , int(vPlayerPos.y * 8) }, 2, olc::CYAN);

            olc::vf2d vStraightSightRay = vCameraDirection * fDrawDistance;
            olc::vf2d vLeftRay = Rotate(vStraightSightRay, fHFovHalf);
            olc::vf2d vRightRay = Rotate(vStraightSightRay, -fHFovHalf);

            // Draw viewing frustrum on the map
            DrawLine(olc::vi2d{ int(vPlayerPos.x * 8), int(vPlayerPos.y * 8) },
                     olc::vi2d{ int((vStraightSightRay * 8 + vPlayerPos * 8).x), int((vStraightSightRay * 8 + vPlayerPos * 8).y) },
                     olc::DARK_BLUE);
            DrawLine(olc::vi2d{ int(vPlayerPos.x * 8), int(vPlayerPos.y * 8) },
                     olc::vi2d{ int((vLeftRay * 8 + vPlayerPos * 8).x), int((vLeftRay * 8 + vPlayerPos * 8).y) },
                     olc::BLUE);
            DrawLine(olc::vi2d{ int(vPlayerPos.x * 8), int(vPlayerPos.y * 8) },
                     olc::vi2d{ int((vRightRay * 8 + vPlayerPos * 8).x), int((vRightRay * 8 + vPlayerPos * 8).y) },
                     olc::BLUE);
        }

        return true;
    }
};

int main()
{
    Engine engine;
    olc::GraphicsMode graphics_mode;
    //graphics_mode = { 1920, 500, 1, 1 };
    //graphics_mode = { 1536, 720, 1, 1 };
    //graphics_mode = { 512, 240, 3, 3 };
    graphics_mode = { 420, 240, 3, 3 };
    //graphics_mode = { 1600, 900, 1, 1 };
    if (engine.Construct(graphics_mode))
        engine.Start();
    //if (engine.Construct(120, 40, 8, 16))
        //engine.Start();
    else return 1;

    return 0;
}