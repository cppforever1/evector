#include "eunsortedvector.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

void check(bool condition)
{
    if (!condition) {
        throw std::runtime_error("eunsortedvector test failed");
    }
}

template<typename T>
void equal(const eunsortedvector<T>& actual, const std::vector<T>& expected)
{
    check(actual.size() == expected.size());
    check(std::equal(actual.begin(), actual.end(), expected.begin()));
}

template<typename T>
struct Allocator
{
    typedef T value_type;
    int id;
    explicit Allocator(int value = 0) : id(value) {}
    template<typename U>
    Allocator(const Allocator<U>& other) : id(other.id) {}
    T* allocate(std::size_t n) { return std::allocator<T>().allocate(n); }
    void deallocate(T* ptr, std::size_t n) { std::allocator<T>().deallocate(ptr, n); }
    template<typename U>
    bool operator==(const Allocator<U>& other) const { return id == other.id; }
    template<typename U>
    bool operator!=(const Allocator<U>& other) const { return !(*this == other); }
};

void constructors_and_allocators()
{
    equal(eunsortedvector<int>(), std::vector<int>());
    equal(eunsortedvector<int>(3), std::vector<int>(3));
    equal(eunsortedvector<int>(3, 7), std::vector<int>(3, 7));
    eunsortedvector<int> values{1, 2, 3};
    equal(eunsortedvector<int>(values.begin(), values.end()), std::vector<int>{1, 2, 3});
    eunsortedvector<int> copy(values);
    eunsortedvector<int> moved(std::move(copy));
    check(moved == values);
    copy = values;
    copy = std::move(moved);
    check(copy == values);
    copy = {9, 8};
    equal(copy, std::vector<int>{9, 8});
    std::istringstream input("4 5 6");
    eunsortedvector<int> streamed((std::istream_iterator<int>(input)), std::istream_iterator<int>());
    equal(streamed, std::vector<int>{4, 5, 6});

    typedef Allocator<int> A;
    typedef eunsortedvector<int, A> V;
    V a(A(1));
    V b(3, A(2));
    V c(3, 7, A(3));
    V d(c.begin(), c.end(), A(4));
    V e({1, 2}, A(5));
    V f(e, A(6));
    V g(std::move(f), A(7));
    check(a.get_allocator().id == 1 && b.get_allocator().id == 2);
    check(c.get_allocator().id == 3 && d.get_allocator().id == 4);
    check(e.get_allocator().id == 5 && g.get_allocator().id == 7);
    check(g.size() == 2 && g[1] == 2);
    d = std::move(g);
    check(d.get_allocator().id == 4 && d.size() == 2 && d[1] == 2);
}

void modifiers()
{
    eunsortedvector<int> v;
    std::vector<int> s;
    v.assign(3, 5); s.assign(3, 5); equal(v, s);
    int items[] = {1, 2, 3};
    v.assign(items, items + 3); s.assign(items, items + 3); equal(v, s);
    v.assign({4, 5}); s.assign({4, 5}); equal(v, s);
    int value = 6;
    check(*v.insert(v.cbegin(), value) == *s.insert(s.cbegin(), value)); equal(v, s);
    check(*v.insert(v.cbegin(), 7) == *s.insert(s.cbegin(), 7)); equal(v, s);
    check(*v.insert(v.cend(), 2, 8) == *s.insert(s.cend(), 2, 8)); equal(v, s);
    check(*v.insert(v.cbegin(), items, items + 3) ==
          *s.insert(s.cbegin(), items, items + 3)); equal(v, s);
    check(*v.insert(v.cend(), {9, 10}) == *s.insert(s.cend(), {9, 10})); equal(v, s);
    check(*v.emplace(v.cbegin(), 11) == *s.emplace(s.cbegin(), 11)); equal(v, s);
    v.push_back(value); s.push_back(value);
    v.push_back(12); s.push_back(12);
    v.emplace_back(13); s.emplace_back(13); equal(v, s);
    v.pop_back(); s.pop_back(); equal(v, s);
    v.resize(20); s.resize(20); equal(v, s);
    v.resize(25, 99); s.resize(25, 99); equal(v, s);
    v.resize(2); s.resize(2); equal(v, s);
    v.reserve(100); check(v.capacity() >= 100); equal(v, s);
    v.shrink_to_fit(); equal(v, s);
    check(v.capacity() >= v.size() && v.max_size() >= v.capacity());
    eunsortedvector<int> other{42};
    v.swap(other);
    check(v.size() == 1 && v[0] == 42);
    using std::swap;
    swap(v, other); equal(v, s);
    v.clear(); check(v.empty() && v.size() == 0);
}

