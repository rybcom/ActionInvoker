
#include <iostream>
#include <thread>
#include "ActionInvoker.h"

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
		else
		{
			std::cout << __FUNCTION__ << " - progress : " << state.progress;
			std::cout << '\n';
		}

	}
}

void create_actions(action_invoker::ActionInvoker& invoker)
{
	using namespace actions;

	invoker.addDeferredAction(action_hello, 1);
	invoker.addDeferredAction(action_first, 2);
	invoker.addContinuousDeferredAction(continuous_action_test, 3, 5);
}

int main()
{
	using namespace action_invoker;

	ActionInvoker invoker;
	create_actions(invoker);

	using namespace std::chrono_literals;
	double time_s{ 0 };

	while (time_s < 100)
	{
		std::this_thread::sleep_for(0.0166666666666s);
		time_s += 0.0166666666666;

		invoker.update(time_s);
	}
}
