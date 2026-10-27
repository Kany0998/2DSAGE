#ifndef EDITORSTATE_H
#define EDITORSTATE_H

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
};

#endif // !EDITORSTATE_H
