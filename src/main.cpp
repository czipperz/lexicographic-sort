#include <Tracy.hpp>
#include <algorithm>
#include <cz/buffer_array.hpp>
#include <cz/defer.hpp>
#include <cz/heap.hpp>
#include <cz/str.hpp>
#include <cz/string.hpp>
#include <cz/vector.hpp>

static void add_value(cz::Vector<cz::Vector<cz::Str> >& buckets, cz::Str value) {
    if (buckets.len() <= value.len) {
        size_t extra = value.len - buckets.len() + 1;
        buckets.reserve(cz::heap_allocator(), extra);
        for (size_t i = 0; i < extra; ++i) {
            buckets.push({});
        }
    }

    buckets[value.len].reserve(cz::heap_allocator(), 1);
    buckets[value.len].push(value);
}

static void read_file(cz::Buffer_Array& buffer_array,
                      cz::Vector<cz::Vector<cz::Str> >& buckets,
                      FILE* file,
                      char separator) {
    char buffer[1024];
    cz::String value = {};
    while (1) {
        size_t buffer_len = fread(buffer, 1, sizeof(buffer), file);
        if (buffer_len == 0) {
            add_value(buckets, value);
            break;
        }

        const char* start = buffer;
        while (1) {
            cz::Str str = {start, buffer_len - (start - buffer)};
            const char* end = str.find(separator);
            if (end) {
                value.reserve(buffer_array.allocator(), end - start);
                value.append({start, (size_t)(end - start)});
                value.realloc(buffer_array.allocator());

                add_value(buckets, value);

                value = {};
                start = end + 1;
            } else {
                value.reserve(buffer_array.allocator(), str.len);
                value.append(str);
                break;
            }
        }
    }
}

static void print_results(cz::Vector<cz::Vector<cz::Str> >& buckets, FILE* file, char separator) {
    for (size_t i = 0; i < buckets.len(); ++i) {
        cz::Slice<cz::Str> bucket = buckets[i];
        std::sort(bucket.elems, bucket.elems + bucket.len);
        for (size_t j = 0; j < bucket.len; ++j) {
            cz::Str v = bucket[j];
            fwrite(v.buffer, 1, v.len, file);
            putchar(separator);
        }
    }
}

int main(int argc, char** argv) {
    cz::Buffer_Array buffer_array;
    buffer_array.create();
    CZ_DEFER(buffer_array.drop());

    cz::Vector<cz::Vector<cz::Str> > buckets = {};
    CZ_DEFER(buckets.drop(cz::heap_allocator()));

    read_file(buffer_array, buckets, stdin, '\n');
    print_results(buckets, stdout, '\n');
}
