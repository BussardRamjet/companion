#include "companion.h"
#include "imgui.h"
#include <vector>
#include <array>
#include <string>

using int32 = int32_t;

struct ivec2
{
    int32 x;
    int32 y;
};

enum NeighborState
{
    ns_Unknown,
    ns_No,
    ns_Maybe,
    ns__Count,
};

const std::string s_neighbor_state_labels[]
{
    "Unknown",
    "No",
    "Maybe",
};

enum RoomState
{
    rs_Unknown,
    rs_Maybe,
    rs_Yes,
    rs_No,
    rs__Count,
};

const std::string s_room_state_labels[]
{
    "Unknown",
    "Maybe",
    "Yes",
    "No",
};

static_assert(ns__Count == std::size(s_neighbor_state_labels));

enum Attribute
{
    a_Pit,
    a_Arrow,
    a_Dragon,
    a__Count,
};

float room_screen_size = 52.f;
float room_font_size_mult = 0.28f;

class Room;
const Room& get_dungeon_room(
    const ivec2& room_pos);

using NeighborArray = std::array< const Room*, 4 >;

class Room
{
public:
    Room(
        const ivec2& pos) :
        m_pos(pos)
    {
        reset();
    }

    void reset()
    {
        m_visited = false;
        for (int32 i = 0; i < a__Count; i++)
        {
            m_neighborState[i] = ns_Unknown;
            m_roomState[i] = rs_Unknown;
        }
    }

    NeighborArray get_neighbor_rooms() const
    {
        return {
            &get_dungeon_room(ivec2{ m_pos.x - 1, m_pos.y }),
            &get_dungeon_room(ivec2{ m_pos.x + 1, m_pos.y }),
            &get_dungeon_room(ivec2{ m_pos.x, m_pos.y - 1 }),
            &get_dungeon_room(ivec2{ m_pos.x, m_pos.y + 1 })
        };
    }

    NeighborState get_neighbor_state(
        Attribute attrib)
    {
        NeighborArray neighbor_rooms = get_neighbor_rooms();

        for (auto neighbor_room : neighbor_rooms)
        {
            if (neighbor_room->m_neighborState[attrib] == ns_No)
                return ns_No;
        }

        for (auto neighbor_room : neighbor_rooms)
        {
            if (neighbor_room->m_neighborState[attrib] == ns_Maybe)
                return ns_Maybe;
        }

        return ns_Unknown;
    }

    void update_room_state_attr_no(
        Attribute attrib,
        const NeighborArray& neighbor_rooms)
    {
        m_roomState[attrib] = rs_Unknown;

        for (auto neighbor_room : neighbor_rooms)
        {
            if (neighbor_room->m_neighborState[attrib] == ns_No)
            {
                m_roomState[attrib] = rs_No;
            }
        }
    }

    int32 get_no_count(
        Attribute attrib) const
    {
        NeighborArray neighbor_rooms = get_neighbor_rooms();

        return
            (int32)(neighbor_rooms[0]->m_roomState[attrib] == rs_No) +
            (int32)(neighbor_rooms[1]->m_roomState[attrib] == rs_No) +
            (int32)(neighbor_rooms[2]->m_roomState[attrib] == rs_No) +
            (int32)(neighbor_rooms[3]->m_roomState[attrib] == rs_No);
    }

    void update_room_state_attr_maybe_yes(
        Attribute attrib,
        const NeighborArray& neighbor_rooms)
    {
        if (m_roomState[attrib] == rs_No)
        {
            return;
        }

        for (auto neighbor_room : neighbor_rooms)
        {
            if (neighbor_room->m_neighborState[attrib] == ns_Maybe)
            {
                m_roomState[attrib] = neighbor_room->get_no_count(attrib) == 3 ? rs_Yes : rs_Maybe;
            }

            if (m_roomState[attrib] != rs_Unknown)
            {
                return;
            }
        }
    }


