#include "companion.h"
#include "imgui.h"
#include <array>

enum class Certainty
{
	Unknown,
	Uncertain,
	Certain,
};
class FuzzyBool
{
public:
	FuzzyBool()
	{
		m_value = false;
		m_certainty = Certainty::Unknown;
	}
	Certainty m_certainty;
	bool m_value;
};

class Room
{
public:
	Room()
	{
		m_visited = false;
	}

	bool m_visited;
	FuzzyBool m_pit;
	FuzzyBool m_dragon;
	FuzzyBool m_arrow;
};

constexpr int dungeon_size = 10;

using RoomRow = std::array<Room, dungeon_size>;
using RoomRows = std::array<RoomRow, dungeon_size>;

constexpr float room_screen_size = 32.f;

class Dungeon
{
public:
	void draw() const
	{
		float dungeon_screen_size = (dungeon_size + 1) * room_screen_size;
		auto screen_pos = ImGui::GetCursorScreenPos();
		ImGui::Dummy({ dungeon_screen_size, dungeon_screen_size});
		auto draw_list = ImGui::GetWindowDrawList();

		draw_dungeon(screen_pos, draw_list);
		draw_grid(screen_pos, draw_list);
	}

	RoomRows m_rows;
private:

	void draw_dungeon(
		const ImVec2 screen_pos,
		ImDrawList* draw_list) const
	{
		auto dungeon_pos = ImVec2{ screen_pos.x + room_screen_size, screen_pos.y + room_screen_size };

		for (int y = 0; y < dungeon_size; y++)
		{
			float row_start_y = dungeon_pos.y + y * room_screen_size;
			for (int x = 0; x < dungeon_size; x++)
			{
				const auto& room = m_rows[y][x];

				ImU32 alpha = 96 + (x & 1) * 16 + (y & 1) * 16;
				ImU32 color = IM_COL32(255, 255, 255, alpha);
				if (room.m_visited)
				{
					color = IM_COL32(32, 32, 32, alpha);
				}

				float room_start_x = dungeon_pos.x + x * room_screen_size;
				draw_list->AddRectFilled(
					{ room_start_x, row_start_y },
					{ room_start_x + room_screen_size, row_start_y + room_screen_size },
					color);

				if (!room.m_visited &&
					room.m_dragon.m_certainty == Certainty::Unknown)
				{
					draw_list->AddText(nullptr, room_screen_size * 0.75f, { room_start_x + room_screen_size * 0.3f, row_start_y + room_screen_size * 0.15f }, IM_COL32_WHITE, "?");
				}
			}
		}
	}

	void draw_grid(
		ImVec2 screen_pos,
		ImDrawList* draw_list) const
	{
		for (int i = 0; i <= dungeon_size + 1; i++)
		{
			float room_start_x = screen_pos.x + i * room_screen_size;
			float room_start_y = screen_pos.y + i * room_screen_size;

			if (i > 0 && i <= dungeon_size)
			{
				char str[2] = { 0 };

				str[0] = '0' + char(i - 1);
				draw_list->AddText(nullptr, room_screen_size * 0.75f, { room_start_x + room_screen_size * 0.3f, screen_pos.y + room_screen_size * 0.15f }, IM_COL32_WHITE, str);

				str[0] = 'A' + char(i - 1);
				draw_list->AddText(nullptr, room_screen_size * 0.75f, { screen_pos.x + room_screen_size * 0.3f, room_start_y + room_screen_size * 0.15f }, IM_COL32_WHITE, str);
			}

			draw_list->AddLine(
				{ room_start_x, screen_pos.y },
				{ room_start_x, screen_pos.y + (dungeon_size + 1) * room_screen_size },
				IM_COL32_WHITE);

			draw_list->AddLine(
				{ screen_pos.x, room_start_y },
				{ screen_pos.x + (dungeon_size + 1) * room_screen_size, room_start_y },
				IM_COL32_WHITE);
		}
	}
};

Dungeon s_dungeon;

void companion_draw()
{
	ImGui::Begin("Companion", 0, ImGuiWindowFlags_AlwaysAutoResize);                          
	ImGui::Text("Welcome to Mattel DnD Portable Companion!");

	s_dungeon.draw();

	ImGui::Text("(c) 2019 Norbert Szabo");

	ImGui::End();

}