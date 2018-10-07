//
//  AppDelegate.m
//  Nanocat_iOS
//
//  Created by Neko Code on 1/29/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AppDelegate.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    
    
    /*UIImage *navBackgroundImage = [UIImage imageNamed:@"nav_bg"];
    [[UINavigationBar appearance] setBackgroundImage:navBackgroundImage forBarMetrics:UIBarMetricsDefault];
    
    // color of button
    [[UINavigationBar appearance] setTintColor:[UIColor whiteColor]];
    
    [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
    [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent];
    
    
    // "back" navigation icon
    //[[UINavigationBar appearance] setBackIndicatorImage:[UIImage imageNamed:@"back_btn.png"]];
    //[[UINavigationBar appearance] setBackIndicatorTransitionMaskImage:[UIImage imageNamed:@"back_btn.png"]];
    
    // font style
    
    int red_c = 127;
    int green_c = 127;
    int blue_c = 127;
    
    float alpha_c = 1.0;
    
    [[UIBarItem appearance] setTitleTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                                    [UIColor colorWithRed:red_c/255.0 green:green_c/255.0 blue:blue_c/255.0 alpha:alpha_c],
                                                    [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0],
                                                    [NSValue valueWithUIOffset:UIOffsetMake(0, 1)],
                                                    [UIFont fontWithName:@"AmericanTypewriter" size:0.0],
                                                    nil]
                                          forState:UIControlStateNormal];*/
    
    return YES;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    //[[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent animated:NO];
    //[UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end