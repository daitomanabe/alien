#pragma once

#include "Model/AccessPorts/SimulationAccess.h"
#include "Model/Context/SimulationContext.h"
#include "Model/Context/UnitObserver.h"
#include "Model/Entities/Descriptions.h"

#include "GpuObserver.h"

class SimulationAccessGpuImpl
	: public SimulationAccess
	, public GpuObserver
{
public:
	SimulationAccessGpuImpl(QObject* parent = nullptr) : SimulationAccess(parent) {}
	virtual ~SimulationAccessGpuImpl();

	virtual void init(SimulationContextApi* context) override;

	virtual void updateData(DataDescription const &desc) override;
	virtual void requireData(IntRect rect, ResolveDescription const& resolveDesc) override;
	virtual DataDescription const& retrieveData() override;

	virtual void unregister() override;
	virtual void accessToUnits() override;
	
private:
	SimulationContextGpuImpl* _context = nullptr;
	bool _registered = false;

	IntRect _requiredRect;
	ResolveDescription _resolveDesc;
	DataDescription _dataCollected;
};

