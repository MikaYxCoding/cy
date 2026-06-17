<div align="center">
<h1>CY</h1>

An utility library for C++.
</div>

----------


### Features
1. Custom typenames.
2. A value/none (``Some<T>/None``) class implementation (``Maybe<T>``).
3. A value/error (``Ok<T>/Err<E>``) class implementation (``Result<T, E>``)
4. A custom `error_formatter<E>` that you can specialize to provide readable errors when something goes wrong.

Both `Result` and `Maybe` are implemented to be as trivial as possible and to minimize copying, meaning their performance cost should be negligible.
They are also very convenient. Both types allow you to use references without any reference wrappers, and `Result<void, E>` is naturally accepted without any problems.

### Usage
Since CY is just header files, you can clone this repo (or download the source code) and include the `include/` directoy in your project's search paths.

If you want to try the tests (which are used to debug CY) you can use cmake to compile them.

Some examples:
```cpp
// Maybe

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

// Result. This is using the most common alias `Result<T, str>`. You can of course use you own error types with `cy::result::Result<T, E>`.

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
```

```cpp
// Since `Result` can be implicitly converted to a boolean, we can do cool stuff like this:

// 1. If statement
if (auto result = FnReturningResult()) { // capture the result and also check if is `Ok`!
  auto value = result.unwrap();
}

// 2. Early return
if (auto result = FnReturningResult(); !result) { // capture the result and use it in a condition! This one is equivalent to Result::is_error
  auto err = result.unwrap_err();
  // handle error
  return;
}
```

```cpp
// You can quickly check a predicate on a `Result` and `Maybe`.

cy::Result<int, str> resultTest {Ok(10)};
cy::Maybe<int> maybeTest {Some(10)};

if (resultTest.is_ok_and([](int v) { return v > 4; })) {
  // ...
}

if (maybeTest.is_some_and([](int v) { return v > 4; })) {
  // ...
}
```

# License
This project is licensed under the MIT license. Please check [LICENSE](LICENSE) for more details.
