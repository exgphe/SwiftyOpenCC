#ifndef COPENCC_HEADER_H
#define COPENCC_HEADER_H

#include <string>
#include <vector>
#include <swift/bridging>

// MARK: Error

enum class CCErrorCode {
    None,
    InvalidTextDictionary,
    InvalidFormat,
    InvalidUTF8,
    FileNotFound,
    Unknown
};

// MARK: CCDict

class CCDict;

// Forward declaration of implementation class
class CCDictImpl;

class CCDict {
public:
    // Factory methods for creating dictionaries
    // Returns std::pair<CCDict, CCErrorCode> where .second == CCErrorCode::None on success
    static std::pair<CCDict, CCErrorCode> createDartsWithPath(const char* path);
    static std::pair<CCDict, CCErrorCode> createMarisaWithPath(const char* path);
    static CCDict createWithGroup(const CCDict *__counted_by(count) dictGroup, size_t count);

    // Default constructor creates an empty/invalid dict
    CCDict();
    
    // Destructor
    ~CCDict();
    
    // Copy semantics (shares underlying dictionary data)
    CCDict(const CCDict& other);
    CCDict& operator=(const CCDict& other);
    
    // Move semantics
    CCDict(CCDict&& other) noexcept;
    CCDict& operator=(CCDict&& other) noexcept;
    
    // Check if the dictionary is valid
    bool isValid() const SWIFT_COMPUTED_PROPERTY;
    
    // Internal access (for implementation only)
    CCDictImpl* impl() const { return impl_.get(); }

private:
    // Private constructor for internal use
    explicit CCDict(std::unique_ptr<CCDictImpl> impl);
    
    std::unique_ptr<CCDictImpl> impl_;
};

// MARK: CCConverter

// Forward declaration of implementation class
class CCConverterImpl;

class CCConverter {
public:
    // Factory method for creating converter
    static CCConverter create(
        const char* name,
        CCDict segmentation,
        const CCDict* conversionChain,
        size_t chainCount
    );
    
    // Destructor
    ~CCConverter();
    
    // Move semantics (no copying to enforce ownership)
    CCConverter(CCConverter&& other) noexcept;
    CCConverter& operator=(CCConverter&& other) noexcept;
    
    // Delete copy operations
    CCConverter(const CCConverter&) = delete;
    CCConverter& operator=(const CCConverter&) = delete;
    
    // Convert a string
    // Returns std::pair<std::string, CCErrorCode> where .second == CCErrorCode::None on success
    std::pair<std::string, CCErrorCode> convert(const char* input) const;
    
    // Check if the converter is valid
    bool isValid() const SWIFT_COMPUTED_PROPERTY;

protected:
    CCConverter() = default;
    
private:
    // Private constructor for internal use
    explicit CCConverter(std::unique_ptr<CCConverterImpl> impl);
    
    std::unique_ptr<CCConverterImpl> impl_;
};

#endif // COPENCC_HEADER_H

