#ifndef EXT_ARRAY_VIEW_HPP
#define EXT_ARRAY_VIEW_HPP

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <cassert>
#include <cstddef>

#include "meta.hpp"

namespace ext
{
    constexpr struct assume_safe_array_tag {} assume_safe_array {};

    /**
     * Non-owning view of contiguous memory region.
     */
    template<typename T>
    struct array_view
    {
        using access_type = T;
        using value_type = std::remove_const_t<T>;
        using pointer = access_type*;
        using const_pointer = access_type const*;
        using reference = access_type&;
        using const_reference = access_type const&;
        using iterator = access_type*;
        using const_iterator = access_type const*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        // Creation ____________________________________________________________

        /**
         * Constructs a null view.
         */
        constexpr
        array_view() noexcept
            : ptr_(nullptr), size_(0)
        {
        }

        /**
         * Constructs a view into the memory region.
         */
        constexpr
        array_view(pointer ptr, size_type size)
            : ptr_(ptr), size_(size)
        {
        }

        constexpr
        array_view(pointer ptr, size_type count, assume_safe_array_tag) noexcept
            : ptr_(ptr), size_(count)
        {
        }

        /**
         * Conversion.
         */
        template<typename U,
                 std::enable_if_t<std::is_convertible<U*, T*>::value, int> = 0>
        constexpr
        array_view(array_view<U> other) noexcept
            : array_view(other.data(), other.size(), assume_safe_array)
        {
        }

        // Element access functions ____________________________________________

        constexpr
        reference operator[](size_type idx)
        {
            return ptr_[idx];
        }

        constexpr
        const_reference operator[](size_type idx) const
        {
            return ptr_[idx];
        }

        constexpr
        reference at(size_type idx)
        {
            if (idx > size_)
            {
                throw std::range_error("out of range");
            }
            return ptr_[idx];
        }

        constexpr
        const_reference at(size_type idx) const
        {
            if (idx > size_)
            {
                throw std::range_error("out of range");
            }
            return ptr_[idx];
        }

        constexpr
        reference front()
        {
            return *ptr_;
        }

        constexpr
        const_reference front() const
        {
            return *ptr_;
        }

        constexpr
        reference back()
        {
            return ptr_[size_ - 1];
        }

        constexpr
        const_reference back() const
        {
            return ptr_[size_ - 1];
        }

        constexpr
        pointer data() noexcept
        {
            return ptr_;
        }

        constexpr
        const_pointer data() const noexcept
        {
            return ptr_;
        }

        // Capacity query ______________________________________________________

        constexpr
        size_type size() const noexcept
        {
            return size_;
        }

        constexpr
        bool empty() const noexcept
        {
            return size() == 0;
        }

        // Range support _______________________________________________________

        constexpr iterator begin() noexcept { return ptr_; }
        constexpr iterator end() noexcept { return ptr_ + size_; }
        constexpr const_iterator begin() const noexcept { return ptr_; }
        constexpr const_iterator end() const noexcept { return ptr_ + size_; }
        constexpr const_iterator cbegin() const noexcept { return ptr_; }
        constexpr const_iterator cend() const noexcept { return ptr_ + size_; }
        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        reverse_iterator rend() noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rend() const noexcept { return reverse_iterator(end()); }
        const_reverse_iterator crbegin() const noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator crend() const noexcept { return reverse_iterator(begin()); }

        // Range modification __________________________________________________

        constexpr
        void pop_front()
        {
            ++ptr_;
            --size_;
        }

        constexpr
        void pop_back()
        {
            --size_;
        }

        constexpr
        void pop_front(size_type n)
        {
            ptr_ += n;
            size_ -= n;
        }

        constexpr
        void pop_back(size_type n)
        {
            size_ -= n;
        }

        constexpr
        void swap(array_view& other) noexcept
        {
            std::swap(ptr_, other.ptr_);
            std::swap(size_, other.size_);
        }

        constexpr friend
        void swap(array_view& a, array_view& b) noexcept
        {
            a.swap(b);
        }

      private:

        pointer   ptr_;
        size_type size_;
    };

    //__ Comparison Operators ______________________________________________

    template<typename T>
    bool operator==(array_view<T> const& a, array_view<T> const& b)
    {
        return a.size() == b.size() && std::equal(a.cbegin(), a.cend(), b.cbegin(), b.cend());
    }

    template<typename T>
    bool operator!=(array_view<T> const& a, array_view<T> const& b)
    {
        return !(a == b);
    }

    template<typename T>
    bool operator<(array_view<T> const& a, array_view<T> const& b)
    {
        return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
    }

    template<typename T>
    bool operator>(array_view<T> const& a, array_view<T> const& b)
    {
        return b < a;
    }

    template<typename T>
    bool operator<=(array_view<T> const& a, array_view<T> const& b)
    {
        return !(a > b);
    }

    template<typename T>
    bool operator>=(array_view<T> const& a, array_view<T> const& b)
    {
        return !(a < b);
    }

    //__ Factory Functions _________________________________________________

    /**
     * Creates an array_view into the storage of a std::vector.
     */
    template<typename T, typename Allocator>
    array_view<T> make_array_view(std::vector<T, Allocator>& vec) noexcept
    {
        return array_view<T>(vec.data(), vec.size(), assume_safe_array);
    }

    template<typename T, typename Allocator>
    array_view<T const> make_array_view(std::vector<T, Allocator> const& vec) noexcept
    {
        return array_view<T const>(vec.data(), vec.size(), assume_safe_array);
    }

