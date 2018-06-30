#pragma once
#include <type_traits>
namespace jua::detail
{
	template<int Item, int tf, int...Items>
	struct vfinder
	{
		static constexpr bool value = std::conditional<Item == tf, std::true_type,
			std::conditional<(sizeof...(Items) > 0), vfinder<Item, Items...>, std::false_type>::type>::type::value;
	};


	template<int idx, int Item, int tf, int...Items>
	struct ifinder
	{
		static constexpr int indx = std::conditional<Item == tf, std::integral_constant<int, idx>,
			std::conditional<
			(sizeof...(Items) > 0),
			std::integral_constant<int, ifinder<(idx + 1), Item, Items...>::indx>::type,
			std::integral_constant<int, -1>::type
		>::type
			>::type::value;
	};

	template<int Item, int tf, int...Items>
	struct cfinder
	{
		static constexpr int value =
			std::conditional<Item == tf, std::integral_constant<int, 1>::type, std::integral_constant<int, 0>::type>::type::value +
			std::conditional<(sizeof...(Items) > 0), std::integral_constant<int, cfinder<Item, Items...>::value>::type,
			std::integral_constant<int, 0>>::type::value;
	};

	template<int...Args>
	struct var_list
	{
	private:
		template<size_t, class T> struct Select;

		template<size_t offset, size_t...indx>
		struct Select<offset, std::index_sequence<indx...>>
		{
			using type = var_list<std::get<offset + indx>(std::make_tuple(Args...))...>;
		};
	public:
		static constexpr auto size = sizeof...(Args);


		template<int...NewArgs>
		using push_back = var_list<Args..., NewArgs...>;

		template<int...NewArgs>
		using push_front = var_list<NewArgs..., Args...>;

		template<size_t Index>
		static constexpr auto item = std::get<Index>(std::make_tuple(Args...));


		template<int Item>
		static constexpr bool contains = vfinder<Item, Args...>::value;

		template<int Item>
		static constexpr int count = cfinder<Item, Args...>::value;

		template<int Item>
		static constexpr int position = ifinder<0, Item, Args...>::indx;

		using clean = var_list<>;


	};
	template<typename T, typename Arg>
	auto empl_impl(Arg art)
	{
		if constexpr(std::is_same_v<T, Arg>)
		{
			return art;
		}
		else if constexpr(std::is_constructible_v<T, Arg>)
		{
			return T(art);
		}
		else {
			static_assert(false, "This is not constructible")
		}
	}
	template<typename T, typename...Args>
	void emplace_by(std::vector<T>&v, Args&&...args)
	{
		(v.emplace_back(std::forward<T>(empl_impl<T, Args>(args))), ...);
	}
	template<typename T, typename...Args>
	void emplace(std::vector<T>& v, Args&&...args)
	{
		(v.emplace_back(std::forward<Args>(args)), ...);
	}
}