    void update_room_state_no()
    {
        NeighborArray neighbor_rooms = get_neighbor_rooms();
        for (int32 i = 0; i < a__Count; i++)
        {
            update_room_state_attr_no((Attribute)i, neighbor_rooms);
        }
    }

    void update_room_state_maybe_yes()
    {
        NeighborArray neighbor_rooms = get_neighbor_rooms();
        for (int32 i = 0; i < a__Count; i++)
        {
            update_room_state_attr_maybe_yes((Attribute)i, neighbor_rooms);
        }
    }

    bool is_state_visible(RoomState state)
    {
        return state == rs_Maybe || state == rs_Yes;
    }

    ImU32 get_state_color(RoomState state)
    {
        return state == rs_Maybe ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
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

        if (!m_visited)
        {
            RoomState dragonState = m_roomState[a_Dragon];
            RoomState arrowState = m_roomState[a_Arrow];
            RoomState pitState = m_roomState[a_Pit];

            if (dragonState == rs_Unknown &&
                arrowState == rs_Unknown &&
                pitState == rs_Unknown)
            {
                draw_list.AddText(nullptr, room_screen_size * 0.75f, { room_pos.x + room_screen_size * 0.3f, room_pos.y + room_screen_size * 0.15f }, IM_COL32_WHITE, "?");
            }
            else
            {
                int lineCounter = (int32)(is_state_visible(dragonState)) + (int32)(is_state_visible(pitState)) + (int32)(is_state_visible(arrowState));
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
                    else if (lineCounter == 3)
                    {
                        pos.y += room_screen_size * 0.1f;
                    }

                    if (is_state_visible(pitState))
                    {
                        draw_list.AddText(nullptr, room_screen_size * room_font_size_mult, { pos.x + room_screen_size * 0.05f, pos.y }, get_state_color(pitState), "Pit");
                        pos.y += lineHeight;
                    }

                    if (is_state_visible(dragonState))
                    {
                        draw_list.AddText(nullptr, room_screen_size * room_font_size_mult, { pos.x + room_screen_size * 0.05f, pos.y }, get_state_color(dragonState), "DRAGON");
                        pos.y += lineHeight;
                    }

                    if (is_state_visible(arrowState))
                    {
                        draw_list.AddText(nullptr, room_screen_size * room_font_size_mult, { pos.x + room_screen_size * 0.05f, pos.y }, get_state_color(arrowState), "Arrow");
                        pos.y += lineHeight;
                    }
                }
            }
        }

        return hovered && ImGui::IsMouseClicked(0);
    }

    ivec2 m_pos;
    bool m_visited;
    NeighborState m_neighborState[a__Count];
    RoomState m_roomState[a__Count];
};

constexpr int dungeon_size = 10;

using RoomRow = std::vector<Room>;
using RoomRows = std::vector<RoomRow>;

class Dungeon
{
public:
    Dungeon()
    {
        for (int y = 0; y < dungeon_size; y++)
        {
            RoomRow row;
            for (int x = 0; x < dungeon_size; x++)
            {
                row.push_back({ ivec2{x, y} });
            }

            m_rows.push_back(row);
        }
    }

    void draw()
    {
        float dungeon_screen_size = (dungeon_size + 1) * room_screen_size;
        auto screen_pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy({ dungeon_screen_size, dungeon_screen_size });
        auto draw_list = ImGui::GetWindowDrawList();

        draw_grid(screen_pos, *draw_list);
        draw_dungeon(screen_pos, *draw_list);

        if (ImGui::Button("Reset dungeon"))
        {
            reset();
        }
    }

    void reset()
    {
        m_selected_room = { 0,0 };
        for (RoomRow& roomRow : m_rows)
        {
            for (Room& room : roomRow)
            {
                room.reset();
            }
        }
    }

