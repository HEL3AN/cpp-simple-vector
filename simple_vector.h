#pragma once

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <initializer_list>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity) : capacity_(capacity) {}

    size_t GetCapacity() const {
        return capacity_;
    }
private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}; 

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : data_(size), size_(size), capacity_(size) {
        if (size) {
            std::fill(data_.Get(), data_.Get() + size_, Type());
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : data_(size), size_(size), capacity_(size) {
        if (size) {
            std::fill(data_.Get(), data_.Get() + size_, value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : data_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), data_.Get());
    }

    SimpleVector(ReserveProxyObj reserve) : data_(reserve.GetCapacity()), size_(0), capacity_(reserve.GetCapacity()) {} 

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
        return !size_;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size_");
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size_");
        }
        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            std::fill(data_.Get() + size_, data_.Get() + new_size, Type());
            size_ = new_size;
        } else if (new_size > capacity_) {
            ArrayPtr<Type> new_data(new_size);
            std::copy(data_.Get(), data_.Get() + size_, new_data.Get());
            std::fill(new_data.Get() + size_, new_data.Get() + new_size, Type());
            data_.swap(new_data);
            capacity_ = std::max(capacity_ * 2, new_size);
            size_ = new_size;
        }
    }

    SimpleVector(const SimpleVector& other) : data_(other.size_), size_(other.size_), capacity_(other.capacity_) {
        std::copy(other.begin(), other.end(), data_.Get());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            Resize(size_ + 1);
        } else {
            ++size_;
        }
        data_[size_ - 1] = item;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = static_cast<size_t>(pos - begin());
        if (size_ == capacity_) {
            ArrayPtr<Type> new_data(capacity_ == 0 ? 1 : capacity_ * 2);
            std::copy(begin(), end(), new_data.Get());
            std::copy_backward(new_data.Get() + index, new_data.Get() + size_, new_data.Get() + size_ + 1);
            new_data[index] = value;
            data_.swap(new_data);
            ++size_;
            capacity_ = capacity_ == 0 ? 1 : capacity_ * 2;
        } else {
            std::copy_backward(begin() + index, end(), end() + 1);
            data_[index] = value;
            ++size_;
        }
        return begin() + index;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        assert(size_);
        size_t index = static_cast<size_t>(pos - begin());
        std::copy(begin() + index + 1, end(), begin() + index);
        --size_;
        return begin() + index;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> temp(new_capacity);
            std::copy(begin(), end(), temp.Get());
            data_.swap(temp);
            capacity_ = new_capacity;
        }
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return data_.Get() + size_;
    }

private:
    ArrayPtr<Type> data_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 