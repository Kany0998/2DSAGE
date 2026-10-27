#ifndef EDITORSYSTEM_H
#define EDITORSYSTEM_H


#include "EditorState.h"
#include <imgui/imgui.h>

class EditorSystem {

public:

	EditorSystem(EditorState& editorState) :
		editorState(editorState) {
	}

	void Update() {
		return;
	}

private:
	const EditorState& editorState;
};

#endif // !EDITORSYSTEM_H
