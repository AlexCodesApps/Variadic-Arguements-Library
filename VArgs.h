/*
    C++ Templated Variadic Library By Alex Adewole.
    I Designed To Be Somewhat Robust, Although May Be Lacking Features.
    Made In About The Span Of A Day.
    Supports C++14 And Above, With Partial Support For C++11.
    Would Like To Recieve Feedback.
    
    Unavailable VArgs Extensions For C++ 11, [ForEach, ArrayCollapse, FunctionArrayCollapse]
*/
#ifndef VARGS_H_CPP
#define VARGS_H_CPP
#ifndef __cplusplus
#error C Is Unsupported
#else
#if (__cplusplus < 201103L)
#error Must Be Using C++ 11 Or Above
#endif
#include <cstddef> // For std::size_t
#include <array> // For Array Collapse Functions
#include <exception> // For UniformValueAtIndex Runtime Exception
// define DISABLE_RUNTIME_EXCEPTION To Disable Exceptions
namespace VArgs {
// DNU. Prefer Length() Instead.
template <typename ...Args>
struct InternalLength {};
// DNU. Prefer Length() Instead.
template <>
struct InternalLength<> {
    static constexpr std::size_t value = 0;
};
// DNU. Prefer Length() Instead.
template <typename T, typename ...Args>
struct InternalLength<T, Args...> {
    static constexpr std::size_t value = 1 + InternalLength<Args...>::value;
};
/* 
Used To Get The Length Of A Variadic Array. 
In C++20 sizeof...(Args) Can Be Used 
But This Function Maintains Backwards Compatability 
*/
template <typename ...Args>
constexpr std::size_t Length() {
    return InternalLength<Args...>::value;
}
/* 
Used To Get The Length Of A Variadic Array. 
In C++20 sizeof...(Args) Can Be Used 
But This Function Maintains Backwards Compatability 
*/
template <typename ...Args>
constexpr std::size_t Length(Args... args) {
    return InternalLength<Args...>::value;
}
// Get The Type Of A Specified Index, Useful for Writing Variadic Type Checking Or Variadic Manipulation Functions
template <std::size_t Index, typename ...Args>
struct TypeAtIndex{
    static_assert(Index < Length<Args...>(), "Index Must Be Less Than Length");
};
// Get The Type Of A Specified Index, Useful for Writing Variadic Type Checking Or Variadic Manipulation Functions
template <std::size_t Index, typename F, typename ...Args>
struct TypeAtIndex<Index, F, Args...> {
    static_assert(Index > 0, "Index Must Be Greater Than 0");
    using type = typename TypeAtIndex<Index - 1, Args...>::type;
};
// Get The Type Of A Specified Index, Useful for Writing Variadic Type Checking Or Variadic Manipulation Functions
template <typename F, typename ...Args>
struct TypeAtIndex<0, F, Args...> {
    using type = F;
};
// Get The Value Of A Specified Index, Can Only Function Constexpr, Use ArrayCollapse To Dynamically Get Values Instead If Possible
template <std::size_t Index, typename ...Args>
struct ValueAtIndex {};
// Get The Value Of A Specified Index, Can Only Function Constexpr, Use ArrayCollapse To Dynamically Get Values Instead If Possible
template <typename T, typename ...Args>
struct ValueAtIndex<0, T, Args...> {
    static constexpr T Get(T val, Args&&... args) {return val;}
};
// Get The Value Of A Specified Index, Can Only Function Constexpr, Use ArrayCollapse To Dynamically Get Values Instead If Possible
template <std::size_t Index, typename T, typename ...Args>
struct ValueAtIndex<Index, T, Args...> {
    static_assert(Index != 0, "Should Be Unreachable Error");
    using type = typename TypeAtIndex<Index-1, Args...>::type;
    static constexpr type Get(T val, Args&&... args) {
        return ValueAtIndex<Index - 1, Args...>::Get(std::forward<Args>(args)...);
    }
};
// Check If List Is Of A Uniform Type.
template <typename ...Args>
struct ListUniform {
    constexpr static bool value = false;
};
// Check If List Is Of A Uniform Type.
template <typename T, typename ...Args>
struct ListUniform<T, T, Args...> {
    constexpr static bool value = ListUniform<T, Args...>::value;
};
// Check If List Is Of A Uniform Type.
template <typename T>
struct ListUniform<T> {
    constexpr static bool value = true;
};
// Check If The List Is Only Of A Specified Type
template <typename T, typename ...Args>
struct ListIsOfType {
    constexpr static bool value = ListUniform<T, Args...>::value;
};

template <typename ...Args>
struct InternalUniformValueAtIndex {};
// DNU. Prefer UniformTypeAtIndex() Instead
template <typename T, typename ...Args>
struct InternalUniformValueAtIndex<T, Args...> {
    static_assert(Length<Args...>() > 0, "SFINAE Construct Should Be Invisible");
    static T Get(std::size_t index, T first, Args&&... args) {
        return (index == 0) ? first : InternalUniformValueAtIndex<Args...>::Get(index - 1, std::forward<Args>(args)...);
    }
};
// DNU. Prefer UniformTypeAtIndex() Instead
template <typename T>
struct InternalUniformValueAtIndex<T> {
    static T Get(std::size_t index, T val) {
        #ifndef DISABLE_RUNTIME_EXCEPTION
        #define VARGS_H_CPP_UNIFORMVALUEATINDEX_EXCEPTION
        if (index != 0) throw std::runtime_error("Index Out Of Bounds");
        #endif
        return val;
    }
};

// Get Value At Index Of Uniform List At Runtime
template <typename ...Args>
constexpr typename TypeAtIndex<0, Args...>::type UniformValueAtIndex(std::size_t index, Args&&... args) {
    return InternalUniformValueAtIndex<Args...>::Get(index, std::forward<Args>(args)...);
}

#if (__cplusplus >= 201402L)
#define VARGS_H_CPP_ARRAY_ITERATORS
// DNU. Prefer ArrayCollapse Or FunctionArrayCollapse Instead.
template <typename ...>
struct InternalArrayCollapse {};
// DNU. Prefer ArrayCollapse Or FunctionArrayCollapse Instead.
template <typename First, typename ...Args>
struct InternalArrayCollapse<First, Args...> {
    static_assert(Length<Args...>() > 0, "Should Be Unreachable Error");
    static_assert(ListUniform<First, Args...>::value, "List Must Be Of Single Type");
    static constexpr std::size_t argslen = 1 + Length<Args...>();
    static constexpr std::array<First, argslen> Collapse(First t, Args&&... args) {
        std::array<First, argslen> nArr = {t, std::forward<Args>(args)...};
        return nArr;
    }
};
// DNU. Prefer ArrayCollapse Or FunctionArrayCollapse Instead.
template <typename Only>
struct InternalArrayCollapse<Only> {
    static constexpr std::array<Only, 1> Collapse(Only t) {
        return std::array<Only, 1>({std::forward<Only>(t)});
    }
};

// Collapse A Uniform List Of Arguements Into An Actionable List
template <typename ...Args>
constexpr std::array<typename TypeAtIndex<0, Args...>::type, Length<Args...>()> ArrayCollapse(Args... args) {
    static_assert(ListUniform<Args...>::value, "List Must Be Of Single Type, "
    "Use FunctionArrayCollapse To Transform The List Into A Singly Typed Array Instead");
    return InternalArrayCollapse<Args...>::Collapse(std::forward<Args>(args)...);
}
// DNU. Prefer ForEach()
template <typename Func, typename ...Args>
struct InternalForEach {};
// DNU. Prefer ForEach()
template <typename Func, typename T, typename ...Args>
struct InternalForEach<Func, T, Args...> {
    static constexpr void Operation(Func func, T val, Args&&... args) {
        func(val);
        InternalForEach<Func, Args...>::Operation(func, std::forward<Args>(args)...);
    }
};
// DNU. Prefer ForEach()
template <typename Func>
struct InternalForEach<Func> {
    static constexpr void Operation(Func func) {}
};

// Use VArgs_PassUnExpanded(x) Macro To Pass Templated Functions
template <typename Func, typename ...Args>
void ForEach(Func func, Args&&... args) {
    InternalForEach<Func, Args...>::Operation(func, std::forward<Args>(args)...);
}
// MACROS CANT BE IN NAMESPACES :(
#define VArgs_PassUnExpanded(x) [](auto c){x(c);}
// Capable Of Transforming Polymorphic Arguements
template <typename T, typename Func, typename ...Args>
constexpr std::array<T, Length<Args...>()> FunctionArrayCollapse(Func func, Args&&... args) {
    constexpr std::size_t argslen = Length<Args...>();
    std::array<T, argslen> newarr;
    std::size_t csize = 0;
    ForEach([&](auto arg) {newarr[csize++] = func(arg);});
    return newarr;
}
// Unavailable VArgs Extensions For C++ 11, [ForEach, ArrayCollapse, FunctionArrayCollapse]
#endif
}
#endif
#endif
