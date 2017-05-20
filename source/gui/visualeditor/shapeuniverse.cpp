#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMatrix4x4>

#include "Base/ServiceLocator.h"
#include "Gui/Settings.h"
#include "Gui/visualeditor/ViewportInfo.h"
#include "Model/SimulationController.h"
#include "Model/BuilderFacade.h"
#include "Model/AccessPorts/SimulationAccess.h"
#include "model/Context/SimulationContextApi.h"
#include "model/Context/SpaceMetric.h"

#include "GraphicsItemManager.h"
#include "ShapeUniverse.h"

ShapeUniverse::ShapeUniverse(QObject *parent)
	: QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(UNIVERSE_COLOR));
}

ShapeUniverse::~ShapeUniverse()
{
}

void ShapeUniverse::init(SimulationController * controller, ViewportInfo * viewport)
{
	BuilderFacade* facade = ServiceLocator::getInstance().getService<BuilderFacade>();
	_controller = controller;
	_viewport = viewport;

	auto simAccess = facade->buildSimulationAccess(_controller->getContext());
	auto items = new GraphicsItemManager();
	SET_CHILD(_simAccess, simAccess);
	SET_CHILD(_items, items);

	connect(_simAccess, &SimulationAccess::dataReadyToRetrieve, this, &ShapeUniverse::retrieveAndDisplayData);

	items->init(this);
}

void ShapeUniverse::setActive()
{
	IntVector2D size = _controller->getContext()->getSpaceMetric()->getSize();
	_items->setActive(size);

	connect(_controller, &SimulationController::nextFrameCalculated, this, &ShapeUniverse::requestData);
	ResolveDescription resolveDesc;
	_simAccess->requireData({ { 0, 0 }, size }, resolveDesc);
}

void ShapeUniverse::setInactive()
{
	disconnect(_controller, &SimulationController::nextFrameCalculated, this, &ShapeUniverse::requestData);
}

void ShapeUniverse::requestData()
{
	ResolveDescription resolveDesc;
	IntRect rect = _viewport->getRect();
	_simAccess->requireData(rect, resolveDesc);
}

void ShapeUniverse::retrieveAndDisplayData()
{
	auto const& data = _simAccess->retrieveData();
	_items->buildItems(data);
}


