//
//  BonjourPublisher.m
//  bobbevy
//
//  Created by Brian Richardson on 9/6/12.
//
//

#import "BonjourPublisher.h"

@implementation BonjourPublisher

-(void) publishService
{
  netService = [[NSNetService alloc] initWithDomain:@"" type:@"_osc._udp." name:@"bobbevy" port:23232];
  [netService retain];
  [netService publish];
}

@end

BonjourPublisher* publisher;

void publish_via_bonjour()
{
  publisher = [[BonjourPublisher alloc] init];
  [publisher retain];
  [publisher publishService];
}