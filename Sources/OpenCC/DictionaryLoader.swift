//
//  DictionaryLoader.swift
//  OpenCC
//
//  Created by ddddxxx on 2018/5/5.
//

import Foundation
import copencc

extension ChineseConverter {
    
    struct DictionaryLoader {
        
        private static let subdirectory = "Dictionary"
        nonisolated(unsafe) private static let dictCache = NSCache<NSString, ConversionDictionary>()
        
        private let bundle: Bundle
        
        init(bundle: Bundle) {
            self.bundle = bundle
        }
        
        func dict(_ name: ChineseConverter.DictionaryName) throws -> ConversionDictionary {
            guard let path = bundle.path(forResource: name.description, ofType: "ocd2", inDirectory: DictionaryLoader.subdirectory) else {
                throw ConversionError.fileNotFound
            }
            if let cachedDictionary = DictionaryLoader.dictCache.object(forKey: path as NSString) {
                return cachedDictionary
            } else {
                let dictionary = try ConversionDictionary(path: path)
                DictionaryLoader.dictCache.setObject(dictionary, forKey: path as NSString)
                return dictionary
            }
        }
    }
}

extension ChineseConverter.DictionaryLoader {
    
    func segmentation(options: ChineseConverter.Options) throws -> ConversionDictionary {
        let dictName = options.segmentationDictName
        return try dict(dictName)
    }
    
    func conversionChain(options: ChineseConverter.Options) throws -> [ConversionDictionary] {
        return try options.conversionChain.compactMap { names in
            switch names.count {
            case 0:
                return nil
            case 1:
                return try dict(names.first!)
            case _:
                let dicts = try names.map(dict)
                return ConversionDictionary(group: dicts)
            }
        }
    }
}
