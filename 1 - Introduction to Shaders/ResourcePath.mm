//
//  ResourcePath.m
//  SDL2iOSTest
//
//  Created by Ciaran McCormack on 3/12/2013.
//
//
#include "ResourcePath.h"
#import <Foundation/Foundation.h>

std::string getResourcePath(void)
{
    NSBundle* mainBundle = [NSBundle mainBundle];
    
    if (mainBundle != nil)
    {
        NSString* path = [mainBundle resourcePath];
        return [path UTF8String] + std::string("/");
    }
    
    return "";
}
