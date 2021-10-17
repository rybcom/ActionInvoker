#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>


#pragma region macros

#define DECLARE_COLLECTION_CHAINING_OPERATOR(type, return_type) \
	template <typename Container, typename T> \
	return_type operator|(Container&& c, type<T>&& f) \
	{																\
		return f(default_container<T>::init_from(c));				\
	}																\

#define DECLARE_COLLECTION_CHAINING_OPERATOR_2TYPES(type) \
	template <typename Container, typename T1, typename T2> \
	default_container<T2>  operator|(Container&& c, type<T1,T2>&& f) \
	{																\
		return f(default_container<T1>::init_from(c));				\
	}	

#define DECLARE_COLLECTION_CHAINING_OPERATOR_3TYPES(type) \
	template <typename Container, typename T1, typename T2,typename T3> \
	default_container<T3>  operator|(Container&& c, type<T1,T2,T3>&& f) \
	{																\
		return f(default_container<T1>::init_from(c));				\
	}	

#define if_ref_wrapper(wrapper,type) \
	if(type & wrapper ##_value = wrapper->get(); wrapper)

#define if_cont_ref_wrapper(wrapper,type) \
	if(const type & wrapper ##_value = wrapper->get(); wrapper)

#define if_get_ref(pointer) \
	if(std::remove_pointer_t<decltype(pointer)> & pointer##_ref = *pointer; pointer)

#define if_get_cont_ref(pointer) \
	if(std::remove_pointer_t<decltype(pointer)> const & pointer##_const_ref = *pointer; pointer)

#pragma endregion

namespace views
{

#pragma region types

	template<class T>
	class default_container : public std::vector<T>
	{
	public:
		template<typename CastType>
		default_container<CastType*> cast_to()
		{
			default_container<CastType*> result_list;
			result_list.reserve(this->size());

			for (size_t i = 0; i < this->size(); i++)
			{
				CastType* casted = dynamic_cast<CastType*>(this->at(i));
				if (casted)
				{
					result_list.push_back(casted);
				}
			}

			return result_list;
		}

		default_container(std::size_t count, T const& value)
			: std::vector(count, value)
		{}

		default_container() = default;
		~default_container() = default;


		template< template <typename, typename...> class Container, typename...Args>
		static default_container<T*>  init_from(Container<T*, Args...> const& c)
		{
			default_container<T*> result_list;
			result_list.reserve(c.size());

			for (auto* item : c)
			{
				result_list.push_back(item);
			}

			return result_list;
		}


	

		template< template <typename, typename...> class Container, typename...Args>
		static default_container<T*>  init_from(Container<T, Args...> const& c)
		{
			default_container<T*> result_list;
			result_list.reserve(c.size());

			for (T const& item : c)
			{
				result_list.push_back(const_cast<T*>(&item));
			}

			return result_list;
		}

		template<typename Key, template <typename, typename...> class Container, typename...Args>
		static default_container<T*>  init_from(Container<Key, T, Args...> const& c)
		{

			default_container<T*> result_list;
			result_list.reserve(c.size());

			for (auto&& [key, item] : c)
			{
				result_list.push_back(const_cast<T*>(&item));
			}
			return result_list;
		}

		template< template <typename, typename...> class Container, typename...Args>
		static default_container<T*>  init_from(Container<std::unique_ptr<T>, Args...> const& c)
		{
			default_container<T*> result_list;
			result_list.reserve(c.size());

			for (auto& item : c)
			{
				result_list.push_back(item.get());
			}

			return result_list;
		}

		template< template <typename, typename...> class Container, typename...Args>
		static default_container<T*>  init_from(Container<std::reference_wrapper<T>, Args...> const& c)
		{
			default_container<T*> result_list;
			result_list.reserve(c.size());

			for (T& item : c)
			{
				result_list.push_back(&item);
			}

			return result_list;
		}
	};

	template< typename T>
	class list : public std::vector<T>
	{
	public:

		list() = default;

		list(const list& c)
		{
			(*this).reserve(c.size());

			for (T* item : c)
			{
				this->push_back(*item);
			}
		}

		list(const default_container<T*>&& c)
		{
			(*this).reserve(c.size());

			for (T* item : c)
			{
				this->push_back(*item);
			}
		}
	};

	template< typename T>
	class ref_list : public std::vector<std::reference_wrapper<T>>
	{
	public:

		ref_list() = default;

		ref_list(const ref_list& x) = default;

		ref_list(const default_container<T*>&& c)
		{
			(*this).reserve(c.size());

			for (T* item : c)
			{
				this->push_back(*item);
			}
		}
	};

#pragma  endregion

#pragma region range adaptors

	template<class T>
	struct make_view
	{
		template<typename T>
		list<T> operator()(default_container<T*> const&& rng) const
		{
			list<T> result_list;
			result_list.reserve(rng.size());

			for (T const* item : rng)
			{
				result_list.push_back(*item);
			}

			return result_list;
		}
	};

	template<class T>
	struct make_view_ref
	{
		template<typename T>
		ref_list<T> operator()(default_container<T*> const&& rng) const
		{
			ref_list<T> result_list;
			result_list.reserve(rng.size());

			for (T const* item : rng)
			{
				result_list.push_back(*item);
			}

			return result_list;
		}
	};

	template<class T>
	struct  indexed_pair
	{
		T* value;
		std::size_t index;
	};

	template<class T>
	struct indexed
	{
		indexed(std::size_t index_from)
			:
			_initIndex(index_from)
		{
		}

		default_container<indexed_pair<T>> operator()(default_container<T*> const&& rng) const
		{
			default_container<indexed_pair<T>> result_list;
			result_list.reserve(rng.size());

			std::size_t index = _initIndex;
			for (T* item : rng)
			{
				result_list.push_back({ item,index });
				index++;
			}

			return result_list;
		}

	private:

		std::size_t _initIndex;
	};

	template<class T>
	struct count
	{
		std::size_t operator()(default_container<T*> const&& rng) const
		{
			return rng.size();
		}
	};

	template<class T>
	struct count_if
	{

		count_if(std::function<bool(T&)> f)
		{
			_predicate = f;
		}

		std::size_t operator()(default_container<T*> const&& rng) const
		{
			std::size_t count = 0;

			for (T* item : rng)
			{
				if (_predicate(*item))
				{
					count++;
				}
			}
			return count;
		}

	private:
		std::function<bool(T&)> _predicate;

	};

	template<class T>
	struct none_of
	{
		std::function<bool(T const&)> predicate;

		none_of(std::function<bool(T const&)> f)
		{
			predicate = f;
		}

		template<typename T>
		bool operator()(default_container<T*>&& rng) const
		{

			for (T const* item : rng)
			{
				if (predicate(*item))
				{
					return false;
				}

			}

			return true;
		}
	};

	template<class T>
	struct all_of
	{
		std::function<bool(T const&)> predicate;

		all_of(std::function<bool(T const&)> f)
		{
			predicate = f;
		}

		template<typename T>
		bool operator()(default_container<T*>&& rng) const
		{

			for (T const* item : rng)
			{
				if (predicate(*item) == false)
				{
					return false;
				}

			}

			return true;
		}
	};

	template<class T>
	struct any_of
	{
		std::function<bool(T const&)> predicate;

		any_of(std::function<bool(T const&)> f)
		{
			predicate = f;
		}

		template<typename T>
		bool operator()(default_container<T*>&& rng) const
		{

			for (T const* item : rng)
			{
				if (predicate(*item))
				{
					return true;
				}
			}

			return false;
		}
	};

	template<class T>
	struct filter
	{
		std::function<bool(T const&)> predicate;

		filter(std::function<bool(T const&)> f)
		{
			predicate = f;
		}




		template<typename T>
		default_container<T*> operator()(default_container<T*>&& rng) const
		{
			default_container<T*> result_list;
			result_list.reserve(rng.size());

			for (T const* item : rng)
			{
				if (predicate(*item))
				{
					result_list.push_back(const_cast<T*>(item));
				}
			}
			return result_list;
		}
	};



	template<class T>
	struct for_each
	{
		std::function<void(T&)> func;

		for_each(std::function<void(T&)> f)
		{
			func = f;
		}

		template<typename T>
		default_container<T*> operator()(default_container<T*> const&& rng) const
		{
			for (T* item : rng)
			{
				func(*item);
			}
			return rng;
		}
	};

	template<class T>
	struct get_first
	{
		T* operator()(default_container<T*> const&& rng) const
		{
			for (T* item : rng)
			{
				return item;
			}
			return {};
		}
	};

	template<class T>
	struct find_if
	{

		find_if(std::function<bool(T const&)> f)
		{
			_predicate = f;
		}

		template<typename T>
		T* operator()(default_container<T*> const&& rng) const
		{

			for (T const* item : rng)
			{
				if (_predicate(*item))
				{
					return const_cast<T*>(item);
				}
			}
			return {};
		}

	private:
		std::function<bool(T const&)> _predicate;

	};


	template<class T>
	find_if(bool(*)(T const&))->find_if<T>;


	template<class T>
	struct find
	{

		find(T const& v) : _value(v) {}

		template<typename T>
		T* operator()(default_container<T*> const&& rng) const
		{

			for (T const* item : rng)
			{
				if (*item == _value)
				{
					return const_cast<T*>(item);
				}
			}
			return {};
		}

	private:
		T const& _value;
	};

	template<class T>
	struct max_element
	{
		max_element(std::function<bool(T const&, T const&)> f)
		{

#ifdef false //_HAS_CXX17

			_predicate = std::not_fn(f);

#else
			_predicate = [f](T const& a, T const& b)
			{
				return !f(a, b);
			};
#endif
		}

		max_element()
		{
			_predicate = std::greater<>();
		}

		T* operator()(default_container<T*> const&& rng) const
		{
			auto _First = rng.cbegin();
			auto _Found = _First;
			auto _Last = rng.cend();

			if (_First == _Last)
			{
				return nullptr;
			}

			while (++_First != _Last)
			{
				if (_DEBUG_LT_PRED(_predicate, **_First, **_Found))
				{
					_Found = _First;
				}
			}

			return *(_Found);
		}

	private:

		std::function<bool(T const&, T const&)> _predicate;

	};

	template<class T>
	struct min_element
	{

		min_element(std::function<bool(T const&, T const&)> f)
		{
			_predicate = f;
		}

		min_element()
		{
			_predicate = std::less<>();
		}

		T* operator()(default_container<T*> const&& rng) const
		{
			auto _First = rng.cbegin();
			auto _Found = _First;
			auto _Last = rng.cend();


			if (_First == _Last)
			{
				return nullptr;
			}

			while (++_First != _Last)
			{
				if (_DEBUG_LT_PRED(_predicate, **_First, **_Found))
				{
					_Found = _First;
				}
			}

			return *(_Found);
		}

	private:

		std::function<bool(T const&, T const&)> _predicate;

	};


	template<class TypeIn, class TypeOut>
	struct transform
	{

		transform(std::function<TypeOut(TypeIn const&)> f)
		{
			_func = f;
		}

		default_container<TypeOut> operator()(default_container<TypeIn*>&& rng) const
		{
			default_container<TypeOut> result_list;
			result_list.reserve(rng.size());

			for (TypeIn const* item : rng)
			{
				result_list.push_back(_func(*item));
			}
			return result_list;
		}

	private:

		std::function<TypeOut(TypeIn const&)> _func;

	};

	template<class TypeInA, class TypeInB, class TypeOut>
	struct zip
	{
		template <typename ContainerB>
		zip(ContainerB const& cb, std::function<TypeOut(TypeInA const&, TypeInB const&)> f)
			: _func{ f }
		{
			_rng_B = views::default_container<TypeInB>::init_from(cb);
		}

		template <typename ContainerA, typename ContainerB>
		zip(ContainerA const& ca, ContainerB const& cb, std::function<TypeOut(TypeInA const&, TypeInB const&)> f)
			: _func{ f }
		{
			_rng_A = views::default_container<TypeInA>::init_from(ca);
			_rng_B = views::default_container<TypeInB>::init_from(cb);
		}

		default_container<TypeOut> operator()()
		{
			views::default_container<TypeOut> result_list;
			result_list.reserve(_rng_A.size());

			auto _First1 = _rng_A.cbegin();
			auto _First2 = _rng_B.cbegin();
			auto _Last1 = _rng_A.cend();
			auto _Last2 = _rng_B.cend();

			auto distance = std::min(std::distance(_First1, _Last1), std::distance(_First2, _Last2));

			for (int i = 0; i < distance; ++_First1, ++_First2, ++i)
			{
				TypeInA const& itemA = **_First1;
				TypeInB const& itemB = **_First2;
				result_list.push_back(_func(itemA, itemB));
			}


			return result_list;
		}

		default_container<TypeOut> operator()(
			default_container<TypeInA*> const& rng_A) const
		{
			views::default_container<TypeOut> result_list;
			result_list.reserve(_rng_A.size());

			auto _First1 = rng_A.cbegin();
			auto _First2 = _rng_B.cbegin();
			auto _Last1 = rng_A.cend();
			auto _Last2 = _rng_B.cend();

			auto distance = std::min(std::distance(_First1, _Last1), std::distance(_First2, _Last2));

			for (int i = 0; i < distance; ++_First1, ++_First2, ++i)
			{
				TypeInA const& itemA = **_First1;
				TypeInB const& itemB = **_First2;
				result_list.push_back(_func(itemA, itemB));
			}



			return result_list;
		}

	private:

		std::function<TypeOut(TypeInA const&, TypeInB const&)> _func;
		default_container<TypeInA*> _rng_A;
		default_container<TypeInB*> _rng_B;
	};


#pragma endregion

#pragma region operators


	DECLARE_COLLECTION_CHAINING_OPERATOR(make_view, list<T>)
	DECLARE_COLLECTION_CHAINING_OPERATOR(make_view_ref, ref_list<T>)

	DECLARE_COLLECTION_CHAINING_OPERATOR(indexed, default_container<indexed_pair<T>>)
	DECLARE_COLLECTION_CHAINING_OPERATOR(count, std::size_t)
	DECLARE_COLLECTION_CHAINING_OPERATOR(count_if, std::size_t)
	DECLARE_COLLECTION_CHAINING_OPERATOR(none_of, bool)
	DECLARE_COLLECTION_CHAINING_OPERATOR(all_of, bool)
	DECLARE_COLLECTION_CHAINING_OPERATOR(any_of, bool)
	DECLARE_COLLECTION_CHAINING_OPERATOR(get_first, T*)
	DECLARE_COLLECTION_CHAINING_OPERATOR(find, T*)
	DECLARE_COLLECTION_CHAINING_OPERATOR(find_if, T*)
	DECLARE_COLLECTION_CHAINING_OPERATOR(filter, default_container<T*>)
	DECLARE_COLLECTION_CHAINING_OPERATOR(for_each, default_container<T*>)
	DECLARE_COLLECTION_CHAINING_OPERATOR(max_element, T*)
	DECLARE_COLLECTION_CHAINING_OPERATOR(min_element, T*)

	DECLARE_COLLECTION_CHAINING_OPERATOR_2TYPES(transform)

	DECLARE_COLLECTION_CHAINING_OPERATOR_3TYPES(zip)


#pragma endregion

#pragma  region c++ 17 template deduction guides

	template<typename T>
	filter(bool(*)(T const&))->filter<T>;

	template<typename T>
	for_each(void(*)(T&))->for_each<T>;

	template<typename T>
	any_of(bool(*)(T const&))->any_of<T>;

	template<typename T>
	all_of(bool(*)(T const&))->all_of<T>;

	template<typename T>
	none_of(bool(*)(T const&))->none_of<T>;

#pragma  endregion

}

namespace ranges
{

	template <typename T, typename = void>
	struct has_push_back : std::false_type {};

	template <typename T>
	struct has_push_back<T,
		std::void_t<
		decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))
		>
	> : std::true_type {};


	template<typename Container, typename Operation>
	void for_each(Container&& c, Operation op)
	{
		auto begin = c.begin();
		auto end = c.end();

		for (; begin != end; begin++) {
			op(*begin);
		}
	}

	template<typename Container, typename Pred, typename Operation>
	void for_each_where(Container&& c, Pred p, Operation op)
	{
		auto begin = c.begin();
		auto end = c.end();

		for (; begin != end; begin++) {
			if (p(*begin)) {
				op(*begin);
			}
		}
	}

	template<typename T, template <typename, typename...> class Container, typename Operation, typename...Args>
	void for_each(Container<std::unique_ptr<T>&&, Args...> const& c, Operation op)
	{
		for (auto& refUniquePtr : c)
		{
			op(*refUniquePtr);
		}
	}

	template<typename T, template <typename, typename...> class Container, typename Pred, typename Operation, typename...Args>
	void for_each_where(Container<std::unique_ptr<T>&&, Args...> const& c, Pred p, Operation op)
	{
		for (auto& refUniquePtr : c)
		{
			T& refValue = *refUniquePtr;
			if (p(refValue))
			{
				op(refValue);
			}
		}
	}

	template <typename Container, typename Pred>
	auto filter(Container const& c, Pred p) {
		Container out;
		if constexpr (has_push_back<Container>::value)
		{
			std::copy_if(begin(c), end(c), std::back_inserter(out), p);
		}
		else
		{
			std::copy_if(begin(c), end(c), std::inserter(out, out.begin()), p);
		}

		return out;
	}


	template<typename Container, typename Value>
	size_t erase(Container&& c, Value val)
	{
		auto new_end = std::remove(c.begin(), c.end(), val);

		size_t count = std::distance(new_end, c.end());

		c.erase(new_end, c.end());

		return count;
	}

	template<typename Container, typename Pred>
	size_t erase_if(Container&& c, Pred predicate)
	{
		auto new_end = std::remove_if(c.begin(), c.end(), predicate);

		size_t count = std::distance(new_end, c.end());

		c.erase(new_end, c.end());

		return count;
	}

	template<typename T, typename Pred, typename Container>
	T* find_if(Container const& rng, Pred predicate)
	{
		for (T const* item : views::default_container<T>::init_from(rng))
		{
			if (predicate(*item))
			{
				return const_cast<T*>(item);
			}
		}
		return {};
	}

	template<typename T, typename Container>
	T* find(Container const& rng, T const& value)
	{
		for (T const* item : views::default_container<T>::init_from(rng))
		{
			if (*item == value)
			{
				return const_cast<T*>(item);
			}
		}
		return {};
	}

	template<typename T, typename Pred, typename Container>
	bool any_of(Container const& rng, Pred predicate)
	{
		for (T const* item : views::default_container<T>::init_from(rng))
		{
			if (predicate(*item))
			{
				return true;
			}
		}
		return false;
	}

	template<typename T, typename Pred, typename Container>
	bool all_of(Container const& rng, Pred predicate)
	{
		for (T const* item : views::default_container<T>::init_from(rng))
		{
			if (predicate(*item) == false)
			{
				return false;
			}
		}
		return true;
	}

	template<typename T, typename Pred, typename Container>
	bool none_of(Container const& rng, Pred predicate)
	{
		for (T const* item : views::default_container<T>::init_from(rng))
		{
			if (predicate(*item))
			{
				return false;
			}
		}
		return true;
	}

	template<typename TypeInA, typename TypeInB, typename TypeOut, typename Func, typename ContainerA, typename ContainerB>
	views::default_container<TypeOut> zip(ContainerA  const& c1, ContainerB const& c2, Func func)
	{
		auto rng_A = views::default_container<TypeInA>::init_from(c1);
		auto rng_B = views::default_container<TypeInB>::init_from(c2);

		views::default_container<TypeOut> result_list;
		result_list.reserve(rng_A.size());

		auto _First1 = rng_A.cbegin();
		auto _First2 = rng_B.cbegin();
		auto _Last1 = rng_A.cend();
		auto _Last2 = rng_B.cend();

		auto distance = std::min(std::distance(_First1, _Last1), std::distance(_First2, _Last2));

		for (int i = 0; i < distance; ++_First1, ++_First2, ++i)
		{
			TypeInA const& itemA = **_First1;
			TypeInB const& itemB = **_First2;
			result_list.push_back(func(itemA, itemB));
		}


		return result_list;
	}

	template<typename T>
	views::default_container<T> generate(std::size_t count, T const& init_value = T())
	{
		views::default_container<T> result_list{ count, init_value };

		return result_list;
	}

	template<typename T>
	views::default_container<T> iota(std::size_t count, T const& init_value = T())
	{
		views::default_container<T> result_list;
		result_list.reserve(count);

		T value = init_value;

		for (std::size_t i = 0; i < count; ++i)
		{
			result_list.push_back(value++);
		}

		return result_list;
	}

	template<typename ResultValueType, typename T, typename Container>
	ResultValueType accumulate(Container const& rng, ResultValueType initValue = ResultValueType{},
		std::function<ResultValueType(ResultValueType const&, T const&)>&& op = std::plus<>{})
	{
		ResultValueType result = initValue;

		for (T const* item : views::default_container<T>::init_from(rng))
		{
			result = op(result, *item);
		}

		return result;
	}



}


