#include "GpuWorker.h"
#include "SimulationContextGpuImpl.h"
#include "SimulationAccessGpuImpl.h"

SimulationAccessGpuImpl::~SimulationAccessGpuImpl()
{
	if (_registered) {
		_context->unregisterObserver(this);
	}
}

void SimulationAccessGpuImpl::init(SimulationContextApi * context)
{
	_context = static_cast<SimulationContextGpuImpl*>(context);
	_context->registerObserver(this);
	_registered = true;
}

void SimulationAccessGpuImpl::updateData(DataDescription const & desc)
{
}

void SimulationAccessGpuImpl::requireData(IntRect rect, ResolveDescription const & resolveDesc)
{
	_requiredRect = rect;
	_resolveDesc = resolveDesc;
}

DataDescription const & SimulationAccessGpuImpl::retrieveData()
{
	return _dataCollected;
}

void SimulationAccessGpuImpl::unregister()
{
	_registered = false;
}

void SimulationAccessGpuImpl::accessToUnits()
{
	_context->getGpuWorker()->getData(_requiredRect, _resolveDesc, _dataCollected);

	Q_EMIT dataReadyToRetrieve();
}

