#ifndef LITTERALSTRING_HPP
#define LITTERALSTRING_HPP

#include <cstddef>
#include <cstring>
#include <cassert>
#include <string>

namespace Utils {
/*!
 * This code is strongly inspired by
 * http://www.daniweb.com/software-development/cpp/code/482276/c-11-compile-time-string-concatenation-with-constexpr
 * itself coming from
 * http://github.com/boostcon/cppnow_presentations_2012/blob/master/wed/schurr_cpp11_tools_for_class_authors.pdf
 *
 * \FIXME Successive call to operator+ discard const qualifier
 */
class LiteralString {
private:
    const char* const text_ptr;
    const unsigned int text_size;
    const LiteralString* const head;
    constexpr char get_char_from_head(unsigned int i, unsigned int hd_size) const {
        return (i < hd_size ? (*head)[i] : (i < (hd_size + text_size) ? text_ptr[i - hd_size] : '\0'));
    }

    static constexpr std::size_t fnv_prime = (sizeof(std::size_t) == 8 ? 1099511628211u : 16777619u);
    static constexpr std::size_t fnv_offset = (sizeof(std::size_t) == 8 ? 14695981039346656037u : 2166136261u);
    constexpr std::size_t fnv_1a_hash(unsigned int i) const {
        return (i == 0 ?
                    (head != nullptr ?
                    ((head->fnv_1a_hash(head->text_size-1) ^ text_ptr[0]) * fnv_prime) :
                    fnv_offset) :
            ((fnv_1a_hash(i-1) ^ text_ptr[i]) * fnv_prime));
    }

//    template <typename FwdIter>
//    inline void copy_to_recurse(FwdIter& beg, FwdIter end) const {
//        if( head != nullptr )
//        head->copy_to_recurse(beg, end);
//        for(unsigned int i = 0; (i < text_size) && (beg != end); ++i, ++beg)
//            *beg = text_ptr[i];
//    }
//    inline void copy_to_recurse(char*& beg, char* end) const {
//        if( head != nullptr )
//            head->copy_to_recurse(beg, end);
//        std::size_t sz_to_cpy = (end - beg < text_size ? end - beg : text_size);
//        std::memcpy(beg, text_ptr, sz_to_cpy);
//        beg += sz_to_cpy;
//    }
    constexpr LiteralString(const char* aStr, unsigned int N,
                               const LiteralString* aHead = nullptr) :
        text_ptr(aStr), text_size(N), head(aHead) { }
    friend std::ostream& operator<<(std::ostream& os, const LiteralString& str);

    //! \warning The current instance is invalidated if aStr is  destructed
    //! Should be used internally only
    LiteralString(const std::string& aStr) :
        text_ptr(aStr.c_str()), text_size(aStr.length()), head(nullptr) {
        assert(aStr.length() >= 1);
    }


public:
    template <unsigned int N>
    constexpr LiteralString(const char(&aStr)[N],
                            const LiteralString* aHead = nullptr) :
        text_ptr(aStr), text_size(N-1), head(aHead) {
        static_assert(N >= 1, "Invalid string literal! Length is zero!");
    }
    constexpr const char* buf() const { return text_ptr; }
    constexpr unsigned int size() const {
        return text_size + (head != nullptr ? head->size() : 0);
    }
    constexpr char operator[](unsigned int i) const {
        return (head != nullptr ?
                    get_char_from_head(i, head->size()) :
                    (i < text_size ? text_ptr[i] : '\0'));
    }
    template <unsigned int N>
    constexpr LiteralString operator+(const char(&aHead)[N]) const {
        return LiteralString(aHead, this);
    }
    constexpr LiteralString operator+(const LiteralString& aHead) const {
        return LiteralString(aHead.text_ptr, aHead.text_size, this);
    }
//    constexpr LiteralString operator+(const LiteralString& aHead) const {
//        return (head != nullptr ?
//                    LiteralString((*head+aHead).text_ptr, (*head+aHead).text_size, this) :
//                    LiteralString(aHead.text_ptr, aHead.text_size, this));
//    }
    constexpr std::size_t hash() const {
        return fnv_1a_hash(text_size-1);
    }

    static std::size_t hash(const std::string& str){
        LiteralString litStr (str);
        return litStr.hash();
    }
    constexpr bool operator==(const LiteralString& lside) const {
        return hash() == lside.hash();
    }

//    template <typename FwdIter>
//    inline void copy_to(FwdIter beg, FwdIter end) const {
//        copy_to_recurse(beg, end);
//    }
//    inline void copy_to(char* beg, char* end) const {
//        copy_to_recurse(beg, end);
//    }
    inline operator std::string() const{
        if( head != nullptr )
            return std::string(*head).append(std::string(text_ptr));
        return std::string(text_ptr);
    }
};

inline std::ostream& operator<<(std::ostream& os, const LiteralString& str)
{
    if( str.head != nullptr )
        os << *(str.head);
    os << std::string(str.text_ptr);
    return os;
}

} // namespace Utils

#endif // LITTERALSTRING_HPP
