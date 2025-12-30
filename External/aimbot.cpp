#include <Windows.h>
#include <vector>
#include "Mathematics.h"
#include "Player.h"
#include "Entities.h"

class Aimbot {
public:
    float FOV = 60.0f; // degrees
    float smooth = 10.0f; // higher = slower
    bool enabled = true;

    void AimAtClosest(Player& localPlayer, Entities& entities, Mathematics& math) {
        vec3d_f localPos = localPlayer.position_feet;

        float bestDist = FOV;
        vec3d_f bestAngle;
        bool found = false;

        for (int i = 0; i < entities.amount; i++) {
            Entity& ent = entities.list[i];
            if (ent.health <= 0 || ent.team == localPlayer.team) continue;

            vec3d_f angles = math.CalculateAngles(localPos, ent.position_feet);

            // convert angles difference to screen distance (approximation)
            float deltaX = angles.x; // yaw difference
            float deltaY = angles.y; // pitch difference
            float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

            if (distance < bestDist) {
                bestDist = distance;
                bestAngle = angles;
                found = true;
            }
        }

        if (found && enabled) {
            // Move mouse via SendInput
            // Get screen center
            POINT cursorPos;
            GetCursorPos(&cursorPos);

            // Simple smoothing
            float moveX = (bestAngle.x) / smooth;
            float moveY = (bestAngle.y) / smooth;

            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dx = static_cast<LONG>(moveX);
            input.mi.dy = static_cast<LONG>(moveY);
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(INPUT));
        }
    }
};
