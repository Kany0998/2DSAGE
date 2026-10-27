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
};

#endif // !EDITORSTATE_H
