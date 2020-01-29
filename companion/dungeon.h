#pragma once

#include <vector>
#include <array>
#include "imgui.h"

//////////////////////////////
// POD Types
//////////////////////////////

using int32 = int32_t;
using uint32 = uint32_t;

struct ivec2
{
    int32 x;
    int32 y;
};

//////////////////////////////
// NeighborState
//////////////////////////////

enum NeighborState
{
    ns_Unknown,
    ns_No,
    ns_Yes,
    ns__Count,
};

//////////////////////////////
// RoomState
//////////////////////////////

enum RoomState
{
    rs_Unknown,
    rs_Maybe,
    rs_Yes,
    rs_No,
    rs__Count,
};

//////////////////////////////
// Attribute
//////////////////////////////

enum Attribute
{
    a_Pit,
    a_Arrow,
    a_Dragon,
    a__Count,
};

//////////////////////////////
// Room class
//////////////////////////////

class Room;

using NeighborArray = std::array< const Room*, 4 >;

class Room
{
public:
    Room(
        const ivec2& pos);
    void reset();
    void update_room_state_no();
    void update_room_state_maybe_yes();
    bool draw(
        ImU32 background_alpha,
        const ImVec2& room_pos,
        ImDrawList& draw_list);

    bool m_visited;
    NeighborState m_neighbor_state[a__Count];
    RoomState m_room_state[a__Count];

private:
    ivec2 m_pos;

    NeighborArray get_neighbor_rooms() const;
    NeighborState get_neighbor_state(
        const Attribute attrib) const;
    void update_room_state_attr_no(
        const Attribute attrib,
        const NeighborArray& neighbor_rooms);

    int32 get_neighbor_attr_no_count(
        const Attribute attrib) const;

    void update_room_state_attr_maybe_yes(
        const Attribute attrib,
        const NeighborArray& neighbor_rooms);
};

//////////////////////////////
// Dungeon class
//////////////////////////////

using RoomRow = std::vector<Room>;
using RoomRows = std::vector<RoomRow>;

class Dungeon
{
public:
    Dungeon();
    void draw();
    void reset();
    void draw_selected_room_details();
    void move_selection(
        const ivec2& offset);
    void explore(
        const bool pit,
        const bool arrow,
        const bool dragon);

    void found_a_pit();
    void update_room_states();

    const Room& get_room(
        const ivec2& roomCoord) const;

private:

    RoomRows m_rows;
    ivec2 m_selected_room{ 0,0 };

    void draw_dungeon(
        const ImVec2 screen_pos,
        ImDrawList& draw_list);

    void draw_grid(
        ImVec2 screen_pos,
        ImDrawList& draw_list) const;
};
