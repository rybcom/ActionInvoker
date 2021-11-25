#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include "Ranges.h"

namespace action_invoker
{
	struct ContinuousState
	{
		double progress{ 0 };
		bool is_first_pass{ false };
		bool is_last_pass{ false };
	};

	using Action = std::function<void()>;
	using ContinousAction = std::function<void(ContinuousState const&)>;

	struct ActionStruct
	{
		Action doAction;
		double time_s{ 0 };
	};

	struct ContinousActionStruct
	{
		ContinousAction doAction;
		double time_s{ 0 };
		double duration_s{ 0 };
		bool is_running{ false };
	};

	struct DeltaTime
	{
		double value{ 0.0 };
	};

	struct GlobalTime
	{
		double value{ 0.0 };
	};

	class ActionInvoker
	{
	public:

		void update(GlobalTime global_time_s)
		{
			_currentTime = global_time_s.value;
			updateInternal();
		}

		void update(DeltaTime delta_time_s)
		{
			_currentTime += delta_time_s.value;
			updateInternal();
		}

		bool containsDeferredActions() const
		{
			return _deferred_actions.empty() == false || _deferred_continous_actions.empty() == false;
		}

		void addDeferredAction(Action action, DeltaTime in_delta_time_s)
		{
			double time_s = _currentTime + in_delta_time_s.value;
			_deferred_actions.emplace_back(ActionStruct{ action,time_s });
		}

		void addDeferredAction(Action action, GlobalTime time_s)
		{
			_deferred_actions.emplace_back(ActionStruct{ action,time_s.value });
		}

		void addContinuousDeferredAction(ContinousAction action, DeltaTime in_delta_time_s, double duration_s)
		{
			double time_s = _currentTime + in_delta_time_s.value;
			_deferred_continous_actions.emplace_back(ContinousActionStruct{ action,time_s,duration_s });
		}

		void addContinuousDeferredAction(ContinousAction action, GlobalTime time_s, double duration_s)
		{
			_deferred_continous_actions.emplace_back(ContinousActionStruct{ action,time_s.value,duration_s });
		}


	private:

		void updateInternal()
		{
			ranges::for_each(_deferred_actions, std::bind(&ActionInvoker::executeActionIfExpired, this, std::placeholders::_1));
			ranges::erase_if(_deferred_actions, std::bind(&ActionInvoker::actionExpired, this, std::placeholders::_1));

			ranges::for_each(_deferred_continous_actions, std::bind(&ActionInvoker::executeActionIfIsInProgress, this, std::placeholders::_1));
			ranges::erase_if(_deferred_continous_actions, std::bind(&ActionInvoker::continuousActionExpired, this, std::placeholders::_1));
		}

		bool actionExpired(ActionStruct const& action)
		{
			return action.time_s < _currentTime;
		}

		void executeActionIfExpired(ActionStruct const& action)
		{
			if (actionExpired(action))
			{
				action.doAction();
			}
		}

		bool continuousActionExpired(ContinousActionStruct const& action)
		{
			return (action.time_s + action.duration_s) < _currentTime;
		}

		void executeActionIfIsInProgress(ContinousActionStruct& action)
		{
			if (_currentTime > action.time_s)
			{
				double timeInProgress = _currentTime - action.time_s;

				ContinuousState status;
				status.progress = std::clamp(timeInProgress / action.duration_s, 0.0, 1.0);
				status.is_last_pass = timeInProgress >= action.duration_s;
				status.is_first_pass = action.is_running == false;

				action.is_running = true;
				action.doAction(status);
			}
		}

		double _currentTime{ 0.0 };
		std::vector<ActionStruct> _deferred_actions;
		std::vector<ContinousActionStruct> _deferred_continous_actions;
	};

}