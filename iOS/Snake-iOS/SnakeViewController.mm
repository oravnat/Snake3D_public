//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  SnakeViewController.m
//  Snake-iOS
//
//  Created by Or Avant on 19/09/2025.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

#include "SnakeGame.hpp"

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <AVFoundation/AVAudioPlayer.h>

bool SnakeInit(const char path[]);

SnakeGame* g_game;
SnakeRenderer g_snakeRenderer;
char path[512];
extern const char* g_path;

extern AVAudioPlayer *players[6];

/*
 This class is the choosed controller in Xcode IDE under "Main.storyboard"
 */

@interface SnakeViewController : GLKViewController

@end

@implementation SnakeViewController
{
    //EAGLView *_view;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Create an OpenGL ES context and assign it to the view loaded from storyboard
    GLKView *view = (GLKView *)self.view;

    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!context || ![EAGLContext setCurrentContext:context])
        return;

    view.context = context;
 
    // Configure renderbuffers created by the view
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    
    self.preferredFramesPerSecond = 60;
    
    NSBundle *main = [NSBundle mainBundle];
    NSString* res = [main resourcePath];

    strcpy(path, [res UTF8String]);
    strcat(path, "/");

    g_game = new SnakeGame;
    SnakeInit(path);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double nowNs = now.tv_sec*1000000000ull + now.tv_nsec;

    g_game->Think(nowNs / 1000000000.0);
    g_game->Resize(rect.size.width, rect.size.height);
    g_game->Render(g_snakeRenderer);
    
    for (int i = 0; i < 6; i++)
    {
        if (g_game->IsSoundPlayed(i) && players[i])
            [players[i] play];
    }
}

@end

bool SnakeInit(const char path[])
{
    if (!g_snakeRenderer.Init())
        return false;

    g_path = path;

    if (!g_game->Init(path)) {
        delete g_game;
        g_game = NULL;
        return false;
    }
    if (g_game)
        g_game->NewGame();

    return true;
}
