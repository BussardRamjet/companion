#include "companion.h"
#include "imgui.h"
#include <array>
#include <string>

using int32 = int32_t;

enum class Certainty
{
    Unknown,
    Uncertain,
    Certain,
    _Count,
};

const std::string s_certainty_labels[]
{
    "Unknown",
    "Uncertain",
    "Certain",
};

static_assert((int32)Certainty::_Count == std::size(s_certainty_labels));

const ImU32 s_certainty_colors[]
{
    IM_COL32(128, 128, 128, 255),
    IM_COL32(255, 255, 0, 255),
    IM_COL32(255, 0, 0, 255),
};

static_assert((int32)Certainty::_Count == std::size(s_certainty_colors));

class FuzzyBool
{
public:
    FuzzyBool()
    {
        m_value = false;
        m_certainty = Certainty::Unknown;
    }

    void draw(
        const char* name)
    {
        ImGui::Checkbox(name, &m_value);
        ImGui::SameLine();
        if (ImGui::BeginCombo((std::string("##combo") + name).c_str(), s_certainty_labels[(int32)m_certainty].c_str())) // The second parameter is the label previewed before opening the combo.
        {
            for (int32 i = 0; i < (int32)Certainty::_Count; i++)
            {
                if (ImGui::Selectable(s_certainty_labels[i].c_str(), (int32)m_certainty == i)) m_certainty = (Certainty)i;
            }
            ImGui::EndCombo();
        }
    }

    void apply(
        bool value)
    {
        if (m_certainty == Certainty::Certain)
        {
            return;
        }

        m_value = value;

        if (!m_value)
        {
            m_certainty = Certainty::Certain;
        }
        else
        {
            m_certainty = Certainty::Uncertain;
        }
    }

    Certainty m_certainty;
    bool m_value;
};

float room_screen_size = 52.f;
float room_font_size_mult = 0.28f;

class Room
{
public:
    Room()
    {
        m_visited = false;
    }

    bool draw(
        ImU32 background_alpha,
        const ImVec2& room_pos,
        ImDrawList& draw_list)
    {
        ImVec2 room_pos_max{ room_pos.x + room_screen_size, room_pos.y + room_screen_size };
        bool hovered = ImGui::IsMouseHoveringRect(room_pos, room_pos_max);

        if (hovered)
        {
            background_alpha += 32;
        }

        ImU32 color = IM_COL32(255, 255, 255, background_alpha);
        if (m_visited)
        {
            color = IM_COL32(32, 32, 32, background_alpha);
        }

        draw_list.AddRectFilled(room_pos, room_pos_max, color);

        if (m_dragon.m_certainty == Certainty::Unknown &&
            m_pit.m_certainty == Certainty::Unknown &&
            m_arrow.m_certainty == Certainty::Unknown)
        {
            draw_list.AddText(nullptr, room_screen_size * 0.75f, { room_pos.x + room_screen_size * 0.3f, room_pos.y + room_screen_size * 0.15f }, IM_COL32_WHITE, "?");        
        }
        else
        {
            int lineCounter = m_pit.m_value + m_dragon.m_value + m_arrow.m_value;
            if (lineCounter != 0)
            {
                ImVec2 pos(room_pos);
                float lineHeight = room_screen_size * room_font_size_mult;
                if (lineCounter == 1)
                {
                    pos.y += (room_screen_size - (room_screen_size * room_font_size_mult)) / 2;
                }
                else if (lineCounter == 2)
                {
                    pos.y += (room_screen_size - (room_screen_size * room_font_size_mult)) / 3;
                }
                else if(lineCounter == 3)
                {
                    pos.y += room_screen_size * 0.1f;
                }

                if (m_pit.m_value)
                {
                    draw_list.AddText(nullptr, room_screen_size * room_font_size_mult, { pos.x + room_screen_size * 0.05f, pos.y }, s_certainty_colors[(int32)m_pit.m_certainty], "Pit");
                    pos.y += lineHeight;
                }

                if (m_dragon.m_value)
                {
                    draw_list.AddText(nullptr, room_screen_size * room_font_size_mult, { pos.x + room_screen_size * 0.05f, pos.y }, s_certainty_colors[(int32)m_dragon.m_certainty], "DRAGON");
                    pos.y += lineHeight;
                }

                if (m_arrow.m_value)
                {
                    draw_list.AddText(nullptr, room_screen_size * room_font_size_mult, { pos.x + room_screen_size * 0.05f, pos.y }, s_certainty_colors[(int32)m_arrow.m_certainty], "Arrow");
                    pos.y += lineHeight;
                }
            }
        }

        return hovered && ImGui::IsMouseClicked(0);
    }

