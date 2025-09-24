//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  main.m
//  Snake-iOS
//
//  Created by Or Avnat on 29/08/2025.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