void unordered_erase()
{
    for (int size = 0; size <= 10; ++size) {
        for (int first = 0; first <= size; ++first) {
            for (int last = first; last <= size; ++last) {
                eunsortedvector<int> values;
                std::vector<int> expected;
                for (int i = 0; i < size; ++i) {
                    values.push_back(i);
                    if (i < first || i >= last) {
                        expected.push_back(i);
                    }
                }
                const auto result = values.erase(values.cbegin() + first,
                                                 values.cbegin() + last);
                check(result == values.begin() + first);
                check(values.size() == expected.size());
                for (int i = 0; i < first; ++i) {
                    check(values[i] == i);
                }
                if (first == last) {
                    equal(values, expected);
                }
                std::sort(values.begin(), values.end());
                equal(values, expected);
            }
        }
        for (int pos = 0; pos < size; ++pos) {
            eunsortedvector<int> values;
            std::vector<int> expected;
            for (int i = 0; i < size; ++i) {
                values.push_back(i);
                expected.push_back(i);
            }
            expected[pos] = expected.back();
            expected.pop_back();
            check(values.erase(values.cbegin() + pos) == values.begin() + pos);
            equal(values, expected);
        }
    }

    eunsortedvector<std::unique_ptr<int> > pointers;
    for (int i = 0; i < 5; ++i) {
        pointers.emplace_back(new int(i));
    }
    check(**pointers.erase(pointers.cbegin() + 1) == 4);
    check(pointers.erase(pointers.cbegin() + 1, pointers.cbegin() + 3) ==
          pointers.begin() + 1);
    check(pointers.size() == 2 && *pointers[0] == 0 && *pointers[1] == 3);

    eunsortedvector<bool> bits{true, false, false, true};
    check(*bits.erase(bits.cbegin() + 1));
    check(bits.erase(bits.cbegin(), bits.cbegin() + 2) == bits.begin());
    equal(bits, std::vector<bool>{false});
}

void access_and_comparisons()
{
    eunsortedvector<int> v{1, 2, 3};
    const eunsortedvector<int>& c = v;
    check(v.at(1) == 2 && c.at(1) == 2);
    check(v.front() == 1 && c.front() == 1 && v.back() == 3 && c.back() == 3);
    check(v.data() == &v[0] && c.data() == &c[0]);
    check(v.end() - v.begin() == 3 && c.end() - c.begin() == 3);
    check(c.cend() - c.cbegin() == 3);
    check(*v.rbegin() == 3 && *c.rbegin() == 3 && *c.crbegin() == 3);
    check(v.rend() - v.rbegin() == 3 && c.rend() - c.rbegin() == 3);
    check(c.crend() - c.crbegin() == 3);
    bool threw = false;
    try { c.at(3); } catch (const std::out_of_range&) { threw = true; }
    check(threw);
    eunsortedvector<int> greater{1, 2, 4};
    check(v == v && !(v != v) && v != greater && v < greater);
    check(v <= greater && greater > v && greater >= v && v <= v && v >= v);
}

