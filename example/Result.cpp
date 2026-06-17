#include "CY/result.hpp"
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

// More involved example with a custom error.
enum class NetworkError
{
    ConnectionLost,
    ResourceNotFound
};
namespace cy {
template<>
struct error_formatter<NetworkError>
{
    static constexpr auto readable(NetworkError const& err)
    {
        switch (err) {
            case NetworkError::ConnectionLost:
                return "Connection lost";
            case NetworkError::ResourceNotFound:
                return "Resource not found";
        }
    }
};
} // namespace cy

// Alias for convenience
template<typename T>
using NetworkResult = cy::result::Result<T, NetworkError>;

static NetworkResult<str> GetResourceById(uint32 id)
{
    str resources[] = { "Resource 1", "Resource 2", "Resource 3",
                        "Resource 4", "Resource 5", "Resource 6" };

    // *clearly doing networking here*

    if (id >= _countof(resources))
        return cy::Err(NetworkError::ResourceNotFound);

    return cy::Ok(resources[id]);
}

static NetworkResult<void> SendPing()
{
    // *clearly sending stuff here*

    // oops!
    return cy::Err(NetworkError::ConnectionLost);
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

    // the other example
    if (auto result = GetResourceById(3))
        std::printf("Got our resource: %s\n", result.unwrap());
    else {
        // might as well use our error_formatter
        std::printf(
            "Got an error: %s\n",
            cy::error_formatter<NetworkError>::readable(result.unwrap_err()));
    }

    // one that fails
    auto resourceResult = GetResourceById(24);
    try {
        std::printf("Somehow got our resource: %s\n", resourceResult.unwrap());
    } catch (cy::unwrap_on_err_error err) {
        std::printf("Caught an (expected) error: %s\n", err.what());
    }

    auto pingResult = SendPing();
    assert(pingResult.is_error() && !pingResult.is_ok());
    std::printf(
        "Got this error: %s\n",
        cy::error_formatter<NetworkError>::readable(pingResult.get_err()));

    try {
        pingResult.unwrap();
    } catch (cy::unwrap_on_err_error e) {
        std::printf("Caught an (expected) error: %s\n", e.what());
    }

    std::printf("All good!\n");
    return 0;
}
