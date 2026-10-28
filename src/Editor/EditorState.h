#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include "EditorCommand.h"
#include <memory>
#include <vector>
#include <string>


enum class EditorTool {
	Paint,
	Fill,
	Pick
};

struct EditorState {
	int selectedTileId = 0;					//first tile form the tilesheet
	EditorTool tool = EditorTool::Paint;	//default tool paint(brush)
	int hoveredCol = -1;
	int hoveredRow = -1;
	int fillLimit = 100000;
	std::string message;

	std::vector<std::unique_ptr<EditorCommand>> undoStack;
	std::vector<std::unique_ptr<EditorCommand>> redoStack;
};

#endif // !EDITORSTATE_H
