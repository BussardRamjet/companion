#include "companion.h"
#include "dungeon.h"
#include "imgui.h"

// Naming conventions:
// pod types - lowercase
// types,using - PascalCase
// functions - snake_case
// member functions - snake_case
// members variables - snake_case with m_ prefix
// static variables - snake_case
// enums - PascalCase
// enum values - enum_abbrv_PascalCase

//////////////////////////////
// Utility functions
//////////////////////////////

ImVec2 operator* (
    const ImVec2 v, 
    const float f)
{
    return { v.x * f, v.y * f };
}

//////////////////////////////
// Constants
//////////////////////////////

constexpr float room_screen_size_base = 52.f;
constexpr float room_font_size_mult_base = 0.28f;

static float window_scale = 1.0f;

extern float room_screen_size;
extern float room_font_size_mult;

//////////////////////////////
// Layout
//////////////////////////////

class Layout
{
public:
    const ImVec2 m_nativeSize;

    const ImVec2 m_companionPos;
    const ImVec2 m_dungeonPos;
    const ImVec2 m_roomPropertiesPos;
    const ImVec2 m_actionsPos;

    float get_window_scale() const
    {
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;

        float xMult = displaySize.x / m_nativeSize.x;
        float yMult = displaySize.y / m_nativeSize.y;

        return std::min(xMult, yMult);
    }
};

static const std::array<Layout, 2> layouts
{ // Cannot do constexrp :(
    {
        { // Regular
            .m_nativeSize { 933.f, 656.f},
            .m_companionPos { 612.f, 579.f },
            .m_dungeonPos { 17.f, 14.f },
            .m_roomPropertiesPos { 615.f, 15.f },
            .m_actionsPos { 693.f, 325.f }
        },

        { // Vertical
            .m_nativeSize { 618.f, 950.f},
            .m_companionPos { 24.f, 874.f },
            .m_dungeonPos { 17.f, 14.f },
            .m_roomPropertiesPos { 331.f, 650.f },
            .m_actionsPos { 118.f, 648.f }
        }
    }
};

//////////////////////////////
// Companion
//////////////////////////////

constexpr uint32 windowSettings =
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoSavedSettings;

class Companion
{
public:
    void draw()
    {
        const Layout& layout = find_best_layout();
        window_scale = layout.get_window_scale();

        room_screen_size = room_screen_size_base * window_scale;
        room_font_size_mult = room_font_size_mult_base * window_scale;

        static ImGuiStyle originalStyle = ImGui::GetStyle();
        ImGui::GetStyle() = originalStyle;
        ImGui::GetStyle().ScaleAllSizes(window_scale);
        ImGui::GetIO().FontGlobalScale = window_scale;

        ImGui::SetNextWindowPos(layout.m_companionPos * window_scale);
        ImGui::Begin("Companion", 0, windowSettings);
        ImGui::Text("Welcome to Mattel DnD Portable Companion!");
        ImGui::Text("v0.5.1             (c) 2020 BussardRamjet");
        ImGui::End();

        ImGui::SetNextWindowPos(layout.m_dungeonPos * window_scale);
        ImGui::Begin("Dungeon", 0, windowSettings);
        m_dungeon.draw();
        ImGui::End();

        ImGui::SetNextWindowPos(layout.m_roomPropertiesPos * window_scale);
        ImGui::Begin("Room properties", 0, windowSettings);
        m_dungeon.draw_selected_room_details();
        ImGui::End();

        ImGui::SetNextWindowPos(layout.m_actionsPos * window_scale);
        ImGui::Begin("Actions", 0, windowSettings);
        actions_draw();
        ImGui::End();
    }

    const Room& get_dungeon_room(
        const ivec2& room_pos)
    {
        return m_dungeon.get_room(room_pos);
    }

private:
    Dungeon m_dungeon;

    bool m_dragon = false;
    bool m_pit = false;
    bool m_arrow = false;

    void actions_draw()
    {
        ImGui::Dummy({ 29.f, 0.f });
        ImGui::SameLine();
        if (ImGui::Button(" ^ ") ||
            ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
        {
            m_dungeon.move_selection({ 0, -1 });
        }

        if (ImGui::Button(" < ") ||
            ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
        {
            m_dungeon.move_selection({ -1, 0 });
        }
        ImGui::SameLine();

        if (ImGui::Button(" v ") ||
            ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
        {
            m_dungeon.move_selection({ 0, 1 });
        }
        ImGui::SameLine();

        if (ImGui::Button(" > ") ||
            ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
        {
            m_dungeon.move_selection({ 1, 0 });
        }

        ImGui::Dummy({ 4.f, 4.f });
        ImGui::Separator();
        ImGui::Dummy({ 4.f, 4.f });

        ImGui::Checkbox("Pit", &m_pit);

        if (ImGui::IsKeyPressed('P'))
        {
            m_pit = !m_pit;
        }

        ImGui::Checkbox("Arrow", &m_arrow);
        if (ImGui::IsKeyPressed('A'))
        {
            m_arrow = !m_arrow;
        }

        ImGui::Checkbox("Dragon", &m_dragon);
        if (ImGui::IsKeyPressed('D'))
        {
            m_dragon = !m_dragon;
        }

        ImGui::Dummy({ 4.f, 2.f });
        ImGui::Dummy({ 9.f, 0.f });
        ImGui::SameLine();
        if (ImGui::Button("Explore!") ||
            ImGui::IsKeyPressed(' '))
        {
            m_dungeon.explore(m_pit, m_arrow, m_dragon);
        }

        ImGui::Separator();

        if (ImGui::Button("Found a Pit!"))
        {
            m_dungeon.found_a_pit();
        }
    }

    const Layout& find_best_layout() const
    {
        uint32 bestLayout = 0;
        float scale = layouts[bestLayout].get_window_scale();

        for (uint32 i = 1; i < layouts.size(); i++)
        {
            float current_scale = layouts[i].get_window_scale();
            if (current_scale > scale)
            {
                scale = current_scale;
                bestLayout = i;
            }
        }
        return layouts[bestLayout];
    }
};

static Companion s_companion;

const Room& get_dungeon_room(
    const ivec2& room_pos)
{
    return s_companion.get_dungeon_room(room_pos);
}


//////////////////////////////
// Main entry point
//////////////////////////////
void companion_draw()
{
    s_companion.draw();
}