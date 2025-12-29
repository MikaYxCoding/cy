#include "CY/safety.hpp"
#include "CY/types.hpp"
#include <cassert>

// A function to return a const& to an element inside a vector, we find the
// element by using ==.
template<typename T>
cy::Result<T const&> FindInVector(std::vector<T> const& vec, T const& value)
{
    for (auto const& item : vec)
        if (item == value)
            return cy::Ok<T const&>(item);

    return cy::Err("could not find item in vector");
}

int32 main(void)
{
    std::vector<int32> ints = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    if (auto result = FindInVector(ints, 10)) {
        assert(result.is_ok() && !result.is_error());
        std::printf("Got the value: %i\n", result.unwrap());

        // let's try to unwrap again, this should fail.
        try {
            auto _ = result.unwrap();
        } catch (cy::bad_result error) {
            std::printf("Caught an (expected) error: %s\n", error.what());
        }
    } else {
        assert(!result.is_ok() && result.is_error());
        std::printf("Error ocurred: %s\n", result.unwrap_err());
    }

    // let's try one that doesn't exist in the list.
    auto result = FindInVector(ints, 24);
    assert(result.is_error() && !result.is_ok());
    std::printf("Got an (expected) error: %s\n", result.get_err());

    // let's try unwrapping it, it should fail.
    try {
        auto _ = result.unwrap();
    } catch (cy::unwrap_on_err_error err) {
        std::printf("Caught an (expected) error: %s\n", err.what());
    }

    std::printf("All good!\n");
    return 0;
}