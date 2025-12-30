#include "GDI_drawing.h"
#include <sstream>
#include <iostream>
#include "aimbot.cpp"

#pragma warning(disable : 4996) // disable strcopy (overflow)
#pragma warning(disable : 4244) // disable loss of data conversion

void GDI_drawing::SetupDrawing(HDC hDesktop, HWND handle)
{
    GDI_drawing::HDC_Desktop = hDesktop;
    GDI_drawing::Handle = handle;
    TextCOLOR = RGB(0, 255, 0);
}

void GDI_drawing::DrawFilledRect(int x, int y, int w, int h, HBRUSH brushColor)
{
    RECT rect = { x, y, x + w, y + h };
    FillRect(GDI_drawing::HDC_Desktop, &rect, brushColor);
}

void GDI_drawing::DrawBorderBox(int x, int y, int w, int h, int thickness, HBRUSH brushColor)
{
    DrawFilledRect(x, y, w, thickness, brushColor);
    DrawFilledRect(x, y, thickness, h, brushColor);
    DrawFilledRect((x + w), y, thickness, h, brushColor);
    DrawFilledRect(x, y + h, w + thickness, thickness, brushColor);
}

void GDI_drawing::DrawString(int x, int y, COLORREF color, const char* text)
{
    SetTextAlign(GDI_drawing::HDC_Desktop, TA_CENTER | TA_NOUPDATECP);
    SetBkColor(GDI_drawing::HDC_Desktop, RGB(0, 0, 0));
    SetBkMode(GDI_drawing::HDC_Desktop, TRANSPARENT);
    SetTextColor(GDI_drawing::HDC_Desktop, color);
    SelectObject(GDI_drawing::HDC_Desktop, GDI_drawing::Font);
    TextOutA(GDI_drawing::HDC_Desktop, x, y, text, strlen(text));
    DeleteObject(Font);
}

void GDI_drawing::DrawESP(
    int x, int y, float distance, int health, char name[20], HBRUSH hBrush, COLORREF Pen,
    int windowWidth, int windowHeight,
    int ar_ammo, int smg_ammo, int sniper_ammo, int shotgun_ammo, int carbine_ammo)
{
    int width = static_cast<int>(windowWidth * 1.1375 / distance);
    int height = static_cast<int>(windowHeight * 3.3333 / distance);

    // Draw ESP box
    DrawBorderBox(x - (width / 2), y - height, width, height, 1, hBrush);

    // Draw health bar
    int barWidth = 3; 
    int barX = x - (width / 2) - barWidth - 2; 
    int barY = y - height;
    int barHeight = height;
    int filledHeight = static_cast<int>(barHeight * (health / 100.0f));
    int filledY = barY + (barHeight - filledHeight);

    HBRUSH hBrushBG = CreateSolidBrush(RGB(60, 60, 60));
    DrawFilledRect(barX, barY, barWidth, barHeight, hBrushBG);
    DeleteObject(hBrushBG);

    COLORREF healthColor = (health > 60) ? RGB(0, 200, 0) : (health > 30) ? RGB(255, 200, 0) : RGB(200, 0, 0);
    HBRUSH hBrushHealth = CreateSolidBrush(healthColor);
    DrawFilledRect(barX, filledY, barWidth, filledHeight, hBrushHealth);
    DeleteObject(hBrushHealth);

    // --- Ammo Bar ---
    int ammoBarWidth = width;
    int ammoBarHeight = 6;
    int ammoBarX = x - (width / 2);
    int ammoBarY = y - height - ammoBarHeight - 2; // 2px gap above box

    float ammoRatio = 0.0f;
    int maxAmmo = 0;
    COLORREF ammoColor = RGB(212, 120, 55); // Default: blue

    if (ar_ammo >= 1 && ar_ammo <= 20) {
        ammoRatio = ar_ammo / 20.0f;
        maxAmmo = 20;
    } else if (smg_ammo >= 1 && smg_ammo <= 30) {
        ammoRatio = smg_ammo / 30.0f;
        maxAmmo = 30;
    } else if (sniper_ammo >= 1 && sniper_ammo <= 5) {
        ammoRatio = sniper_ammo / 5.0f;
        maxAmmo = 5;
    } else if (shotgun_ammo >= 1 && shotgun_ammo <= 7) {
        ammoRatio = shotgun_ammo / 7.0f;
        maxAmmo = 7;
    } else if (carbine_ammo >= 1 && carbine_ammo <= 10) {
        ammoRatio = carbine_ammo / 10.0f;
        maxAmmo = 10;
    }

    if (ammoRatio > 0.0f) {
        // Draw ammo bar background
        HBRUSH hAmmoBG = CreateSolidBrush(RGB(40, 40, 40));
        DrawFilledRect(ammoBarX, ammoBarY, ammoBarWidth, ammoBarHeight, hAmmoBG);
        DeleteObject(hAmmoBG);

        // Draw filled ammo bar
        int filledAmmoWidth = static_cast<int>(ammoBarWidth * ammoRatio);
        HBRUSH hAmmo = CreateSolidBrush(ammoColor);
        DrawFilledRect(ammoBarX, ammoBarY, filledAmmoWidth, ammoBarHeight, hAmmo);
        DeleteObject(hAmmo);
    }

    // --- Draw text ---
    std::stringstream ss;
    ss << std::to_string((int)distance) + " meters";
    std::stringstream nn;
    nn << name;

    char* distanceInfo = new char[ss.str().size() + 1];
    strcpy(distanceInfo, ss.str().c_str());
    char* nameInfo = new char[nn.str().size() + 3];
    strcpy(nameInfo, nn.str().c_str());

    DrawString(x, y, GDI_drawing::TextCOLOR, nameInfo);
    DrawString(x, y + 15, GDI_drawing::TextCOLOR, distanceInfo);

    delete[] distanceInfo;
    delete[] nameInfo;
}


