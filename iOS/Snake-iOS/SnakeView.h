//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  SnakeView.h
//  Snake-iOS
//
//  Created by Or Avnat on 24/09/2025.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>


@interface SnakeView : GLKView

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;

@end
