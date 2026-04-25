#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include <cstddef>
#include <stdexcept>

namespace sjtu {

template<class T>
class deque {
private:
    static const size_t BLOCK_SIZE = 512;
    
    struct Block {
        T* data;
        size_t size;
        size_t capacity;
        Block* prev;
        Block* next;
        
        Block() : data(nullptr), size(0), capacity(BLOCK_SIZE), prev(nullptr), next(nullptr) {
            data = reinterpret_cast<T*>(::operator new(capacity * sizeof(T)));
        }
        
        Block(size_t cap) : data(nullptr), size(0), capacity(cap), prev(nullptr), next(nullptr) {
            data = reinterpret_cast<T*>(::operator new(capacity * sizeof(T)));
        }
        
        ~Block() {
            if (data) {
                for (size_t i = 0; i < size; ++i) {
                    data[i].~T();
                }
                ::operator delete(data);
            }
        }
        
        Block(const Block& other) : data(nullptr), size(other.size), capacity(other.capacity), prev(nullptr), next(nullptr) {
            data = reinterpret_cast<T*>(::operator new(capacity * sizeof(T)));
            for (size_t i = 0; i < size; ++i) {
                new (&data[i]) T(other.data[i]);
            }
        }
        
        void push_back(const T& value) {
            if (size >= capacity) {
                throw std::runtime_error("Block full");
            }
            new (&data[size]) T(value);
            ++size;
        }
        
        void push_front(const T& value) {
            if (size >= capacity) {
                throw std::runtime_error("Block full");
            }
            // Move existing elements
            for (size_t i = size; i > 0; --i) {
                new (&data[i]) T(data[i - 1]);
                data[i - 1].~T();
            }
            new (&data[0]) T(value);
            ++size;
        }
        
        void pop_back() {
            if (size == 0) {
                throw std::runtime_error("Block empty");
            }
            --size;
            data[size].~T();
        }
        
        void pop_front() {
            if (size == 0) {
                throw std::runtime_error("Block empty");
            }
            data[0].~T();
            for (size_t i = 0; i < size - 1; ++i) {
                new (&data[i]) T(data[i + 1]);
                data[i + 1].~T();
            }
            --size;
        }
    };
    
    Block* head;
    Block* tail;
    size_t total_size;
    
