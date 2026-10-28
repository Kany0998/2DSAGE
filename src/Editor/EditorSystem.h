#ifndef EDITORSYSTEM_H
#define EDITORSYSTEM_H


#include "EditorState.h"
#include "EditorCommand.h"
#include "../TileMap/TileMap.h"
#include <SDL.h>
#include <imgui/imgui.h>
#include <memory>
#include <algorithm>
#include <queue>
#include <utility>

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

			const auto ctrl = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];

			const bool undoDown = ctrl && keys[SDL_SCANCODE_Z];
			const bool undoPressed = undoDown && !wasUndoDown;
			wasUndoDown = undoDown;

			const bool redoDown = ctrl && keys[SDL_SCANCODE_Y];
			const bool redoPressed = redoDown && !wasRedoDown;
			wasRedoDown = redoDown;


			if (undoPressed && !leftDown && !editorState.undoStack.empty()) {
				auto command = std::move(editorState.undoStack.back());
				editorState.undoStack.pop_back();
				command->Undo(tileMap);
				editorState.redoStack.push_back(std::move(command));
			}

			if (redoPressed && !leftDown && !editorState.redoStack.empty()) {
				auto command = std::move(editorState.redoStack.back());
				editorState.redoStack.pop_back();
				command->Redo(tileMap);
				editorState.undoStack.push_back(std::move(command));
			}
			

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
					if (clicked && hovered) {
						FillFrom(editorState.hoveredCol, editorState.hoveredRow, tileMap);
					}
					break;
			}

			if (!leftDown) {
				lastPaintedCol = -1;
				lastPaintedRow = -1;
				if (currentStroke) {
					editorState.undoStack.push_back(std::move(currentStroke));
					editorState.redoStack.clear();
				}
			}

		}


	private:
		EditorState& editorState;
		bool wasLeftDown = false;
		bool wasUndoDown = false;
		bool wasRedoDown = false;
		int lastPaintedCol = -1;
		int lastPaintedRow = -1;

		static constexpr int directions = 4;
		static constexpr int dirX[directions] = {0, 1, 0, -1};
		static constexpr int dirY[directions] = {-1, 0, 1, 0};

		std::unique_ptr<PaintCommand> currentStroke;

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

			if (currentStroke == nullptr) {
				currentStroke = std::make_unique<PaintCommand>();
			}

			currentStroke->Add(col, row, tileMap.tileAt(col, row), editorState.selectedTileId);
			tileMap.SetTile(col, row, editorState.selectedTileId);
		}

		void FillFrom(int col, int row, TileMap& tileMap) {
			const int target = tileMap.tileAt(col, row);
			const int newId = editorState.selectedTileId;

			if (target == newId) {
				return;				//lack of this if means endless loop
			}

			auto command = std::make_unique<PaintCommand>();
			std::queue<std::pair<int, int>> toVisit;		//cells that waits for expansion
			int count = 1;

			command->Add(col, row, target, newId);
			tileMap.SetTile(col, row, newId);
			toVisit.push({ col, row });
			

			while (!toVisit.empty()) {
				
				auto cell = toVisit.front();
				toVisit.pop();

				for (int i = 0; i < directions; ++i) {
					int nCol = cell.first + dirX[i];
					int nRow = cell.second + dirY[i];

					if (tileMap.isInside(nCol, nRow) && tileMap.tileAt(nCol, nRow) == target) {
						if (count == editorState.fillLimit) {
							command->Undo(tileMap);
							editorState.message = "Area too big more then fillLimit: "+ std::to_string(editorState.fillLimit) + ", nothing filled";
							return;
						}
						count++;
						command->Add(nCol, nRow, target, newId);
						tileMap.SetTile(nCol, nRow, newId);
						toVisit.push({ nCol, nRow });
					}
				}
			}
			editorState.message = "Filled: " + std::to_string(count) + " tiles";
			editorState.undoStack.push_back(std::move(command));
			editorState.redoStack.clear();
			return;
		}
};

#endif // !EDITORSYSTEM_H
