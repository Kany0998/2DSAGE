#ifndef EDITORCOMMAND_H
#define EDITORCOMMAND_H

#include "../TileMap/TileMap.h"
#include <vector>

class EditorCommand {
	public:
		virtual ~EditorCommand() = default;

		virtual void Undo(TileMap& tileMap) = 0;
		virtual void Redo(TileMap& tileMap) = 0;
};


struct TileChange {
	int col;
	int row;
	int oldTileId;
	int newTileId;
};

//Paint Commands

class PaintCommand : public EditorCommand {
	public:
		std::vector<TileChange> changes;

		void Add(int col, int row, int oldTileId, int newTileId) {
			changes.push_back({ col , row, oldTileId, newTileId });
		}

		void Undo(TileMap& tileMap) override {
			for (auto it = changes.rbegin(); it != changes.rend(); ++it) {
				tileMap.SetTile(it->col, it->row, it->oldTileId);
			}
		}

		void Redo(TileMap& tileMap) override {
			for (const auto& change : changes) {
				tileMap.SetTile(change.col, change.row, change.newTileId);
			}
		}
};

//Resize Commands
class ResizeCommand : public EditorCommand {
	public:
		int oldCols = 0;
		int oldRows = 0;
		std::vector<int> oldTiles;
		int newCols = 0;
		int newRows = 0;
		int fillId = 0;

		void Redo(TileMap& tileMap) override {
			tileMap.Resize(newCols, newRows, fillId);
		}

		void Undo(TileMap& tileMap) override {
			tileMap.Resize(oldCols, oldRows, fillId);
			for (int row = 0; row < oldRows; ++row) {
				for (int col = 0; col < oldCols; ++col) {
					tileMap.SetTile(col, row, oldTiles[row * oldCols + col]);
				}
			}
		}

};


#endif // !EDITORCOMMAND_H
