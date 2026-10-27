#ifndef RENDEREDITORGUISYSTEM_H
#define RENDEREDITORGUISYSTEM_H

#include "EditorState.h"
#include <imgui/imgui.h>

class RenderEditorGUISystem {

	public:

		RenderEditorGUISystem(EditorState& editorState) :
			editorState(editorState) {
		}

		void Update() {
			ImGui::Begin("Editor");
			ImGui::Text("editor mode");
			ImGui::End();
			return;
		}

	private:
		const EditorState& editorState;
};

#endif // !RENDEREDITORGUISYSTEM_H
