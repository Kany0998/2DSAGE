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
#include <format>
#include <filesystem>

class EditorSystem {

	public:

		EditorSystem(EditorState& editorState) :
			editorState(editorState) {
		}

		void Update(double deltaTime, SDL_Rect& camera, TileMap& tileMap) {
			
			const Uint8* keys = SDL_GetKeyboardState(nullptr);
			const bool ctrl = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];
			int speed = 800;

			ImGuiIO& io = ImGui::GetIO();
			const bool typing = io.WantCaptureKeyboard;

			if (keys[SDL_SCANCODE_LSHIFT]) {
				speed = 1600;
			}

			if (!ctrl && !typing) {
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
			}

			const int mapWidthPixels = static_cast<int>(tileMap.Cols() * tileMap.TileWorldSize());
			const int mapHeightPixels = static_cast<int>(tileMap.Rows() * tileMap.TileWorldSize());
			const int maxCameraX = std::max(0, mapWidthPixels - camera.w);
			const int maxCameraY = std::max(0, mapHeightPixels - camera.h);
			camera.x = std::clamp(camera.x, 0 , maxCameraX);
			camera.y = std::clamp(camera.y, 0 , maxCameraY);

			//Hovering mouse over the tile
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


			const bool undoDown = !typing && ctrl && keys[SDL_SCANCODE_Z];
			const bool undoPressed = undoDown && !wasUndoDown;
			wasUndoDown = undoDown;

			const bool redoDown = !typing && ctrl && keys[SDL_SCANCODE_Y];
			const bool redoPressed = redoDown && !wasRedoDown;
			wasRedoDown = redoDown;

			const bool saveDown = ctrl && keys[SDL_SCANCODE_S];
			const bool savePressed = saveDown && !wasSaveDown;
			wasSaveDown = saveDown;


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
			

			if (!fieldsFilled) {
				auto result = std::format_to_n(editorState.saveFileName, sizeof(editorState.saveFileName) - 1, "{}", tileMap.FileName());
				*result.out = '\0';
				fieldsFilled = true;
				editorState.resizeCols = tileMap.Cols();
				editorState.resizeRows = tileMap.Rows();
			}

			if ((savePressed || editorState.saveRequested) && !leftDown) {
				SaveMap(tileMap);
			}
			editorState.saveRequested = false;

			if (editorState.resizeRequested && !leftDown) {
				ResizeMap(tileMap);
			}
			editorState.resizeRequested = false;


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
		bool wasSaveDown = false;
		bool fieldsFilled = false;
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

		void SaveMap(TileMap& tileMap) {
			const std::string name = editorState.saveFileName;
			if (name.empty()) {
				editorState.message = "Enter file name";
				return;
			}

			const auto sourcePath = std::string(ASSETS_SOURCE_DIR) + "/tilemaps/" + name;
			const auto buildPath = "./assets/tilemaps/" + name;

			//Keep the previous version one file away. A new name has nothing to back up,
			//so a failed copy is fine; the error_code overload reports it instead of throwing

			std::error_code errorCode;
			std::filesystem::copy_file(sourcePath, sourcePath + ".bak", std::filesystem::copy_options::overwrite_existing, errorCode);

			if (!tileMap.Save(sourcePath)) {
				editorState.message = "Save failed: " + name;
				return;
			}

			//The build copy is what the running game reads, so update it too (no rebuild needed)
			tileMap.Save(buildPath);
			editorState.message = "Saved " + name + " (previous version in .bak)";
		}

		void ResizeMap(TileMap& tileMap) {
			int cols = editorState.resizeCols;
			int rows = editorState.resizeRows;

			if (cols < 1 || rows < 1 || cols > TileMap::maxMapSize || rows > TileMap::maxMapSize) {
				editorState.message = "Size must be  1-" + std::to_string(TileMap::maxMapSize) + ", skip (will be change in future update to chunks)";
				return;
			}

			else if (cols == tileMap.Cols() && rows == tileMap.Rows()) {
				editorState.message = "Same size,skipped";
				return;
			}

			auto command = std::make_unique<ResizeCommand>();
			command->oldCols = tileMap.Cols();
			command->oldRows = tileMap.Rows();
			command->newCols = cols;
			command->newRows = rows;
			command->fillId = editorState.selectedTileId;
			command->oldTiles.reserve(command->oldCols * command->oldRows);
			for (auto row = 0; row < command->oldRows; ++row) {
				for (auto col = 0; col < command->oldCols; ++col) {
					command->oldTiles.push_back(tileMap.tileAt(col, row));
				}
			}
			command->Redo(tileMap);
			editorState.undoStack.push_back(std::move(command));
			editorState.redoStack.clear();
			editorState.message = "Resize to Cols: " + std::to_string(cols) + " x Rows: " + std::to_string(rows);
		}
};

#endif // !EDITORSYSTEM_H
