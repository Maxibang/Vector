#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <string>
#include <stdexcept>
#include "array_ptr.h"
#include <utility>

using namespace std;



class ReserveProxyObj {
public:
    ReserveProxyObj(size_t new_capacity) : current_capacity(new_capacity) {

    }

    size_t GetCapacity() const noexcept {
        return current_capacity;
    }
private:
    size_t current_capacity;
};


ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}




template <typename Type>
class SimpleVector {
public:

    // Создаёт вектор с ReserveProxyObj 
    SimpleVector(ReserveProxyObj object_init) {
        Reserve(object_init.GetCapacity());
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {
            SimpleVector<Type> new_vector(new_capacity);
            std::copy((*this).begin(), (*this).end(), new_vector.begin());
            swap(new_vector);
            std::swap(size_, new_vector.size_);
        }
    }

    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Moving constructor
    SimpleVector(SimpleVector&& other) : array_(std::move(other.array_)) {
        other.size_ = std::exchange(size_, other.size_);
        other.capacity_ = std::exchange(capacity_, other.capacity_);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : array_(size), size_(size), capacity_(size) {
        for (size_t i = 0; i < size; ++i) {
            const Type value_tmp{};
            *(array_.Get() + i) = std::move(value_tmp);
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : array_(size), size_(size), capacity_(size) {
        fill((*this).begin(), (*this).end(), value);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, Type&& value) : array_(size), size_(size), capacity_(size) {
        for (size_t i = 0; i < size; ++i) {
            *(array_.Get() + i) = std::move(value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : array_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0) ? true : false;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Invalid index: out of range"s);
        }
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Invalid index: out of range"s);
        }
        return array_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {

        // Обнуляем размер вектора
        if (new_size == 0) {
            Clear();
            return;
        }

        // Уменьшаем размер, существующие элементы остаются без изменений
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }

        if (new_size <= capacity_) {
            auto begin = array_.Get() + size_;
            auto end = begin + (capacity_ - size_);

            for (; begin != end; ++begin) {
                Type value_tmp{};
                *begin = std::move(value_tmp);
            }
            size_ = new_size;
            return;
        }

        if (new_size > capacity_) {
            const auto new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> array_tmp(new_capacity);
            std::move((*this).begin(), (*this).end(), array_tmp.Get());

            auto it = array_tmp.Get() + size_;
            for (size_t i = 0; i < new_capacity - size_; ++i) {
                Type value_tmp{};
                *(it + i) = std::move(value_tmp);
            }

            array_.SetPtr(array_tmp.Release());
            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return array_.Get() + size_;
    }

    // Copying constructor
    SimpleVector(const SimpleVector& other) : size_(other.GetSize()), capacity_(other.GetSize()) {
        ArrayPtr<Type> array_tmp(other.GetSize());
        std::copy(other.begin(), other.end(), array_tmp.Get());
        array_.SetPtr(array_tmp.Release());
    }

    // Copying operator
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            const auto new_size = rhs.GetSize();
            ArrayPtr<Type> array_tmp(new_size);
            std::copy(rhs.begin(), rhs.end(), array_tmp.Get());

            array_.swap(array_tmp);
            size_ = new_size;
            capacity_ = new_size;
        }
        return *this;
    }

    // Moving operator
    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            array_.swap(rhs.array_);
            std::swap(size_, rhs.size_);
            std::swap(capacity_, rhs.capacity_);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        const auto new_size = size_ + 1;
        Resize(new_size);
        array_[size_ - 1] = item;
    }

    // Moving PushBack
    void PushBack(Type&& item) {
        const auto new_size = size_ + 1;
        Resize(new_size);
        array_[size_ - 1] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        const auto dist = std::distance(cbegin(), pos);
        const auto new_size = size_ + 1;
        Resize(new_size);

        auto start = begin() + dist;
        std::copy(start, end() - 1, start + 1);
        *start = value;
        return start;
    }

    // Moving-Insert at iter position
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        const auto dist = std::distance(cbegin(), pos);
        const auto new_size = size_ + 1;
        Resize(new_size);

        auto start = begin() + dist;
        std::move(start, end() - 1, start + 1);
        *start = std::move(value);
        return start;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        if (pos == end()) {
            return end();
        }
        const auto dist = std::distance(cbegin(), pos);

        std::move(begin() + dist + 1, end(), begin() + dist);
        --size_;
        return begin() + dist;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        const auto tmp_ptr = array_.Release();
        array_.SetPtr(other.array_.Release());
        other.array_.SetPtr(tmp_ptr);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector&& other) noexcept {
        const auto tmp_ptr = array_.Release();
        array_.SetPtr(other.array_.Release());
        other.array_.SetPtr(tmp_ptr);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};



template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs || lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs || lhs < rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
