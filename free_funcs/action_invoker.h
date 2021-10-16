#pragma once

#include <vector>
#include <algorithm>
#include <ranges>
#include <functional>

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

	inline double _currentTime{ 0 };
	inline std::vector<ActionStruct> deferred_actions;
	inline std::vector<ContinousActionStruct> deferred_continous_actions;

	bool action_expired(ActionStruct const& action)
	{
		return action.time_s < _currentTime;
	}

	void execute_action(ActionStruct const& action)
	{
		action.doAction();
	}

	bool continous_action_expired(ContinousActionStruct const& action)
	{
		return (action.time_s + action.duration_s) < _currentTime;
	}

	void execute_action_if_is_in_progress(ContinousActionStruct& action)
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

	void process_deferred_actions(double currentTime)
	{
		_currentTime = currentTime;

		std::ranges::for_each(deferred_actions | std::views::filter(action_expired), execute_action);
		std::erase_if(deferred_actions, action_expired);

		std::ranges::for_each(deferred_continous_actions, execute_action_if_is_in_progress);
		std::erase_if(deferred_continous_actions, continous_action_expired);
	}

	void add_deferred_action_on_time(Action action, double time_s)
	{
		deferred_actions.emplace_back(ActionStruct{ action,time_s });
	}

	void add_deferred_continous_action_on_time(ContinousAction action, double time_s, double duration_s)
	{
		deferred_continous_actions.emplace_back(ContinousActionStruct{ action,time_s,duration_s });
	}

}