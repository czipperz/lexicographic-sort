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
            if (value.len() > 0) {
                add_value(buckets, value);
            }
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

void print_usage(char* program_name) {
    fprintf(stderr, "Usage: %s [-print0] [-0] [-] [FILE]...\n", program_name);
    fprintf(stderr,
            "Sort strings lexicographically -- ie by length and then alphabetically inside the "
            "length.\n\
If no files are given, stdin is read.\n\
\n\
Options:\n\
  -0          separate strings by null character instead of newline character\n\
  -print0     print strings separated by null character instead of newline character\n\
  -           read from stdin this is done by default if no other files are specified\n");
}

int main(int argc, char** argv) {
    cz::Buffer_Array buffer_array;
    buffer_array.create();
    CZ_DEFER(buffer_array.drop());

    cz::Vector<cz::Vector<cz::Str> > buckets = {};
    CZ_DEFER(buckets.drop(cz::heap_allocator()));

    char in_separator = '\n';
    char out_separator = '\n';
    bool read_stdin = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-0") == 0) {
            in_separator = 0;
        } else if (strcmp(argv[i], "-print0") == 0) {
            out_separator = 0;
        } else if (strcmp(argv[i], "-") == 0) {
            read_file(buffer_array, buckets, stdin, in_separator);
            read_stdin = false;
        } else {
            FILE* file = fopen(argv[i], "r");
            if (file) {
                read_file(buffer_array, buckets, file, in_separator);
                fclose(file);
                read_stdin = false;
            } else {
                fprintf(stderr, "Couldn't open file `%s`\n", argv[i]);
                print_usage(argv[0]);
            }
        }
    }

    if (read_stdin) {
        read_file(buffer_array, buckets, stdin, in_separator);
    }

    print_results(buckets, stdout, out_separator);
}
