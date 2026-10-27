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

		void Update(double deltaTime, SDL_Rect& camera, const TileMap& tileMap) {
			
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
			wasLeftDown = leftDown;

			if (clicked && editorState.hoveredCol != -1) {
				switch (editorState.tool) {
				
					case EditorTool::Pick :
						editorState.selectedTileId = tileMap.tileAt(editorState.hoveredCol, editorState.hoveredRow);
						break;
					case EditorTool::Paint :
						//TODO
						break;
					case EditorTool::Fill :
						//TODO
						break;
				}
			}

		}


	private:
		EditorState& editorState;
		bool wasLeftDown = false;
};

#endif // !EDITORSYSTEM_H
