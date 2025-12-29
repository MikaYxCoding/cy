#include "CY/safety.hpp"
#include "CY/types.hpp"
#include <cassert>

// A function to return a const& to an element inside a vector, we find the
// element by using ==.
template<typename T>
cy::Maybe<T const&> FindInVector(std::vector<T> const& vec, T const& value)
{
    for (auto const& item : vec)
        if (item == value)
            return cy::Some<T const&>(item);

    return cy::None();
}

int32 main(void)
{
    std::vector<int32> ints = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    auto number = FindInVector(ints, 10);
    assert(number.is_some() && !number.is_none());
    assert(number.unwrap() == 10);

    // Let's try one that fails!
    auto anotherNumber = FindInVector(ints, 24);
    assert(anotherNumber.is_none() && !anotherNumber.is_some());
    try {
        int32 number = anotherNumber.unwrap();
        std::printf("Somehow got the number %i!\n", number);
    } catch (cy::unwrap_none_error const& error) {
        std::printf("Caught an (expected) error: %s\n", error.what());
    }

    std::printf("All good!\n");
    return 0;
}