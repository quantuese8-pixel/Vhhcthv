#include <Geode/Geode.hpp>
#include <eclipse.eclipse-menu/include/eclipse.hpp>

using namespace geode::prelude;

// Collects every trigger that sits on the currently selected editor layer.
static CCArray* collectTriggersOnLayer(LevelEditorLayer* editor, int layer) {
    auto result = CCArray::create();
    for (auto obj : CCArrayExt<GameObject*>(editor->m_objects)) {
        if (!obj->m_isTrigger) continue;
        if (obj->m_editorLayer != layer && obj->m_editorLayer2 != layer) continue;
        result->addObject(obj);
    }
    return result;
}

static void deleteTriggersOnCurrentLayer() {
    auto editor = LevelEditorLayer::get();
    if (!editor || !editor->m_editorUI) {
        Notification::create("Open the level editor first", NotificationIcon::Warning)->show();
        return;
    }

    // -1 means "All layers" is selected - we refuse to run in that case
    int layer = editor->m_currentLayer;
    if (layer < 0) {
        Notification::create("Select a specific editor layer (not 'All')", NotificationIcon::Warning)->show();
        return;
    }

    auto count = collectTriggersOnLayer(editor, layer)->count();
    if (count == 0) {
        Notification::create(fmt::format("No triggers on layer {}", layer), NotificationIcon::Info)->show();
        return;
    }

    createQuickPopup(
        "Delete triggers",
        fmt::format("Delete <cr>{}</c> trigger(s) on editor layer <cy>{}</c>?", count, layer),
        "Cancel", "Delete",
        [layer](auto, bool confirmed) {
            if (!confirmed) return;
            auto editor = LevelEditorLayer::get();
            if (!editor || !editor->m_editorUI) return;

            auto triggers = collectTriggersOnLayer(editor, layer);
            // select + delete through the editor UI, so Ctrl+Z (undo) works
            editor->m_editorUI->selectObjects(triggers, true);
            editor->m_editorUI->onDeleteSelected(nullptr);
        }
    );
}

$on_mod(Loaded) {
    // find() creates the tab if it doesn't exist yet
    auto tab = eclipse::MenuTab::find("Editor");
    tab.addButton("Delete triggers on current layer", &deleteTriggersOnCurrentLayer)
        .setDescription("Deletes all triggers on the currently selected editor layer. Doesn't work if 'All' layers is selected.");
}
