//
//  version_helper.h
//  chrome
//
//  Created by Viatcheslav Gachkaylo on 5/21/11.
//  Copyright 2011 Crystalnix. All rights reserved.
//

#ifndef CHROME_BROWSER_COCOA_VERSION_HELPER_H_
#define CHROME_BROWSER_COCOA_VERSION_HELPER_H_
#pragma once

#include "base/string16.h"

#if defined(__OBJC__)

#import <Foundation/Foundation.h>

//@interface version_helper : NSObject {
//
//}
//
//@end
#endif  // __OBJC__

namespace version_helper {
  // The version of the application currently installed on disk.
  string16 CurrentlyInstalledVersion();
}

#endif  // CHROME_BROWSER_COCOA_VERSION_HELPER_H_