    void clear_blocks() {
        Block* current = head;
        while (current) {
            Block* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        total_size = 0;
    }
    
    void copy_from(const deque& other) {
        clear_blocks();
        if (!other.head) {
            return;
        }
        
        Block* other_current = other.head;
        Block* prev_block = nullptr;
        
        while (other_current) {
            Block* new_block = new Block(*other_current);
            if (!head) {
                head = new_block;
            }
            if (prev_block) {
                prev_block->next = new_block;
                new_block->prev = prev_block;
            }
            prev_block = new_block;
            tail = new_block;
            other_current = other_current->next;
        }
        
        total_size = other.total_size;
    }
    
public:
    class const_iterator;
    class iterator {
    private:
        Block* block;
        size_t index;
        const deque* container;
        
    public:
        friend class deque;
        friend class const_iterator;
        
        iterator() : block(nullptr), index(0), container(nullptr) {}
        iterator(Block* b, size_t i, const deque* c) : block(b), index(i), container(c) {}
        
        iterator operator+(const int &n) const {
            iterator result = *this;
            result += n;
            return result;
        }
        
        iterator operator-(const int &n) const {
            iterator result = *this;
            result -= n;
            return result;
        }
        
        int operator-(const iterator &rhs) const {
            if (container != rhs.container) {
                throw std::runtime_error("Invalid iterator comparison");
            }
            int count = 0;
            iterator it = rhs;
            while (it != *this) {
                ++it;
                ++count;
                if (count > 1000000) {
                    it = *this;
                    count = 0;
                    it = rhs;
                    while (it != *this) {
                        --it;
                        --count;
                    }
                    return count;
                }
            }
            return count;
        }
        
        iterator& operator+=(const int &n) {
            if (n >= 0) {
                for (int i = 0; i < n; ++i) {
                    ++(*this);
                }
            } else {
                for (int i = 0; i < -n; ++i) {
                    --(*this);
                }
            }
            return *this;
        }
        
        iterator& operator-=(const int &n) {
            return *this += (-n);
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator& operator++() {
            if (!block) {
                throw std::runtime_error("Invalid iterator");
            }
            ++index;
            if (index >= block->size) {
                block = block->next;
                index = 0;
            }
            return *this;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator& operator--() {
            if (!block) {
                if (container && container->tail) {
                    block = container->tail;
                    index = block->size;
                } else {
                    throw std::runtime_error("Invalid iterator");
                }
            }
            if (index == 0) {
                block = block->prev;
                if (!block) {
                    throw std::runtime_error("Invalid iterator");
                }
                index = block->size;
            }
            --index;
            return *this;
        }
        
        T& operator*() const {
            if (!block || index >= block->size) {
                throw std::runtime_error("Invalid iterator dereference");
            }
            return block->data[index];
        }
        
        T* operator->() const noexcept {
            return &(block->data[index]);
        }
        
        bool operator==(const iterator &rhs) const {
            return block == rhs.block && index == rhs.index;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return block == rhs.block && index == rhs.index;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    
    class const_iterator {
    private:
        const Block* block;
        size_t index;
        const deque* container;
        
    public:
        friend class deque;
        friend class iterator;
        
        const_iterator() : block(nullptr), index(0), container(nullptr) {}
        const_iterator(const Block* b, size_t i, const deque* c) : block(b), index(i), container(c) {}
        const_iterator(const iterator& it) : block(it.block), index(it.index), container(it.container) {}
        
        const_iterator operator+(const int &n) const {
            const_iterator result = *this;
            result += n;
            return result;
        }
        
        const_iterator operator-(const int &n) const {
            const_iterator result = *this;
            result -= n;
            return result;
        }
        
        int operator-(const const_iterator &rhs) const {
            if (container != rhs.container) {
                throw std::runtime_error("Invalid iterator comparison");
            }
            int count = 0;
            const_iterator it = rhs;
            while (it != *this) {
                ++it;
                ++count;
                if (count > 1000000) {
                    it = *this;
                    count = 0;
                    it = rhs;
                    while (it != *this) {
                        --it;
                        --count;
                    }
                    return count;
                }
            }
            return count;
        }
        
        const_iterator& operator+=(const int &n) {
            if (n >= 0) {
                for (int i = 0; i < n; ++i) {
                    ++(*this);
                }
            } else {
                for (int i = 0; i < -n; ++i) {
                    --(*this);
                }
            }
            return *this;
        }
        
        const_iterator& operator-=(const int &n) {
            return *this += (-n);
        }
        
        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        const_iterator& operator++() {
            if (!block) {
                throw std::runtime_error("Invalid iterator");
            }
            ++index;
            if (index >= block->size) {
                block = block->next;
                index = 0;
            }
            return *this;
        }
        
        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }
        
        const_iterator& operator--() {
            if (!block) {
                if (container && container->tail) {
                    block = container->tail;
                    index = block->size;
                } else {
                    throw std::runtime_error("Invalid iterator");
                }
            }
            if (index == 0) {
                block = block->prev;
                if (!block) {
                    throw std::runtime_error("Invalid iterator");
                }
                index = block->size;
            }
            --index;
            return *this;
        }
        
        const T& operator*() const {
            if (!block || index >= block->size) {
                throw std::runtime_error("Invalid iterator dereference");
            }
            return block->data[index];
        }
        
        const T* operator->() const noexcept {
            return &(block->data[index]);
        }
        
        bool operator==(const iterator &rhs) const {
            return block == rhs.block && index == rhs.index;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return block == rhs.block && index == rhs.index;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    
    deque() : head(nullptr), tail(nullptr), total_size(0) {}
    
    deque(const deque &other) : head(nullptr), tail(nullptr), total_size(0) {
        copy_from(other);
    }
    
    ~deque() {
        clear_blocks();
    }
    
    deque &operator=(const deque &other) {
        if (this != &other) {
            copy_from(other);
        }
        return *this;
    }
    
    T& at(const size_t &pos) {
        if (pos >= total_size) {
            throw std::out_of_range("Index out of range");
        }
        size_t current_pos = 0;
        Block* current = head;
        while (current) {
            if (current_pos + current->size > pos) {
                return current->data[pos - current_pos];
            }
            current_pos += current->size;
            current = current->next;
        }
        throw std::out_of_range("Index out of range");
    }
    
    const T& at(const size_t &pos) const {
        if (pos >= total_size) {
            throw std::out_of_range("Index out of range");
        }
        size_t current_pos = 0;
        Block* current = head;
        while (current) {
            if (current_pos + current->size > pos) {
                return current->data[pos - current_pos];
            }
            current_pos += current->size;
            current = current->next;
        }
        throw std::out_of_range("Index out of range");
    }
    
    T& operator[](const size_t &pos) {
        return at(pos);
    }
    
    const T& operator[](const size_t &pos) const {
        return at(pos);
    }
    
    const T& front() const {
        if (total_size == 0) {
            throw std::runtime_error("Container is empty");
        }
        return head->data[0];
    }
    
    const T& back() const {
        if (total_size == 0) {
            throw std::runtime_error("Container is empty");
        }
        return tail->data[tail->size - 1];
    }
    
    iterator begin() {
        if (!head) {
            return iterator(nullptr, 0, this);
        }
        return iterator(head, 0, this);
    }
    
    const_iterator cbegin() const {
        if (!head) {
            return const_iterator(nullptr, 0, this);
        }
        return const_iterator(head, 0, this);
    }
    
    iterator end() {
        return iterator(nullptr, 0, this);
    }
    
    const_iterator cend() const {
        return const_iterator(nullptr, 0, this);
    }
    
    bool empty() const {
        return total_size == 0;
    }
    
    size_t size() const {
        return total_size;
    }
    
    void clear() {
        clear_blocks();
    }
    
    iterator insert(iterator pos, const T &value) {
        if (pos.container != this) {
            throw std::runtime_error("Invalid iterator");
        }
        
        if (pos == begin()) {
            push_front(value);
            return begin();
        }
        
        if (pos == end()) {
            push_back(value);
            return iterator(tail, tail->size - 1, this);
        }
        
        Block* block = pos.block;
        size_t index = pos.index;
        
        if (block->size < block->capacity) {
            for (size_t i = block->size; i > index; --i) {
                new (&block->data[i]) T(block->data[i - 1]);
                block->data[i - 1].~T();
            }
            new (&block->data[index]) T(value);
            ++block->size;
            ++total_size;
            return iterator(block, index, this);
        }
        
        Block* new_block = new Block();
        size_t mid = block->size / 2;
        
        // Move elements from mid to end to new block
        for (size_t i = mid; i < block->size; ++i) {
            new (&new_block->data[new_block->size]) T(block->data[i]);
            ++new_block->size;
        }
        // Destroy moved elements and update size
        for (size_t i = mid; i < block->size; ++i) {
            block->data[i].~T();
        }
        block->size = mid;
        
        new_block->next = block->next;
        new_block->prev = block;
        if (block->next) {
            block->next->prev = new_block;
        } else {
            tail = new_block;
        }
        block->next = new_block;
        
        if (index <= mid) {
            for (size_t i = block->size; i > index; --i) {
                new (&block->data[i]) T(block->data[i - 1]);
                block->data[i - 1].~T();
            }
            new (&block->data[index]) T(value);
            ++block->size;
            ++total_size;
            return iterator(block, index, this);
        } else {
            size_t new_index = index - mid;
            for (size_t i = new_block->size; i > new_index; --i) {
                new (&new_block->data[i]) T(new_block->data[i - 1]);
                new_block->data[i - 1].~T();
            }
            new (&new_block->data[new_index]) T(value);
            ++new_block->size;
            ++total_size;
            return iterator(new_block, new_index, this);
        }
    }
    
    iterator erase(iterator pos) {
        if (pos.container != this || pos == end()) {
            throw std::runtime_error("Invalid iterator");
        }
        
        Block* block = pos.block;
        size_t index = pos.index;
        
        block->data[index].~T();
        for (size_t i = index; i < block->size - 1; ++i) {
            new (&block->data[i]) T(block->data[i + 1]);
            block->data[i + 1].~T();
        }
        --block->size;
        --total_size;
        
        if (block->size == 0) {
            Block* next_block = block->next;
            if (block->prev) {
                block->prev->next = block->next;
            } else {
                head = block->next;
            }
            if (block->next) {
                block->next->prev = block->prev;
            } else {
                tail = block->prev;
            }
            delete block;
            
            if (next_block) {
                return iterator(next_block, 0, this);
            } else {
                return end();
            }
        }
        
        if (index >= block->size) {
            if (block->next) {
                return iterator(block->next, 0, this);
            } else {
                return end();
            }
        }
        
        return iterator(block, index, this);
    }
    
    void push_back(const T &value) {
        if (!tail || tail->size >= tail->capacity) {
            Block* new_block = new Block();
            if (tail) {
                tail->next = new_block;
                new_block->prev = tail;
                tail = new_block;
            } else {
                head = tail = new_block;
            }
        }
        tail->push_back(value);
        ++total_size;
    }
    
    void pop_back() {
        if (total_size == 0) {
            throw std::runtime_error("Container is empty");
        }
        tail->pop_back();
        --total_size;
        
        if (tail->size == 0) {
            Block* old_tail = tail;
            tail = tail->prev;
            if (tail) {
                tail->next = nullptr;
            } else {
                head = nullptr;
            }
            delete old_tail;
        }
    }
    
    void push_front(const T &value) {
        if (!head || head->size >= head->capacity) {
            Block* new_block = new Block();
            if (head) {
                head->prev = new_block;
                new_block->next = head;
                head = new_block;
            } else {
                head = tail = new_block;
            }
        }
        head->push_front(value);
        ++total_size;
    }
    
    void pop_front() {
        if (total_size == 0) {
            throw std::runtime_error("Container is empty");
        }
        head->pop_front();
        --total_size;
        
        if (head->size == 0) {
            Block* old_head = head;
            head = head->next;
            if (head) {
                head->prev = nullptr;
            } else {
                tail = nullptr;
            }
            delete old_head;
        }
    }
};

}

#endif
