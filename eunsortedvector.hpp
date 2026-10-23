#pragma once

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T, typename Alloc = std::allocator<T> >
class eunsortedvector final
{
    typedef std::vector<T, Alloc> storage_type;

public:
    typedef typename storage_type::value_type value_type;
    typedef typename storage_type::allocator_type allocator_type;
    typedef typename storage_type::size_type size_type;
    typedef typename storage_type::difference_type difference_type;
    typedef typename storage_type::reference reference;
    typedef typename storage_type::const_reference const_reference;
    typedef typename storage_type::pointer pointer;
    typedef typename storage_type::const_pointer const_pointer;
    typedef typename storage_type::iterator iterator;
    typedef typename storage_type::const_iterator const_iterator;
    typedef typename storage_type::reverse_iterator reverse_iterator;
    typedef typename storage_type::const_reverse_iterator const_reverse_iterator;

    eunsortedvector() noexcept(noexcept(storage_type())) = default;
    explicit eunsortedvector(const allocator_type& alloc) noexcept
        : data_(alloc) {}
    explicit eunsortedvector(size_type count, const allocator_type& alloc = allocator_type())
        : data_(count, alloc) {}
    eunsortedvector(size_type count, const T& value,
            const allocator_type& alloc = allocator_type())
        : data_(count, value, alloc) {}
    template<typename InputIt,
             typename std::enable_if<!std::is_integral<InputIt>::value, int>::type = 0>
    eunsortedvector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type())
        : data_(first, last, alloc) {}
    eunsortedvector(const eunsortedvector&) = default;
    eunsortedvector(eunsortedvector&&) = default;
    eunsortedvector(const eunsortedvector& other, const allocator_type& alloc)
        : data_(other.data_, alloc) {}
    eunsortedvector(eunsortedvector&& other, const allocator_type& alloc)
        : data_(std::move(other.data_), alloc) {}
    eunsortedvector(std::initializer_list<T> init,
            const allocator_type& alloc = allocator_type())
        : data_(init, alloc) {}

    eunsortedvector& operator=(const eunsortedvector&) = default;
    eunsortedvector& operator=(eunsortedvector&&) = default;
    eunsortedvector& operator=(std::initializer_list<T> init)
    {
        data_ = init;
        return *this;
    }

    void assign(size_type count, const T& value) { data_.assign(count, value); }
    template<typename InputIt>
    typename std::enable_if<!std::is_integral<InputIt>::value>::type
    assign(InputIt first, InputIt last) { data_.assign(first, last); }
    void assign(std::initializer_list<T> init) { data_.assign(init); }
    allocator_type get_allocator() const noexcept { return data_.get_allocator(); }

    reference at(size_type pos) { return data_.at(pos); }
    const_reference at(size_type pos) const { return data_.at(pos); }

    // Element access with bounds checking
    reference operator[](size_type pos) { return data_.at(pos); }
    const_reference operator[](size_type pos) const { return data_.at(pos); }

    reference front() { return data_.front(); }
    const_reference front() const { return data_.front(); }
    
    reference back() { return data_.back(); }
    const_reference back() const { return data_.back(); }

    // Packed vector<bool> has proxy references and no contiguous bool array.
    template<typename U = T>
    typename std::enable_if<std::is_same<U, T>::value &&
                            !std::is_same<U, bool>::value, T*>::type
    data() noexcept { return data_.data(); }
    template<typename U = T>
    typename std::enable_if<std::is_same<U, T>::value &&
                            !std::is_same<U, bool>::value, const T*>::type
    data() const noexcept { return data_.data(); }

    iterator begin() noexcept { return data_.begin(); }
    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator end() const noexcept { return data_.end(); }
    const_iterator cend() const noexcept { return data_.cend(); }
    reverse_iterator rbegin() noexcept { return data_.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
    const_reverse_iterator crbegin() const noexcept { return data_.crbegin(); }
    reverse_iterator rend() noexcept { return data_.rend(); }
    const_reverse_iterator rend() const noexcept { return data_.rend(); }
    const_reverse_iterator crend() const noexcept { return data_.crend(); }

    bool empty() const noexcept { return data_.empty(); }
    size_type size() const noexcept { return data_.size(); }
    size_type max_size() const noexcept { return data_.max_size(); }
    void reserve(size_type new_cap) { data_.reserve(new_cap); }
    size_type capacity() const noexcept { return data_.capacity(); }
    void shrink_to_fit() { data_.shrink_to_fit(); }

    void clear() noexcept { data_.clear(); }
    iterator insert(const_iterator pos, const T& value) { return data_.insert(pos, value); }
    iterator insert(const_iterator pos, T&& value)
    {
        return data_.insert(pos, std::move(value));
    }
    iterator insert(const_iterator pos, size_type count, const T& value)
    {
        return data_.insert(pos, count, value);
    }
    template<typename InputIt>
    typename std::enable_if<!std::is_integral<InputIt>::value, iterator>::type
    insert(const_iterator pos, InputIt first, InputIt last)
    {
        return data_.insert(pos, first, last);
    }
    iterator insert(const_iterator pos, std::initializer_list<T> init)
    {
        return data_.insert(pos, init);
    }
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        return data_.emplace(pos, std::forward<Args>(args)...);
    }

    iterator erase(const_iterator pos)
    {
        const difference_type index = pos - data_.cbegin();
        iterator target = data_.begin() + index;
        if (target != data_.end() - 1) {
            std::iter_swap(target, data_.end() - 1);
        }
        data_.pop_back();
        return data_.begin() + index;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        const difference_type index = first - data_.cbegin();
        const difference_type count = last - first;
        const difference_type replacements = std::min(count, data_.cend() - last);
        // Only swap surviving tail elements; the source and destination never overlap.
        for (difference_type i = 0; i < replacements; ++i) {
            std::iter_swap(data_.begin() + index + i, data_.end() - replacements + i);
        }
        for (difference_type i = 0; i < count; ++i) {
            data_.pop_back();
        }
        return data_.begin() + index;
    }


    void push_back(const T& value) { data_.push_back(value); }
    void push_back(T&& value) { data_.push_back(std::move(value)); }
    template<typename... Args>
    void emplace_back(Args&&... args) { data_.emplace_back(std::forward<Args>(args)...); }
    void pop_back() { data_.pop_back(); }
    void resize(size_type count) { data_.resize(count); }
    void resize(size_type count, const value_type& value) { data_.resize(count, value); }
    void swap(eunsortedvector& other) noexcept(noexcept(data_.swap(other.data_)))
    {
        data_.swap(other.data_);
    }

    template<typename U = T>
    typename std::enable_if<std::is_same<U, T>::value &&
                            std::is_same<U, bool>::value>::type
    flip() noexcept { data_.flip(); }
    template<typename U = T>
    static typename std::enable_if<std::is_same<U, T>::value &&
                                   std::is_same<U, bool>::value>::type
    swap(reference left, reference right) noexcept { storage_type::swap(left, right); }

    friend bool operator==(const eunsortedvector& lhs, const eunsortedvector& rhs)
    {
        return lhs.data_ == rhs.data_;
    }
    friend bool operator!=(const eunsortedvector& lhs, const eunsortedvector& rhs) { return !(lhs == rhs); }
    friend bool operator<(const eunsortedvector& lhs, const eunsortedvector& rhs)
    {
        return lhs.data_ < rhs.data_;
    }
    friend bool operator<=(const eunsortedvector& lhs, const eunsortedvector& rhs) { return !(rhs < lhs); }
    friend bool operator>(const eunsortedvector& lhs, const eunsortedvector& rhs) { return rhs < lhs; }
    friend bool operator>=(const eunsortedvector& lhs, const eunsortedvector& rhs) { return !(lhs < rhs); }

private:
    friend struct std::hash<eunsortedvector>;
    storage_type data_;
};

template<typename T, typename Alloc>
void swap(eunsortedvector<T, Alloc>& lhs, eunsortedvector<T, Alloc>& rhs)
    noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

namespace std {
template<typename Alloc>
struct hash<eunsortedvector<bool, Alloc> >
{
    size_t operator()(const eunsortedvector<bool, Alloc>& value) const
        noexcept(noexcept(hash<vector<bool, Alloc> >()(value.data_)))
    {
        return hash<vector<bool, Alloc> >()(value.data_);
    }
};
}