    /**
     * Creates an array_view into the storage of a std::array.
     */
    template<typename T, std::size_t N>
    constexpr
    array_view<T> make_array_view(std::array<T, N>& ary) noexcept
    {
        return array_view<T>(ary.data(), ary.size(), assume_safe_array);
    }

    template<typename T, std::size_t N>
    constexpr
    array_view<T const> make_array_view(std::array<T, N> const& ary) noexcept
    {
        return array_view<T const>(ary.data(), ary.size(), assume_safe_array);
    }

    /**
     * Creates an array_view into an array.
     */
    template<typename T, std::size_t N>
    constexpr
    array_view<T> make_array_view(T(& ary)[N]) noexcept
    {
        return array_view<T>(ary, N, assume_safe_array);
    }

    template<typename T, std::size_t N>
    constexpr
    array_view<T const> make_array_view(T const(& ary)[N]) noexcept
    {
        return array_view<T const>(ary, N, assume_safe_array);
    }

    /**
     * Creates an array_view into raw memory region. Behavior is undefined if
     * the specified memory region is invalid (hence this function is not
     * noexcept).
     */
    template<typename T>
    constexpr
    array_view<T> make_array_view(T* ptr, std::size_t size)
    {
        return array_view<T>(ptr, size);
    }

    template<typename T, std::size_t N>
    constexpr
    array_view<T const> make_array_view(T const* ptr, std::size_t size)
    {
        return array_view<T const>(ptr, size);
    }
}

//____ Array View with Member Reference ______________________________________

namespace ext
{
    /*
     * Random-access iterator adaptor for accessing non-static data member of
     * the objects in underlying array.
     */
    template<typename T, typename M>
    struct member_array_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = M;
        using access_type = ext::propagate_cv_t<T, M>;
        using member_ptr = M T::*;
        using reference = access_type&;
        using pointer = access_type*;
        using difference_type = std::ptrdiff_t;

        member_array_iterator(T* base, M T::* member)
            : base_(base)
            , member_(member)
        {
        }

        friend bool operator==(member_array_iterator const& i, member_array_iterator const& j)
        {
            return i.base_ == j.base_ && i.member_ == j.member_;
        }

        friend bool operator!=(member_array_iterator const& i, member_array_iterator const& j)
        {
            return !(i == j);
        }

        reference operator*() const
        {
            return base_->*member_;
        }

        pointer operator->() const
        {
            return std::addressof(base_->*member_);
        }

        // Forward

        member_array_iterator& operator++()
        {
            ++base_;
            return *this;
        }

        member_array_iterator operator++(int)
        {
            member_array_iterator copy(*this);
            ++*this;
            return copy;
        }

        // Bidirectional

        member_array_iterator& operator--()
        {
            --base_;
            return *this;
        }

        member_array_iterator operator--(int)
        {
            member_array_iterator copy(*this);
            --*this;
            return copy;
        }

        // Random-access

        member_array_iterator& operator+=(difference_type z)
        {
            base_ += z;
            return *this;
        }

        member_array_iterator operator-=(difference_type z)
        {
            base_ -= z;
            return *this;
        }

        friend
        difference_type operator-(member_array_iterator const& i, member_array_iterator const& j)
        {
            return i.base_ - j.base_;
        }

      private:
        T* base_;
        M T::* member_;
    };

    template< typename T, typename M
            , typename D = typename member_array_iterator<T, M>::difference_type>
    member_array_iterator<T, M> operator+(member_array_iterator<T, M> i, D z)
    {
        return i += z;
    }

    template< typename T, typename M
            , typename D = typename member_array_iterator<T, M>::difference_type>
    member_array_iterator<T, M> operator-(member_array_iterator<T, M> i, D z)
    {
        return i -= z;
    }

    /*
     * Similar to array_view<T>, but this class provides access to non-static
     * data member of the array elements via pointer-to-member.
     */
    template<typename T, typename M>
    struct member_array_view
    {
        using value_type = M;
        using access_type = ext::propagate_cv_t<T, M>;
        using size_type = std::size_t;
        using member_ptr = M T::*;
        using reference = access_type&;
        using pointer = access_type*;
        using iterator = member_array_iterator<T, M>;

        // TODO: more constructors with type convertion

        member_array_view() = default;

        member_array_view(T* data, size_type size, member_ptr member)
            : data_(data)
            , size_(size)
            , member_(member)
        {
        }

        // TODO: more members

        size_type size() const noexcept
        {
            return size_;
        }

        iterator begin() const
        {
            return iterator(data_, member_);
        }

        iterator end() const
        {
            return iterator(data_ + size_, member_);
        }

        reference operator[](size_type idx) const
        {
            return data_[idx].*member_;
        }

      private:
        T* data_ = nullptr;
        size_type size_ = 0;
        member_ptr member_ = nullptr;
    };

    template<typename T, typename M, typename A>
    auto make_member_array_view(std::vector<T, A>& vec, M T::* member) noexcept
    {
        return member_array_view<T, M>(vec.data(), vec.size(), member);
    }

    template<typename T, typename M, typename A>
    auto make_member_array_view(std::vector<T, A> const& vec, M T::* member) noexcept
    {
        return member_array_view<T const, M>(vec.data(), vec.size(), member);
    }

    template<typename T, typename M>
    auto make_member_array_view(T* data, std::size_t size, M std::remove_cv_t<T>::* member)
    {
        return member_array_view<T, M>(data, size, member);
    }
}

#endif
