#ifndef EDITORSYSTEM_H
#define EDITORSYSTEM_H


#include "EditorState.h"
#include "../TileMap/TileMap.h"
#include <SDL.h>
#include <imgui/imgui.h>
#include <algorithm>

class EditorSystem {

	public:

		EditorSystem(EditorState& editorState) :
			editorState(editorState) {
		}

		void Update(double deltaTime, SDL_Rect& camera, TileMap& tileMap) {
			
			const Uint8* keys = SDL_GetKeyboardState(nullptr);

			int speed = 800;
			if (keys[SDL_SCANCODE_LSHIFT]) {
				speed = 1600;
			}
			if (keys[SDL_SCANCODE_W]) {
				camera.y -= static_cast<int>(speed * deltaTime);
			}
			if (keys[SDL_SCANCODE_A]) {
				camera.x -= static_cast<int>(speed * deltaTime);
			}
			if (keys[SDL_SCANCODE_S]) {
				camera.y += static_cast<int>(speed * deltaTime);
			}
			if (keys[SDL_SCANCODE_D]) {
				camera.x += static_cast<int>(speed * deltaTime);
			}
			const int mapWidthPixels = static_cast<int>(tileMap.Cols() * tileMap.TileWorldSize());
			const int mapHeightPixels = static_cast<int>(tileMap.Rows() * tileMap.TileWorldSize());
			camera.x = std::clamp(camera.x, 0 , mapWidthPixels - camera.w);
			camera.y = std::clamp(camera.y, 0 , mapHeightPixels- camera.h);

			//Hovering mouse over the tile
			ImGuiIO& io = ImGui::GetIO();

			if (!io.WantCaptureMouse) {

				double worldX = io.MousePos.x + camera.x;
				double worldY = io.MousePos.y + camera.y;
				int col = tileMap.colAt(worldX);
				int row = tileMap.rowAt(worldY);

				if (tileMap.isInside(col, row)) {
					editorState.hoveredCol = col;
					editorState.hoveredRow = row;
				}
				else {
					editorState.hoveredCol = -1;
					editorState.hoveredRow = -1;
				}
			}
			else {
				editorState.hoveredCol = -1;
				editorState.hoveredRow = -1;
			}

			const Uint32 mouseButtons = SDL_GetMouseState(nullptr, nullptr);
			const bool leftDown = mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT);
			const bool clicked = leftDown && !wasLeftDown;
			const bool hovered = editorState.hoveredCol != -1;
			wasLeftDown = leftDown;
			

			switch (editorState.tool) {

				case EditorTool::Pick:
					if (clicked && hovered) {
						editorState.selectedTileId = tileMap.tileAt(editorState.hoveredCol, editorState.hoveredRow);
					}
					break;
				case EditorTool::Paint:
					if (leftDown && hovered) {
						PaintCell(editorState.hoveredCol, editorState.hoveredRow, tileMap);
					}
					break;
				case EditorTool::Fill:
					//TODO flood fill on click
					break;
			}

			if (!leftDown) {
				lastPaintedCol = -1;
				lastPaintedRow = -1;
			}

		}


	private:
		EditorState& editorState;
		bool wasLeftDown = false;
		int lastPaintedCol = -1;
		int lastPaintedRow = -1;

		void PaintCell(int col, int row, TileMap& tileMap) {
			//No point of painting same tile twice, same cell as last frame
			if (col == lastPaintedCol && row == lastPaintedRow) {
				return;
			}

			lastPaintedCol = col;
			lastPaintedRow = row;

			//No point of painting tile that has the same texture as paint has
			if (tileMap.tileAt(col, row) == editorState.selectedTileId) {
				return;
			}

			tileMap.SetTile(col, row, editorState.selectedTileId);
		}
};

#endif // !EDITORSYSTEM_H