DWORD WINAPI GDI_drawing::esp(Entities entities, Player player, Mathematics math)
{
    GetWindowRect(FindWindow(NULL, L"AssaultCube"), &m_Rect);

    while (true)
    {
        player.GetInfo();
        entities.GetInfo();

        static Aimbot aimbot;

        if (GetAsyncKeyState(VK_XBUTTON2)) { // toggle with mouse button 2
            aimbot.enabled = !aimbot.enabled;
        }

        // call aimbot
        aimbot.AimAtClosest(player, entities, math);


        RECT clientRect;
        HWND hwnd = FindWindow(NULL, L"AssaultCube");
        GetClientRect(hwnd, &clientRect);
        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;

        for (int i = 0; i < entities.amount; i++)
        {
            if (math.WorldToScreen(entities.list[i].position_feet, math.screen, player.matrix, windowWidth, windowHeight) && entities.list[i].health > 0)
            {

                if (entities.list[i].team != player.team)
                {
                    GDI_drawing::DrawESP(
                        math.screen.x, math.screen.y,
                        math.GetDistance3D(player.position_feet, entities.list[i].position_feet),
                        entities.list[i].health, entities.list[i].name,
                        GDI_drawing::hBrushEnemy, GDI_drawing::enemyColor,
                        windowWidth, windowHeight,
                        entities.list[i].ar_ammo, entities.list[i].smg_ammo, entities.list[i].sniper_ammo,
                        entities.list[i].shotgun_ammo, entities.list[i].carbine_ammo
                    );

                }

                else if (entities.list[i].team == player.team)
                {
                    GDI_drawing::DrawESP(
                        math.screen.x, math.screen.y,
                        math.GetDistance3D(player.position_feet, entities.list[i].position_feet),
                        entities.list[i].health, entities.list[i].name,
                        GDI_drawing::hBrushTeam, GDI_drawing::teamColor,
                        windowWidth, windowHeight,
                        entities.list[i].ar_ammo, entities.list[i].smg_ammo, entities.list[i].sniper_ammo,
                        entities.list[i].shotgun_ammo, entities.list[i].carbine_ammo
                    );

                }

 

            }
        }
    }
}
