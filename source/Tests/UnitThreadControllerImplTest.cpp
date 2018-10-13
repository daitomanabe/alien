#include <gtest/gtest.h>

#include "Base/ServiceLocator.h"
#include "ModelBasic/ModelBasicBuilderFacade.h"
#include "ModelBasic/Settings.h"
#include "ModelBasic/SimulationController.h"
#include "ModelCpu/SimulationContextCpuImpl.h"
#include "ModelCpu/Unit.h"
#include "ModelCpu/UnitContext.h"
#include "ModelCpu/MapCompartment.h"
#include "ModelCpu/UnitThreadControllerImpl.h"
#include "ModelCpu/UnitThread.h"

#include "tests/Predicates.h"

class UnitThreadControllerImplTest
	: public ::testing::Test
{
public:
	UnitThreadControllerImplTest();
	~UnitThreadControllerImplTest();

protected:
	SimulationController* _controller = nullptr;
	SimulationContextCpuImpl* _context = nullptr;
	UnitThreadControllerImpl* _threadController = nullptr;
	const IntVector2D _gridSize{ 9, 6 };
	const IntVector2D _universeSize{ 900, 600 };
	const std::string msgWrongNumWorkingUnits = "Wrong number of working units.";
	const std::string msgWrongNumFinishedUnits = "Wrong number of units is finished.";
	const std::string msgWrongWorkingUnit = "Wrong unit is working.";
	const std::string msgWrongReadyUnit = "Wrong unit is ready.";
};

UnitThreadControllerImplTest::UnitThreadControllerImplTest()
{
	ModelBasicBuilderFacade* facade = ServiceLocator::getInstance().getService<ModelBasicBuilderFacade>();
	auto symbols = facade->buildDefaultSymbolTable();
	auto parameters = facade->buildDefaultSimulationParameters();

	_controller = facade->buildSimulationController(4, _gridSize, _universeSize, symbols, parameters);
	_context = static_cast<SimulationContextCpuImpl*>(_controller->getContext());

	_threadController = static_cast<UnitThreadControllerImpl*>(_context->getUnitThreadController());
}

UnitThreadControllerImplTest::~UnitThreadControllerImplTest()
{
	delete _controller;
}

TEST_F(UnitThreadControllerImplTest, testStates)
{
	_threadController->updateDependencies();
	for (auto const& threadAndCalcSignal : _threadController->_threadsAndCalcSignals) {
		ASSERT_TRUE(threadAndCalcSignal.thr->isReady());
		ASSERT_FALSE(threadAndCalcSignal.thr->isFinished());
	}
	vector<int> indexOfWorkingThreads;
	int index = 0;
	for (auto const& threadAndCalcSignal : _threadController->_threadsAndCalcSignals) {
		if (threadAndCalcSignal.thr->isReady()) {
			threadAndCalcSignal.thr->setState(UnitThread::State::Working);
			indexOfWorkingThreads.push_back(index);
		}
		++index;
	}
	ASSERT_EQ(6, indexOfWorkingThreads.size()) << msgWrongNumWorkingUnits;
	ASSERT_EQ(0, indexOfWorkingThreads[0]) << msgWrongWorkingUnit;
	ASSERT_EQ(3, indexOfWorkingThreads[1]) << msgWrongWorkingUnit;
	ASSERT_EQ(18, indexOfWorkingThreads[2]) << msgWrongWorkingUnit;
	ASSERT_EQ(21, indexOfWorkingThreads[3]) << msgWrongWorkingUnit;
	ASSERT_EQ(36, indexOfWorkingThreads[4]) << msgWrongWorkingUnit;
	ASSERT_EQ(39, indexOfWorkingThreads[5]) << msgWrongWorkingUnit;

	for (auto const& threadAndCalcSignal : _threadController->_threadsAndCalcSignals) {
		ASSERT_FALSE(threadAndCalcSignal.thr->isReady());
		ASSERT_FALSE(threadAndCalcSignal.thr->isFinished());
	}
}

TEST_F(UnitThreadControllerImplTest, testStatesWithFinished)
{
	_threadController->updateDependencies();

	_threadController->_threadsAndCalcSignals[3].thr->setState(UnitThread::State::Finished);
	_threadController->_threadsAndCalcSignals[24].thr->setState(UnitThread::State::Finished);

	vector<int> indexOfWorkingThreads;
	int index = 0;
	for (auto const& threadAndCalcSignal : _threadController->_threadsAndCalcSignals) {
		if (threadAndCalcSignal.thr->isReady()) {
			threadAndCalcSignal.thr->setState(UnitThread::State::Working);
			indexOfWorkingThreads.push_back(index);
		}
		++index;
	}
	ASSERT_EQ(6, indexOfWorkingThreads.size()) << msgWrongNumWorkingUnits;
	ASSERT_EQ(0, indexOfWorkingThreads[0]) << msgWrongWorkingUnit;
	ASSERT_EQ(9, indexOfWorkingThreads[1]) << msgWrongWorkingUnit;
	ASSERT_EQ(18, indexOfWorkingThreads[2]) << msgWrongWorkingUnit;
	ASSERT_EQ(27, indexOfWorkingThreads[3]) << msgWrongWorkingUnit;
	ASSERT_EQ(36, indexOfWorkingThreads[4]) << msgWrongWorkingUnit;
	ASSERT_EQ(45, indexOfWorkingThreads[5]) << msgWrongWorkingUnit;

	for (auto const& threadAndCalcSignal : _threadController->_threadsAndCalcSignals) {
		ASSERT_FALSE(threadAndCalcSignal.thr->isReady()) << msgWrongReadyUnit;
	}

	int numFinished = 0;
	for (auto const& threadAndCalcSignal : _threadController->_threadsAndCalcSignals) {
		if (threadAndCalcSignal.thr->isFinished()) {
			++numFinished;
		}
	}
	ASSERT_EQ(2, numFinished) << msgWrongNumFinishedUnits;
}


