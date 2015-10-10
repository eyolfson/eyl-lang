#include <cstddef>
#include <cstdint>

enum class token {
    UNKNOWN,
    INTEGER_LITERAL,
    BINARY_OPERATION,
};

enum class binary_operation {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION
};

class integer_literal {
private:
    int64_t value;
};

int64_t integer_value(const char* start, size_t size) {
    int64_t v = 0;
    int64_t e = 1;
    size_t i = size - 1;
    while (true) {
        v += (start[i] - '0') * e;
        e *= 10;
        if (i == 0) break;
        --i;
    }
    return v;
}

bool lex(const char* input) {
    token t = token::UNKNOWN;

    char c = *input;
    const char* start;
    size_t size;
    while (c != '\0') {
        bool should_advance = true;

        if (t == token::UNKNOWN) {
            if (c >= '0' && c <= '9') {
                t = token::INTEGER_LITERAL;
                start = input;
                size = 1;
            }
            else if (c == '+') {
                printf("addition\n");
            }
            else if (c == '-') {
                printf("subtraction\n");
            }
            else if (c == '*') {
                printf("multiplication\n");
            }
            else if (c == '/') {
                printf("divison\n");
            }
            else if (c == ' ') {
                // ignore spaces
            }
            else {
                return false;
            }
        }
        else if (t == token::INTEGER_LITERAL) {
            if (c >= '0' && c <= '9') {
                ++size;
            }
            else {
                printf("integer literal with value: %ld\n",
                       integer_value(start, size));
                t = token::UNKNOWN;
                should_advance = false;
            }
        }

        if (should_advance) {
            ++input;
            c = *input;
        }
    }

    if (t == token::INTEGER_LITERAL) {
        printf("integer literal with value: %ld\n",
               integer_value(start, size));
    }

    return true;
}
