#include "DartsDict.hpp"
#include "DictGroup.hpp"
#include "Converter.hpp"
#include "MarisaDict.hpp"
#include "MaxMatchSegmentation.hpp"
#include "Conversion.hpp"
#include "ConversionChain.hpp"

#include "header.h"
#include <list>
#include <utility>

// MARK: Error Handling

template <typename Func>
auto executeOpenCC(Func&& func) -> std::pair<typename std::result_of<Func()>::type, CCErrorCode> {
    using ReturnType = typename std::result_of<Func()>::type;

    try {
        return std::make_pair(func(), CCErrorCode::None);
    } catch (const opencc::InvalidTextDictionary&) {
        return std::make_pair(ReturnType{}, CCErrorCode::InvalidTextDictionary);
    } catch (const opencc::InvalidFormat&) {
        return std::make_pair(ReturnType{}, CCErrorCode::InvalidFormat);
    } catch (const opencc::InvalidUTF8&) {
        return std::make_pair(ReturnType{}, CCErrorCode::InvalidUTF8);
    } catch (const opencc::FileNotFound&) {
        return std::make_pair(ReturnType{}, CCErrorCode::FileNotFound);
    } catch (const opencc::Exception&) {
        return std::make_pair(ReturnType{}, CCErrorCode::Unknown);
    } catch (...) {
        return std::make_pair(ReturnType{}, CCErrorCode::Unknown);
    }
}

// MARK: CCDictImpl - Implementation Details

class CCDictImpl {
public:
    explicit CCDictImpl(opencc::DictPtr dict) : dict_(std::move(dict)) {}
    explicit CCDictImpl(opencc::DictGroupPtr dictGroup) : dictGroup_(std::move(dictGroup)) {}
    
    opencc::DictPtr getDict() const {
        if (dictGroup_) {
            return std::static_pointer_cast<opencc::Dict>(dictGroup_);
        }
        return dict_;
    }
    
    bool isValid() const {
        return dict_ || dictGroup_;
    }

private:
    opencc::DictPtr dict_;
    opencc::DictGroupPtr dictGroup_;
};

// MARK: CCDict Implementation

CCDict::CCDict() : impl_(nullptr) {}

CCDict::CCDict(std::unique_ptr<CCDictImpl> impl) : impl_(std::move(impl)) {}

CCDict::~CCDict() = default;

CCDict::CCDict(const CCDict& other) : impl_(nullptr) {
    if (other.impl_) {
        impl_ = std::unique_ptr<CCDictImpl>(new CCDictImpl(*other.impl_));
    }
}

CCDict& CCDict::operator=(const CCDict& other) {
    if (this != &other) {
        if (other.impl_) {
            impl_ = std::unique_ptr<CCDictImpl>(new CCDictImpl(*other.impl_));
        } else {
            impl_.reset();
        }
    }
    return *this;
}

CCDict::CCDict(CCDict&& other) noexcept : impl_(std::move(other.impl_)) {}

CCDict& CCDict::operator=(CCDict&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

bool CCDict::isValid() const {
    return impl_ && impl_->isValid();
}

std::pair<CCDict, CCErrorCode> CCDict::createDartsWithPath(const char* path) {
    return executeOpenCC([path]() {
        auto dict = opencc::SerializableDict::NewFromFile<opencc::DartsDict>(std::string(path));
        auto impl = std::unique_ptr<CCDictImpl>(new CCDictImpl(dict));
        return CCDict(std::move(impl));
    });
}

std::pair<CCDict, CCErrorCode> CCDict::createMarisaWithPath(const char* path) {
    return executeOpenCC([path]() {
        auto dict = opencc::SerializableDict::NewFromFile<opencc::MarisaDict>(std::string(path));
        auto impl = std::unique_ptr<CCDictImpl>(new CCDictImpl(dict));
        return CCDict(std::move(impl));
    });
}

CCDict CCDict::createWithGroup(const CCDict* dictGroup, size_t count) {
    std::list<opencc::DictPtr> dictList;
    for (size_t i = 0; i < count; ++i) {
        if (dictGroup[i].isValid()) {
            dictList.push_back(dictGroup[i].impl()->getDict());
        }
    }
    
    auto groupPtr = opencc::DictGroupPtr(new opencc::DictGroup(dictList));
    auto impl = std::unique_ptr<CCDictImpl>(new CCDictImpl(std::move(groupPtr)));
    return CCDict(std::move(impl));
}

// MARK: CCConverterImpl - Implementation Details

class CCConverterImpl {
public:
    explicit CCConverterImpl(std::unique_ptr<opencc::Converter> converter)
        : converter_(std::move(converter)) {}
    
    opencc::Converter* get() const { return converter_.get(); }
    
    bool isValid() const { return converter_ != nullptr; }

private:
    std::unique_ptr<opencc::Converter> converter_;
};

// MARK: CCConverter Implementation

CCConverter::CCConverter(std::unique_ptr<CCConverterImpl> impl)
    : impl_(std::move(impl)) {}

CCConverter::~CCConverter() = default;

CCConverter::CCConverter(CCConverter&& other) noexcept
    : impl_(std::move(other.impl_)) {}

CCConverter& CCConverter::operator=(CCConverter&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

bool CCConverter::isValid() const {
    return impl_ && impl_->isValid();
}

CCConverter CCConverter::create(
    const char* name,
    CCDict segmentation,
    const CCDict* conversionChain,
    size_t chainCount
) {
    // Build conversion chain
    std::list<opencc::ConversionPtr> conversions;
    for (size_t i = 0; i < chainCount; ++i) {
        if (conversionChain[i].isValid()) {
            auto conversion = opencc::ConversionPtr(
                new opencc::Conversion(conversionChain[i].impl()->getDict())
            );
            conversions.push_back(std::move(conversion));
        }
    }
    
    // Create segmentation
    auto segmentationPtr = opencc::SegmentationPtr(
        new opencc::MaxMatchSegmentation(segmentation.impl()->getDict())
    );
    
    // Create conversion chain
    auto chainPtr = opencc::ConversionChainPtr(
        new opencc::ConversionChain(conversions)
    );
    
    // Create converter
    auto converter = std::unique_ptr<opencc::Converter>(
        new opencc::Converter(std::string(name), segmentationPtr, chainPtr)
    );
    
    auto impl = std::unique_ptr<CCConverterImpl>(
        new CCConverterImpl(std::move(converter))
    );
    
    return CCConverter(std::move(impl));
}

std::pair<std::string, CCErrorCode> CCConverter::convert(const char* input) const {
    if (!isValid()) {
        return std::make_pair(std::string{}, CCErrorCode::Unknown);
    }
    
    return executeOpenCC([this, input]() {
        return impl_->get()->Convert(std::string(input));
    });
}
