//
//  ConversionDictionary.swift
//  OpenCC
//
//  Created by ddddxxx on 2020/1/3.
//

import Foundation
import copencc

class ConversionDictionary {
    
    let group: [ConversionDictionary]
    
    let dict: CCDict
    
    init(path: String) throws {
        let result = CCDict.createMarisaWithPath(path)
        guard result.second == .None else {
            throw ConversionError(result.second)
        }
        self.group = []
        self.dict = result.first
    }
    
    init(group: [ConversionDictionary]) {
        let rawGroup = group.map { $0.dict }
        self.group = group
        self.dict = CCDict.createWithGroup(rawGroup, rawGroup.count)
    }
}