    bool m_visited;
    FuzzyBool m_pit;
    FuzzyBool m_dragon;
    FuzzyBool m_arrow;
};

struct ivec2
{
    int32 x;
    int32 y;
};

constexpr int dungeon_size = 10;

using RoomRow = std::array<Room, dungeon_size>;
using RoomRows = std::array<RoomRow, dungeon_size>;

class Dungeon
{
public:
    void draw()
    {
        float dungeon_screen_size = (dungeon_size + 1) * room_screen_size;
        auto screen_pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy({ dungeon_screen_size, dungeon_screen_size });
        auto draw_list = ImGui::GetWindowDrawList();

        draw_grid(screen_pos, *draw_list);
        draw_dungeon(screen_pos, *draw_list);
    }

    void draw_selected_room_details()
    {
        ImGui::Text("Position: %c:%d", m_selected_room.y + 65, m_selected_room.x);
        auto& room = m_rows[m_selected_room.y][m_selected_room.x];
        ImGui::Checkbox("Visited", &room.m_visited);
        room.m_pit.draw("Pit");
        room.m_dragon.draw("Dragon");
        room.m_arrow.draw("Arrow");
    }

    void move_selection(
        const ivec2& offset)
    {
        m_selected_room.x = m_selected_room.x + offset.x;
        m_selected_room.y = m_selected_room.y + offset.y;
        if (m_selected_room.x >= dungeon_size) m_selected_room.x = 0;
        if (m_selected_room.y >= dungeon_size) m_selected_room.y = 0;
        if (m_selected_room.x < 0) m_selected_room.x = dungeon_size - 1;
        if (m_selected_room.y < 0) m_selected_room.y = dungeon_size - 1;
    }

    void explore(
        bool pit,
        bool arrow,
        bool dragon)
    {
        Room& room = m_rows[m_selected_room.y][m_selected_room.x];
        room.m_visited = true;
        room.m_arrow.apply(false);
        room.m_dragon.apply(false);
        room.m_pit.apply(false);

        move_selection({ 1, 0 });
        apply_speculative(pit, arrow, dragon);

        move_selection({ -1, 1 });
        apply_speculative(pit, arrow, dragon);

        move_selection({ -1, -1 });
        apply_speculative(pit, arrow, dragon);

        move_selection({ 1, -1 });
        apply_speculative(pit, arrow, dragon);

        move_selection({ 0, 1 });
    }

private:



    RoomRows m_rows;
    ivec2 m_selected_room{ 0,0 };

    void draw_dungeon(
        const ImVec2 screen_pos,
        ImDrawList& draw_list)
    {
        auto dungeon_pos = ImVec2{ screen_pos.x + room_screen_size, screen_pos.y + room_screen_size };

        for (int y = 0; y < dungeon_size; y++)
        {
            float row_start_y = dungeon_pos.y + y * room_screen_size;
            for (int x = 0; x < dungeon_size; x++)
            {
                ImVec2 room_pos{ dungeon_pos.x + x * room_screen_size, row_start_y };
                ImU32 background_alpha = 96 + (x & 1) * 16 + (y & 1) * 16;

                auto& room = m_rows[y][x];
                bool should_select = room.draw(background_alpha, room_pos, draw_list);

                if (should_select)
                {
                    m_selected_room = {x, y};
                }
            }
        }

        ImVec2 room_pos{ dungeon_pos.x + m_selected_room.x * room_screen_size, dungeon_pos.y + m_selected_room.y * room_screen_size };
        ImVec2 room_pos_max{ room_pos.x + room_screen_size, room_pos.y + room_screen_size };

        draw_list.AddLine({ room_pos.x, room_pos.y }, { room_pos.x, room_pos_max.y }, IM_COL32(32, 32, 255, 255), 3.f);
        draw_list.AddLine({ room_pos.x, room_pos.y }, { room_pos_max.x, room_pos.y }, IM_COL32(32, 32, 255, 255), 3.f);
        draw_list.AddLine({ room_pos.x, room_pos_max.y }, { room_pos_max.x, room_pos_max.y }, IM_COL32(32, 32, 255, 255), 3.f);
        draw_list.AddLine({ room_pos_max.x, room_pos.y }, { room_pos_max.x, room_pos_max.y }, IM_COL32(32, 32, 255, 255), 3.f);

    }

