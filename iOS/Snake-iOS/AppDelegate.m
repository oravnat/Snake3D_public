//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  AppDelegate.m
//  Snake-iOS
//
//  Created by Or Avnat on 29/08/2025.
//

#import "AppDelegate.h"
#import <AVFoundation/AVAudioPlayer.h>

AVAudioPlayer *players[6];
AVAudioPlayer *CreatePlayer(NSBundle *main, NSString *resName)
{
    NSURL *url = [NSURL fileURLWithPath:[main pathForResource:resName ofType:@"wav"]];

    AVAudioPlayer *player = [AVAudioPlayer alloc];
    player = [player initWithContentsOfURL:url error:nil];
    return player;
}


@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayback  error: nil];
    [session setActive:true error: nil];
    /*NSBundle *main = [NSBundle mainBundle];
    players[0] = CreatePlayer(main, @"Welcometosnake3d");
    players[1] = CreatePlayer(main, @"take");
    players[2] = CreatePlayer(main, @"explosion_deposito");
    players[3] = CreatePlayer(main, @"shoot");
    players[4] = CreatePlayer(main, @"rock_crash");
    players[5] = CreatePlayer(main, @"car_shoot_hit");
    [players[0] play];*/
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


@end
