#include <cassert>
#include <cstdlib>
#include <iostream>

namespace aligned_memory {

std::size_t get_chunks_count(std::size_t size, std::size_t alignment) {
    assert(alignment > 0);
    return size / alignment + bool(size % alignment);
}

std::size_t need_alloc(std::size_t size, std::size_t alignment) {
    assert(alignment > 0);
    const auto chunks_count = get_chunks_count(size, alignment)
                              + get_chunks_count(sizeof(void *), alignment);
    return alignment * chunks_count + alignment - 1;
}

std::size_t get_aligned_shift(void *real, std::size_t alignment) {
    assert(alignment > 0);
    const auto rest = reinterpret_cast<std::size_t>(real) % alignment;
    const auto has = rest > 0 ? alignment - rest : 0;
    if (has < sizeof(void *)) {
        return has + alignment * get_chunks_count(sizeof(void *), alignment);
    } else {
        return has;
    }
}

void* align(void *real, std::size_t alignment) {
    const auto shift = get_aligned_shift(real, alignment);
    void **chunk = reinterpret_cast<void **>(reinterpret_cast<char *>(real) + shift);
    *(chunk - 1) = real;
    return chunk;
}

void* aligned_malloc(std::size_t size, std::size_t alignment) {
    void *real = malloc(need_alloc(size, alignment));
    if (real == nullptr) {
        return real;
    }
    return align(real, alignment);
}

void* get_real(void *chunk) {
    assert(chunk != nullptr);
    return *(reinterpret_cast<void **>(chunk) - 1);
}

void aligned_free(void* chunk) {
    return free(chunk == nullptr ? chunk : get_real(chunk));
}

void test_chunks_count() {
    assert(get_chunks_count(1, 1) == 1);
    assert(get_chunks_count(2, 1) == 2);
    assert(get_chunks_count(3, 2) == 2);
    assert(get_chunks_count(2, 3) == 1);
}

void test_need_alloc() {
    assert(need_alloc(sizeof(void *) - 1, sizeof(void *)) == 2 * sizeof(void *) + sizeof(void *) - 1);
    assert(need_alloc(sizeof(void *), sizeof(void *)) == 2 * sizeof(void *) + sizeof(void *) - 1);
    assert(need_alloc(sizeof(void *) + 1, sizeof(void *)) == 3 * sizeof(void *) + sizeof(void *) - 1);
}

void test_get_aligned_shift() {
    char *begin = reinterpret_cast<char *>(0);
    {
        const std::size_t alignment = 1;
        assert(get_aligned_shift(begin, alignment) == 8);
    }
    {
        const std::size_t alignment = sizeof(void *) - 1;
        assert(get_aligned_shift(begin, alignment) == 2 * sizeof(void *) - 2);
    }
    {
        const std::size_t shift = sizeof(void *) - 1;
        const std::size_t alignment = sizeof(void *) - 1;
        assert(get_aligned_shift(begin + shift, alignment) == 2 * sizeof(void *) - 2);
    }
    {
        const std::size_t shift = sizeof(void *) - 1;
        const std::size_t alignment = sizeof(void *);
        assert(get_aligned_shift(begin + shift, alignment) == sizeof(void *) + 1);
    }
    {
        const std::size_t shift = sizeof(void *) - 1;
        const std::size_t alignment = sizeof(void *) + 1;
        assert(get_aligned_shift(begin + shift, alignment) == sizeof(void *) + 3);
    }
}

void test_align() {
    {
        const std::size_t size = 1;
        const std::size_t alignment = 1;
        const std::size_t real_size = need_alloc(size, alignment);
        void *real = malloc(real_size);
        assert(real != nullptr);
        void *result = align(real, alignment);
        assert(reinterpret_cast<std::size_t>(result) % alignment == 0);
        assert(get_real(result) == real);
        assert(reinterpret_cast<char *>(real) + real_size >= reinterpret_cast<char *>(result) + size);
        free(real);
    }
    {
        const std::size_t size = sizeof(void *);
        const std::size_t alignment = 1;
        const std::size_t real_size = need_alloc(size, alignment);
        void *real = malloc(real_size);
        assert(real != nullptr);
        void *result = align(real, alignment);
        assert(reinterpret_cast<std::size_t>(result) % alignment == 0);
        assert(get_real(result) == real);
        assert(reinterpret_cast<char *>(real) + real_size >= reinterpret_cast<char *>(result) + size);
        free(real);
    }
    {
        const std::size_t size = sizeof(void *);
        const std::size_t alignment = sizeof(void *);
        const std::size_t real_size = need_alloc(size, alignment);
        void *real = malloc(real_size);
        assert(real != nullptr);
        void *result = align(real, alignment);
        assert(reinterpret_cast<std::size_t>(result) % alignment == 0);
        assert(get_real(result) == real);
        assert(reinterpret_cast<char *>(real) + real_size >= reinterpret_cast<char *>(result) + size);
        free(real);
    }
    {
        const std::size_t size = sizeof(void *);
        const std::size_t alignment = sizeof(void *) + 1;
        const std::size_t real_size = need_alloc(size, alignment);
        void *real = malloc(real_size);
        assert(real != nullptr);
        void *result = align(real, alignment);
        assert(reinterpret_cast<std::size_t>(result) % alignment == 0);
        assert(get_real(result) == real);
        assert(reinterpret_cast<char *>(real) + real_size >= reinterpret_cast<char *>(result) + size);
        free(real);
    }
}

void test_aligned_malloc_and_free() {
    void *memory = aligned_malloc(42, 13);
    aligned_free(memory);
}

void test() {
    test_chunks_count();
    test_need_alloc();
    test_get_aligned_shift();
    test_align();
    test_aligned_malloc_and_free();
    std::cout << "done" << std::endl;
}

}

int main() {
    aligned_memory::test();
    return 0;
}
