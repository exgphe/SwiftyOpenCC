//
//  ConversionError.swift
//  OpenCC
//
//  Created by ddddxxx on 2020/1/3.
//

import Foundation
import copencc

public enum ConversionError: Error {
    
    case fileNotFound
    
    case invalidFormat
    
    case invalidTextDictionary
    
    case invalidUTF8
    
    case unknown
    
    init(_ code: CCErrorCode) {
        switch code {
        case .FileNotFound:
            self = .fileNotFound
        case .InvalidFormat:
            self = .invalidFormat
        case .InvalidTextDictionary:
            self = .invalidTextDictionary
        case .InvalidUTF8:
            self = .invalidUTF8
        case .Unknown, _:
            self = .unknown
        }
    }
}