void move_only_and_bool()
{
    eunsortedvector<std::unique_ptr<int> > v;
    v.emplace_back(new int(3));
    v.push_back(std::unique_ptr<int>(new int(4)));
    v.insert(v.cbegin(), std::unique_ptr<int>(new int(2)));
    v.emplace(v.cbegin(), new int(1));
    v.reserve(20);
    eunsortedvector<std::unique_ptr<int> > moved(std::move(v));
    check(moved.size() == 4 && *moved[0] == 1 && *moved[3] == 4);
    eunsortedvector<std::string> words;
    words.emplace_back(3, 'x');
    check(words.front() == "xxx");
    eunsortedvector<bool> bits{true, false, true};
    bits[1] = true;
    bits.flip();
    equal(bits, std::vector<bool>{false, false, false});
    bits[0] = true;
    eunsortedvector<bool>::swap(bits[0], bits[1]);
    equal(bits, std::vector<bool>{false, true, false});
    bits.emplace_back(true);
    bits.emplace(bits.cbegin(), true);
    check(bits.size() == 5 && bits.front() && bits.back());
    check(std::hash<eunsortedvector<bool> >()(bits) ==
          std::hash<std::vector<bool> >()(std::vector<bool>(bits.begin(), bits.end())));
    static_assert(std::is_same<eunsortedvector<int>::reference, int&>::value, "reference type");
    static_assert(std::is_nothrow_move_constructible<eunsortedvector<int> >::value, "move noexcept");
    static_assert(std::is_same<decltype(v.emplace_back(nullptr)), void>::value,
                  "C++11 emplace_back returns void");
}

struct Tracked
{
    static int alive;
    static int copies_before_throw;
    int value;
    explicit Tracked(int n = 0) : value(n) { ++alive; }
    Tracked(const Tracked& other) : value(other.value)
    {
        if (copies_before_throw == 0) {
            throw std::runtime_error("copy failure");
        }
        if (copies_before_throw > 0) {
            --copies_before_throw;
        }
        ++alive;
    }
    Tracked& operator=(const Tracked&) = default;
    ~Tracked() { --alive; }
};

int Tracked::alive = 0;
int Tracked::copies_before_throw = -1;

void exception_safety_and_lifetimes()
{
    {
        eunsortedvector<Tracked> values;
        values.emplace_back(1);
        values.emplace_back(2);
        const std::size_t old_capacity = values.capacity();
        Tracked::copies_before_throw = 1;
        bool threw = false;
        try {
            values.reserve(old_capacity + 10);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        check(threw && values.size() == 2 && values.capacity() == old_capacity);
        check(values[0].value == 1 && values[1].value == 2 && Tracked::alive == 2);
        Tracked::copies_before_throw = -1;
        values.resize(1);
        check(Tracked::alive == 1);
        values.clear();
        check(Tracked::alive == 0);
        values.emplace_back(3);
        check(Tracked::alive == 1);
    }
    check(Tracked::alive == 0);
}

bool run_test(const char* name, void (*test)())
{
    try {
        test();
        std::cout << "[PASS] " << name << '\n';
        return true;
    } catch (const std::exception& error) {
        std::cout << "[FAIL] " << name << ": " << error.what() << '\n';
    } catch (...) {
        std::cout << "[FAIL] " << name << ": unknown exception\n";
    }
    return false;
}

int main()
{
    const struct {
        const char* name;
        void (*test)();
    } tests[] = {
        {"constructors_and_allocators", constructors_and_allocators},
        {"modifiers", modifiers},
        {"unordered_erase", unordered_erase},
        {"access_and_comparisons", access_and_comparisons},
        {"move_only_and_bool", move_only_and_bool},
        {"exception_safety_and_lifetimes", exception_safety_and_lifetimes}
    };

    std::size_t passed = 0;
    std::size_t failed = 0;
    for (const auto& test : tests) {
        if (run_test(test.name, test.test)) {
            ++passed;
        } else {
            ++failed;
        }
    }
    std::cout << "Summary: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
