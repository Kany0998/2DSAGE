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


#endif // !EDITORCOMMAND_H
