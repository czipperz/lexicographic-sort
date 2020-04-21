#include <Tracy.hpp>
#include <algorithm>
#include <cz/buffer_array.hpp>
#include <cz/defer.hpp>
#include <cz/heap.hpp>
#include <cz/str.hpp>
#include <cz/string.hpp>
#include <cz/vector.hpp>

int main(int argc, char** argv) {
    cz::Buffer_Array buffer_array;
    buffer_array.create();
    CZ_DEFER(buffer_array.drop());

    cz::Vector<cz::Vector<cz::Str> > buckets = {};
    CZ_DEFER(buckets.drop(cz::heap_allocator()));

    char buffer[1024];
    cz::String value = {};
    while (1) {
        size_t buffer_len = fread(buffer, 1, sizeof(buffer), stdin);
        if (buffer_len == 0) {
            // Todo: add this value
            break;
        }

        const char* start = buffer;
        while (1) {
            cz::Str str = {start, buffer_len - (start - buffer)};
            const char* end = str.find('\n');
            if (end) {
                value.reserve(buffer_array.allocator(), end - start);
                value.append(str);
                value.realloc(buffer_array.allocator());

                if (buckets.len() < value.len()) {
                    size_t extra = value.len() - buckets.len();
                    buckets.reserve(cz::heap_allocator(), extra);
                    for (size_t i = 0; i < extra; ++i) {
                        buckets.push({});
                    }
                }

                buckets[value.len()].reserve(cz::heap_allocator(), 1);
                buckets[value.len()].push(value);

                value = {};
                start = end + 1;
            } else {
                value.reserve(buffer_array.allocator(), str.len);
                value.append(str);
            }
        }
    }

    for (size_t i = 0; i < buckets.len(); ++i) {
        cz::Slice<cz::Str> bucket = buckets[i];
        std::sort(bucket.elems, bucket.elems + bucket.len);
        for (size_t j = 0; j < bucket.len; ++j) {
            cz::Str v = bucket[j];
            fwrite(v.buffer, 1, v.len, stdout);
            putchar('\n');
        }
    }
}
