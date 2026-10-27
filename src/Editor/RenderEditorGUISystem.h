#ifndef RENDEREDITORGUISYSTEM_H
#define RENDEREDITORGUISYSTEM_H

#include "EditorState.h"
#include <imgui/imgui.h>
#include "../TileMap/TileMap.h"
#include <SDL.h>
#include <algorithm>

class RenderEditorGUISystem {

	public:

		RenderEditorGUISystem(EditorState& editorState) :
			editorState(editorState) {
		}

		void Update(const SDL_Rect& camera, const TileMap& tileMap) {
			ImGui::Begin("Editor");
			ImGui::Text("editor mode");

			ImDrawList* drawList = ImGui::GetBackgroundDrawList();

			const int firstCol = std::max(0, tileMap.colAt(camera.x));
			const int lastCol = std::min(tileMap.Cols() - 1, tileMap.colAt(camera.x + camera.w));
			const int firstRow = std::max(0, tileMap.rowAt(camera.y));
			const int lastRow = std::min(tileMap.Rows() - 1, tileMap.rowAt(camera.y + camera.h));

			const int tileWorldSize = static_cast<int>(tileMap.TileWorldSize());
			const int top = 0;
			const int left = 0;
			const int right = tileMap.Cols() * tileWorldSize;
			const int bottom = tileMap.Rows() * tileWorldSize;

			const auto color = IM_COL32(0, 0, 0, 100);
			const auto boundryColor = IM_COL32(155, 150, 255, 255);

			//top down lines
			for (int col = firstCol; col <= lastCol + 1; ++col) {
				const int x = col * tileWorldSize - camera.x;
				drawList->AddLine(ImVec2(x, top), ImVec2(x, bottom), color);
			}
			//left to right lines
			for (int row = firstRow; row <= lastRow + 1; ++row) {
				const int y = row * tileWorldSize - camera.y;
				drawList->AddLine(ImVec2(left, y), ImVec2(right, y), color);
			}

			if (editorState.hoveredCol != -1) {
				const int x = editorState.hoveredCol * tileWorldSize - camera.x;
				const int y = editorState.hoveredRow * tileWorldSize - camera.y;
				ImGui::Text("Cell (%d, %d)  id %d", editorState.hoveredCol, editorState.hoveredRow, tileMap.tileAt(editorState.hoveredCol, editorState.hoveredRow));
				drawList->AddRect(ImVec2(x, y), ImVec2(x + tileWorldSize, y + tileWorldSize), boundryColor, 0.0f, 0, 2.0f);
			}

			else {
				ImGui::Text("no cell");
			}

			ImGui::End();
			return;
		}

	private:
		EditorState& editorState;
};

#endif // !RENDEREDITORGUISYSTEM_H
