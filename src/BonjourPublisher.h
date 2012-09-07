//
//  BonjourPublisher.h
//  bobbevy
//
//  Created by Brian Richardson on 9/6/12.
//
//

#import <Cocoa/Cocoa.h>

@class NSNetService;

@interface BonjourPublisher : NSObject
{
    NSNetService *netService;
}

@end