    void draw_selected_room_details()
    {
        ImGui::Text("Position: %c:%d", m_selected_room.y + 65, m_selected_room.x);
        auto& room = m_rows[m_selected_room.y][m_selected_room.x];
        ImGui::Checkbox("Visited", &room.m_visited);
        draw_neighbor_state("Neighbor Pit", &room.m_neighborState[a_Pit]);
        draw_neighbor_state("Neighbor Dragon", &room.m_neighborState[a_Dragon]);
        draw_neighbor_state("Neighbor Arrow", &room.m_neighborState[a_Arrow]);

        draw_room_state("Room Pit", &room.m_roomState[a_Pit]);
        draw_room_state("Room Dragon", &room.m_roomState[a_Dragon]);
        draw_room_state("Room Arrow", &room.m_roomState[a_Arrow]);
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
        if (room.m_neighborState[a_Pit] != ns_No)
        {
            room.m_neighborState[a_Pit] = pit ? ns_Maybe : ns_No;
        }

        if (room.m_neighborState[a_Arrow] != ns_No)
        {
            room.m_neighborState[a_Arrow] = arrow ? ns_Maybe : ns_No;
        }

        if (room.m_neighborState[a_Dragon] != ns_No)
        {
            room.m_neighborState[a_Dragon] = dragon ? ns_Maybe : ns_No;
        }

        update_room_states();
    }

    void update_room_states()
    {
        for (int y = 0; y < dungeon_size; y++)
        {
            for (int x = 0; x < dungeon_size; x++)
            {
                auto& room = m_rows[y][x];
                room.update_room_state_no();
            }
        }

        for (int y = 0; y < dungeon_size; y++)
        {
            for (int x = 0; x < dungeon_size; x++)
            {
                auto& room = m_rows[y][x];
                room.update_room_state_maybe_yes();
            }
        }
    }

    const Room& get_room(
        const ivec2& roomCoord) const
    {
        int32 x = roomCoord.x;
        while (x < 0) x += dungeon_size;
        while (x >= dungeon_size) x -= dungeon_size;

        int32 y = roomCoord.y;
        while (y < 0) y += dungeon_size;
        while (y >= dungeon_size) y -= dungeon_size;
        return m_rows[y][x];
    }

private:

    RoomRows m_rows;
    ivec2 m_selected_room{ 0,0 };

    static void draw_neighbor_state(
        const char* name,
        NeighborState* pState)
    {
        ImGui::Text(name);
        ImGui::SameLine();

        if (ImGui::BeginCombo((std::string("##combo") + name).c_str(), s_neighbor_state_labels[*pState].c_str())) // The second parameter is the label previewed before opening the combo.
        {
            for (int32 i = 0; i < ns__Count; i++)
            {
                if (ImGui::Selectable(s_neighbor_state_labels[i].c_str(), *pState == i)) *pState = (NeighborState)i;
            }
            ImGui::EndCombo();
        }
    }

    static void draw_room_state(
        const char* name,
        RoomState* pState)
    {
        ImGui::Text(name);
        ImGui::SameLine();

        if (ImGui::BeginCombo((std::string("##combo") + name).c_str(), s_room_state_labels[*pState].c_str())) // The second parameter is the label previewed before opening the combo.
        {
            for (int32 i = 0; i < rs__Count; i++)
            {
                if (ImGui::Selectable(s_room_state_labels[i].c_str(), *pState == i)) *pState = (RoomState)i;
            }
            ImGui::EndCombo();
        }
    }

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
                    m_selected_room = { x, y };
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
};

Dungeon s_dungeon;

const Room& get_dungeon_room(
    const ivec2& room_pos)
{
    return s_dungeon.get_room(room_pos);
}


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
        s_dungeon.move_selection({ 0, -1 });
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
    ImGui::SameLine();
    ImGui::Dummy({ 305.f, 0.f });
    ImGui::SameLine();
    ImGui::Text("(c) 2019 Norbert Szabo");
    ImGui::End();

    ImGui::Begin("Room", 0, ImGuiWindowFlags_AlwaysAutoResize);
    s_dungeon.draw_selected_room_details();
    ImGui::End();

    ImGui::Begin("Actions", 0, ImGuiWindowFlags_AlwaysAutoResize);
    actions_draw();
    ImGui::End();
}