    void draw_grid(
        ImVec2 screen_pos,
        ImDrawList& draw_list) const
    {
        for (int i = 0; i <= dungeon_size + 1; i++)
        {
            float room_start_x = screen_pos.x + i * room_screen_size;
            float room_start_y = screen_pos.y + i * room_screen_size;

            if (i > 0 && i <= dungeon_size)
            {
                char str[2] = { 0 };

                str[0] = '0' + char(i - 1);
                draw_list.AddText(nullptr, room_screen_size * 0.75f, { room_start_x + room_screen_size * 0.3f, screen_pos.y + room_screen_size * 0.15f }, IM_COL32_WHITE, str);

                str[0] = 'A' + char(i - 1);
                draw_list.AddText(nullptr, room_screen_size * 0.75f, { screen_pos.x + room_screen_size * 0.3f, room_start_y + room_screen_size * 0.15f }, IM_COL32_WHITE, str);
            }

            draw_list.AddLine(
                { room_start_x, screen_pos.y },
                { room_start_x, screen_pos.y + (dungeon_size + 1) * room_screen_size },
                IM_COL32_WHITE);

            draw_list.AddLine(
                { screen_pos.x, room_start_y },
                { screen_pos.x + (dungeon_size + 1) * room_screen_size, room_start_y },
                IM_COL32_WHITE);
        }
    }

    void apply_speculative(
        bool pit,
        bool arrow,
        bool dragon)
    {
        Room& room = m_rows[m_selected_room.y][m_selected_room.x];
        if (!room.m_visited)
        {
            room.m_pit.apply(pit);
            room.m_dragon.apply(dragon);
            room.m_arrow.apply(arrow);
        }
    }
};

Dungeon s_dungeon;

static bool s_dragon = false;
static bool s_pit = false;
static bool s_arrow = false;

void actions_draw()
{
    ImGui::Dummy({ 29.f, 0.f });
    ImGui::SameLine();
    if (ImGui::Button(" ^ ") ||
        ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
    {
        s_dungeon.move_selection({0, -1});
    }
    
    if (ImGui::Button(" < ") ||
        ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        s_dungeon.move_selection({ -1, 0 });
    }
    ImGui::SameLine();
    
    if (ImGui::Button(" v ") ||
        ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
    {
        s_dungeon.move_selection({ 0, 1 });
    }
    ImGui::SameLine();
    
    if (ImGui::Button(" > ") ||
        ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        s_dungeon.move_selection({ 1, 0 });
    }

    ImGui::Dummy({ 4.f, 16.f });

    ImGui::Checkbox("Pit", &s_pit);

    if (ImGui::IsKeyPressed('P'))
    {
        s_pit = !s_pit;
    }

    ImGui::Checkbox("Arrow", &s_arrow);
    if (ImGui::IsKeyPressed('A'))
    {
        s_arrow = !s_arrow;
    }

    ImGui::Checkbox("Dragon", &s_dragon);
    if (ImGui::IsKeyPressed('D'))
    {
        s_dragon = !s_dragon;
    }

    ImGui::Dummy({ 4.f, 2.f });
    ImGui::Dummy({ 9.f, 0.f });
    ImGui::SameLine();
    if (ImGui::Button("Explore!") ||
        ImGui::IsKeyPressed(' '))
    {
        s_dungeon.explore(s_pit, s_arrow, s_dragon);
    }
}

void companion_draw()
{
    ImGui::Begin("Companion", 0, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Welcome to Mattel DnD Portable Companion!");
    s_dungeon.draw();
    ImGui::Text("(c) 2019 Norbert Szabo");
    ImGui::End();

    ImGui::Begin("Room", 0, ImGuiWindowFlags_AlwaysAutoResize);
    s_dungeon.draw_selected_room_details();
    ImGui::End();

    ImGui::Begin("Actions", 0, ImGuiWindowFlags_AlwaysAutoResize);
    actions_draw();
    ImGui::End();


}