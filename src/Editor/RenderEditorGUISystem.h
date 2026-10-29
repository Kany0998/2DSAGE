#ifndef RENDEREDITORGUISYSTEM_H
#define RENDEREDITORGUISYSTEM_H

#include "EditorState.h"
#include <imgui/imgui.h>
#include "../TileMap/TileMap.h"
#include "../AssetStore/AssetStore.h"
#include <SDL.h>
#include <algorithm>

class RenderEditorGUISystem {

	public:

		RenderEditorGUISystem(EditorState& editorState, AssetStore& assetStore) :
			editorState(editorState), assetStore(assetStore) {
		}

		void Update(const SDL_Rect& camera, const TileMap& tileMap) {
			ImGui::Begin("Editor");
			ImGui::Text("editor mode");

			//drawn over the map but under every ImGui window
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
				//Only draw a visbale range 
				drawList->AddRect(ImVec2(x, y), ImVec2(x + tileWorldSize, y + tileWorldSize), boundryColor, 0.0f, 0, 2.0f);
			}

			else {
				ImGui::Text("no cell");
			}


			ImGui::End();


			ImGui::Begin("Palette", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

			SDL_Texture* texture = assetStore.GetTexture(tileMap.TextureAssetId());

			if (texture == nullptr) {
				ImGui::Text("no tileset");
				ImGui::End();
				return;
			}

			int textureWidth = 32;
			int textureHeight = 32;

			SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
			const int tileSize = tileMap.TileSize();

			const int tilesPerRow = textureWidth / tileSize;
			//Return if we have 6 rows otherwise program would crash
			if (tilesPerRow <= 0) {
				ImGui::End();
				return;
			}

			const int rows = textureHeight / tileSize;
			const int count = tilesPerRow * rows;

			int selected = editorState.selectedTileId;
			ImGui::Text("Selected: id %d", selected);
			const int selX = (selected % tilesPerRow) * tileSize;
			const int selY = (selected / tilesPerRow) * tileSize;

			const ImVec2 selUv0 = ImVec2(static_cast<float>(selX) / textureWidth, static_cast<float>(selY) / textureHeight);
			const ImVec2 selUv1 = ImVec2(static_cast<float>(selX + tileSize) / textureWidth, static_cast<float>(selY + tileSize) / textureHeight);
			ImGui::Image(texture, ImVec2(64, 64), selUv0, selUv1);

			ImGui::Separator();

			if (ImGui::RadioButton("Paint", editorState.tool == EditorTool::Paint)) {
				editorState.tool = EditorTool::Paint;
			}
			ImGui::SameLine();

			if (ImGui::RadioButton("Fill", editorState.tool == EditorTool::Fill)) {
				editorState.tool = EditorTool::Fill;
			}
			ImGui::SameLine();

			if (ImGui::RadioButton("Pick", editorState.tool == EditorTool::Pick)) {
				editorState.tool = EditorTool::Pick;
			}
			
			if (editorState.tool == EditorTool::Fill) {
				ImGui::Separator();
				ImGui::InputInt("Fill limit", &editorState.fillLimit);
				if (editorState.fillLimit < 1) {
					editorState.fillLimit = 1;
				}
			}
			ImGui::Separator();

			ImGui::InputText("File", editorState.saveFileName, sizeof(editorState.saveFileName));
			if (ImGui::Button("Save")) {
				editorState.saveRequested = true;
			}

			ImGui::TextUnformatted(editorState.message.c_str());

			ImGui::Separator();

			for (int id = 0; id < count ; ++id) {
				int posX = (id % tilesPerRow) * tileSize;
				int posY = (id / tilesPerRow) * tileSize;

				//UVs are  (0 to 1)  fractions of the texture not pixels
				const ImVec2 uv0 = ImVec2(static_cast<float>(posX) / textureWidth, static_cast<float>(posY) / textureHeight);
				const ImVec2 uv1 = ImVec2(static_cast<float>(posX + tileSize) / textureWidth, static_cast<float>(posY + tileSize) / textureHeight);
				
				ImGui::PushID(id);
				if (ImGui::ImageButton(texture, ImVec2(40, 40), uv0, uv1)) {
					editorState.selectedTileId = id;
				}
				ImGui::PopID();

				if ((id + 1) % tilesPerRow != 0) {
					ImGui::SameLine();
				}

				//Show which tile is selected
				if (id == editorState.selectedTileId) {
					ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
				}
			}

			ImGui::End();


			return;
		}

	private:
		EditorState& editorState;
		AssetStore& assetStore;
};

#endif // !RENDEREDITORGUISYSTEM_H
