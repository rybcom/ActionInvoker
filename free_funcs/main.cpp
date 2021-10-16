
#include <iostream>
#include <thread>
#include "action_invoker.h"

namespace actions
{
	void action_hello()
	{
		std::cout << __FUNCTION__;
		std::cout << '\n';
	}

	void action_first()
	{
		std::cout << __FUNCTION__;
		std::cout << '\n';
	}

	void continuous_action_test(action_invoker::ContinuousState const& state)
	{
		if (state.is_first_pass)
		{
			std::cout << __FUNCTION__ << " - first execution ";
			std::cout << '\n';
		}
		else if (state.is_last_pass)
		{
			std::cout << __FUNCTION__ << " - last execution ";
			std::cout << '\n';
		}

		std::cout << __FUNCTION__ << " - progress : " << state.progress;
		std::cout << '\n';
	}
}

void create_actions()
{
	using namespace actions;

	action_invoker::add_deferred_action_on_time(action_hello, 1);
	action_invoker::add_deferred_action_on_time(action_first, 2);
	action_invoker::add_deferred_continous_action_on_time(continuous_action_test, 3,5);
}

int main()
{
	create_actions();

	using namespace std::chrono_literals;
	double time_s{ 0 };

	while (time_s < 100)
	{
		std::this_thread::sleep_for(0.0166666666666s);
		time_s += 0.0166666666666;

		action_invoker::process_deferred_actions(time_s);
	}
}
