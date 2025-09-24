//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  SnakeView.m
//  Snake-iOS
//
//  Created by Or Avnat on 24/09/2025.
//

#import "SnakeView.h"
#include "SnakeGame.hpp"

/*
 This class is the choosed view in Xcode IDE under "Main.storyboard"
 */

extern SnakeGame* g_game;

@implementation SnakeView

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    CGPoint touchPoint = [touch locationInView:self];
    float width = self.bounds.size.width;
    float height = self.bounds.size.height;

    int x = touchPoint.x;
    int y = touchPoint.y;
    if (g_game)
        g_game->StartClick(2.0f * x / width - 1.0f, 2.0f * y / height - 1.0f);
    
    if (g_game && g_game->m_players[0] && g_game->m_players[0]->m_pCar)
    {
        Car* pCar = g_game->m_players[0]->m_pCar;
        if (x < width / 2)
            pCar->m_rotation = 1.0;
        else
            pCar->m_rotation = -1.0;
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    CGPoint touchPoint = [touch locationInView:self];
    float width = self.bounds.size.width;
    float height = self.bounds.size.height;
    int x = touchPoint.x;
    int y = touchPoint.y;
    if (g_game)
        g_game->EndClick(2.0f * x / width - 1.0f, 2.0f * y / height - 1.0f);
    
    if (g_game && g_game->m_players[0] && g_game->m_players[0]->m_pCar)
    {
        Car* pCar = g_game->m_players[0]->m_pCar;
        pCar->m_rotation = 0.0;
    }
}


@end
