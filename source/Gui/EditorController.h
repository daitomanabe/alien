#pragma once

#include "Base/Definitions.h"
#include "EngineInterface/Descriptions.h"

#include "Definitions.h"

class _EditorController
{
public:
    _EditorController(SimulationController const& simController, Viewport const& viewport);

    bool isOn() const;
    void setOn(bool value);

    void process();

    SelectionWindow getSelectionWindow() const;
    PatternEditorWindow getPatternEditorWindow() const;
    CreatorWindow getCreatorWindow() const;
    MultiplierWindow getMultiplierWindow() const;
    GenomeEditorWindow getGenomeEditorWindow() const;
    EditorModel getEditorModel() const;

    bool areInspectionWindowsActive() const;
    void onCloseAllInspectorWindows();

    bool isObjectInspectionPossible() const;
    bool isGenomeInspectionPossible() const;
    void onInspectSelectedObjects();
    void onInspectSelectedGenomes();
    void onInspectObjects(std::vector<CellOrParticleDescription> const& entities, bool selectGenomeTab);

    bool isCopyingPossible() const;
    void onCopy();
    bool isPastingPossible() const;
    void onPaste();
    bool isDeletingPossible() const;
    void onDelete();


private:
    void processEvents();
    void processSelectionRect();
    void processInspectorWindows();

    void selectObjects(RealVector2D const& viewPos, bool modifierKeyPressed);
    void moveSelectedObjects(RealVector2D const& viewPos, RealVector2D const& prevViewPos);
    void fixateSelectedObjects(RealVector2D const& viewPos, RealVector2D const& initialViewPos);
    void accelerateSelectedObjects(RealVector2D const& viewPos, RealVector2D const& prevViewPos);
    void applyForces(RealVector2D const& viewPos, RealVector2D const& prevViewPos);

    void createSelectionRect(RealVector2D const& viewPos);
    void resizeSelectionRect(RealVector2D const& viewPos, RealVector2D const& prevViewPos);
    void removeSelectionRect();

private:
    EditorModel _editorModel;
    SelectionWindow _selectionWindow;
    PatternEditorWindow _patternEditorWindow;
    CreatorWindow _creatorWindow; 
    MultiplierWindow _multiplierWindow;
    GenomeEditorWindow _genomeEditorWindow;

    SimulationController _simController;
    Viewport _viewport;

    bool _on = false;

    struct SelectionRect
    {
        RealVector2D startPos;
        RealVector2D endPos;
    };
    std::optional<SelectionRect> _selectionRect;
    std::vector<InspectorWindow> _inspectorWindows;
    DataDescription _drawing;
    std::optional<RealVector2D> _selectionPositionOnClick;
    std::optional<RealVector2D> _mousePosOnClick;
    std::optional<RealVector2D> _prevMousePos;
};
