//
//  version_helper.mm
//  chrome
//
//  Created by Viatcheslav Gachkaylo on 5/21/11.
//  Copyright 2011 Crystalnix. All rights reserved.
//

#import "chrome/browser/cocoa/version_helper.h"
#include "base/sys_string_conversions.h"

namespace version_helper {
  string16 CurrentlyInstalledVersion() {
    NSString *appPath = [[NSBundle mainBundle] bundlePath];
    NSString* appInfoPlistPath = [[appPath stringByAppendingPathComponent:@"Contents"] 
                                      stringByAppendingPathComponent:@"Info.plist"];
    NSDictionary* infoPlist =
        [NSDictionary dictionaryWithContentsOfFile:appInfoPlistPath];
    NSString *version = [infoPlist objectForKey:@"CFBundleShortVersionString"];
    return base::SysNSStringToUTF16(version);
  }
}
